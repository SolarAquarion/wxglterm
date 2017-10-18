#include <iostream>

#ifdef WXGLTERM_DYNAMIC_INTERFACE
#include <pybind11/pybind11.h>
#else
#include <pybind11/embed.h>
#endif

#include "py_term_cell.h"
#include "py_term_line.h"
#include "py_term_selection.h"
#include "py_term_buffer.h"
#include "py_term_window.h"
#include "py_task.h"
#include "py_term_ui.h"
#include "py_term_network.h"
#include "py_term_data_handler.h"
#include "py_term_context.h"

#include "py_plugin_manager.h"

#include "py_multiple_instance_plugin.h"

#include "py_app_config.h"

#include "wxglterm_interface.h"

namespace py = pybind11;

void print_plugin_info(Plugin * plugin)
{
    std::cerr << "Name:" << plugin->GetName()
              << ", Description:" << plugin->GetDescription()
              << ", Version:" << plugin->GetVersion()
              << std::endl;
}

#ifdef WXGLTERM_DYNAMIC_INTERFACE
PYBIND11_MODULE(wxglterm_interface, m)
#else
PYBIND11_EMBEDDED_MODULE(wxglterm_interface, m)
#endif
{
    m.def("print_plugin_info", &print_plugin_info);

    py::class_<Plugin, PyPlugin<>, std::shared_ptr<Plugin>> plugin(m, "Plugin");
    plugin.def(py::init<>())
            .def("get_name", &Plugin::GetName)
            .def("get_description", &Plugin::GetDescription)
            .def("get_version", &Plugin::GetVersion)
            .def("init_plugin", &Plugin::InitPlugin)
            ;

    py::class_<MultipleInstancePlugin, PyMultipleInstancePlugin<>, std::shared_ptr<MultipleInstancePlugin>> multiple_instance_plugin(m, "MultipleInstancePlugin", plugin);
    multiple_instance_plugin.def(py::init<>())
            .def("new_instance", &MultipleInstancePlugin::NewInstance);

    py::class_<Context, PyContext<>, std::shared_ptr<Context>> context(m, "Context", multiple_instance_plugin);
    context.def(py::init<>())
            .def_property("app_config", &Context::GetAppConfig, &Context::SetAppConfig)
            ;

    py::class_<TermBuffer, PyTermBuffer<>, std::shared_ptr<TermBuffer>> term_buffer(m, "TermBuffer", multiple_instance_plugin);
    term_buffer.def(py::init<>())
            .def("resize", &TermBuffer::Resize, py::call_guard<py::gil_scoped_release>())
            .def_property_readonly("rows", &TermBuffer::GetRows, py::call_guard<py::gil_scoped_release>())
            .def_property_readonly("cols", &TermBuffer::GetCols, py::call_guard<py::gil_scoped_release>())
            .def_property("row", &TermBuffer::GetRow, &TermBuffer::SetRow, py::call_guard<py::gil_scoped_release>())
            .def_property("col", &TermBuffer::GetCol, &TermBuffer::SetCol, py::call_guard<py::gil_scoped_release>())
            .def("get_line", &TermBuffer::GetLine, py::call_guard<py::gil_scoped_release>())
            .def("get_cell", &TermBuffer::GetCell, py::call_guard<py::gil_scoped_release>())
            .def_property_readonly("cur_line", &TermBuffer::GetCurLine, py::call_guard<py::gil_scoped_release>())
            .def_property_readonly("cur_cell", &TermBuffer::GetCurCell, py::call_guard<py::gil_scoped_release>())
            .def("scroll_buffer", &TermBuffer::ScrollBuffer, py::call_guard<py::gil_scoped_release>())
            .def_property("scroll_region_begin", &TermBuffer::GetScrollRegionBegin, &TermBuffer::SetScrollRegionBegin, py::call_guard<py::gil_scoped_release>())
            .def_property("scroll_region_end", &TermBuffer::GetScrollRegionEnd, &TermBuffer::SetScrollRegionEnd, py::call_guard<py::gil_scoped_release>())
            .def("delete_lines", &TermBuffer::DeleteLines, py::call_guard<py::gil_scoped_release>())
            .def("insert_lines", &TermBuffer::InsertLines, py::call_guard<py::gil_scoped_release>())
            .def("set_cell_defaults", &TermBuffer::SetCellDefaults, py::call_guard<py::gil_scoped_release>())
            .def("create_cell_with_defaults", &TermBuffer::CreateCellWithDefaults, py::call_guard<py::gil_scoped_release>())
            .def_property("selection", &TermBuffer::GetSelection, &TermBuffer::SetSelection, py::call_guard<py::gil_scoped_release>())
            .def("clear_selection", &TermBuffer::ClearSelection, py::call_guard<py::gil_scoped_release>())
            .def("move_cur_row", &TermBuffer::MoveCurRow, py::call_guard<py::gil_scoped_release>())
            .def("set_cur_cell_data", &TermBuffer::SetCurCellData, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<TermLine, PyTermLine<>, std::shared_ptr<TermLine>> term_line(m, "TermLine", plugin);
    term_line.def(py::init<>())
            .def("resize", &TermLine::Resize, py::call_guard<py::gil_scoped_release>())
            .def("get_cell", &TermLine::GetCell, py::call_guard<py::gil_scoped_release>())
            .def("insert_cell", &TermLine::InsertCell, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<TermCell, PyTermCell<>, std::shared_ptr<TermCell>> term_cell(m, "TermCell", plugin);
    term_cell.def(py::init<>())
            .def_property("char", &TermCell::GetChar, &TermCell::SetChar, py::call_guard<py::gil_scoped_release>())
            .def_property("fore_color_idx", &TermCell::GetForeColorIndex, &TermCell::SetForeColorIndex, py::call_guard<py::gil_scoped_release>())
            .def_property("back_color_idx", &TermCell::GetBackColorIndex, &TermCell::SetBackColorIndex, py::call_guard<py::gil_scoped_release>())
            .def_property("mode", &TermCell::GetMode, &TermCell::SetMode, py::call_guard<py::gil_scoped_release>())
            .def("add_mode", &TermCell::AddMode, py::call_guard<py::gil_scoped_release>())
            .def("remove_mode", &TermCell::RemoveMode, py::call_guard<py::gil_scoped_release>())
            .def("reset", &TermCell::Reset, py::call_guard<py::gil_scoped_release>())
            .def_property("is_wide_char", &TermCell::IsWideChar, &TermCell::SetWideChar, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<TermSelection, PyTermSelection<>, std::shared_ptr<TermSelection>> term_selection(m, "TermSelection", plugin);
    term_selection.def(py::init<>())
            .def_property("row_begin", &TermSelection::GetRowBegin, &TermSelection::SetRowBegin, py::call_guard<py::gil_scoped_release>())
            .def_property("col_begin", &TermSelection::GetColBegin, &TermSelection::SetColBegin, py::call_guard<py::gil_scoped_release>())
            .def_property("row_end", &TermSelection::GetRowEnd, &TermSelection::SetRowEnd, py::call_guard<py::gil_scoped_release>())
            .def_property("col_end", &TermSelection::GetColEnd, &TermSelection::SetColEnd, py::call_guard<py::gil_scoped_release>())
            ;

    py::enum_<TermCell::ColorIndexEnum>(term_cell, "ColorIndex")
            .value("DefaultColorIndex", TermCell::ColorIndexEnum::DefaultColorIndex)
            .export_values();
    py::enum_<TermCell::TextModeEnum>(term_cell, "TextMode")
            .value("Stdout", TermCell::TextModeEnum::Stdout)
            .value("Reverse", TermCell::TextModeEnum::Stdout)
            .value("Selection", TermCell::TextModeEnum::Selection)
            .value("Cursor", TermCell::TextModeEnum::Cursor)
            .value("Bold", TermCell::TextModeEnum::Bold)
            .value("Dim", TermCell::TextModeEnum::Dim)
            .export_values();

    py::class_<TermUI, PyTermUI<>, std::shared_ptr<TermUI>> term_ui(m, "TermUI", plugin);
    term_ui.def(py::init<>())
            .def("start_main_ui_loop", &TermUI::StartMainUILoop, py::call_guard<py::gil_scoped_release>())
            .def("create_window", &TermUI::CreateWindow, py::call_guard<py::gil_scoped_release>())
            .def("schedule_task", &TermUI::ScheduleTask, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<TermWindow, PyTermWindow<>, std::shared_ptr<TermWindow>> term_window(m, "TermWindow", plugin);
    term_window.def(py::init<>())
            .def("refresh", &TermWindow::Refresh, py::call_guard<py::gil_scoped_release>())
            .def("show", &TermWindow::Show, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<TermNetwork, PyTermNetwork<>, std::shared_ptr<TermNetwork>> term_network(m, "TermNetwork", multiple_instance_plugin);
    term_network.def(py::init<>())
            .def("disconnect", &TermNetwork::Disconnect, py::call_guard<py::gil_scoped_release>())
            .def("connect", &TermNetwork::Connect, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<TermContext, PyTermContext<>, std::shared_ptr<TermContext>> term_context(m, "TermContext", context);
    term_context.def(py::init<>())
            .def_property("term_buffer", &TermContext::GetTermBuffer, &TermContext::SetTermBuffer, py::call_guard<py::gil_scoped_release>())
            .def_property("term_window", &TermContext::GetTermWindow, &TermContext::SetTermWindow, py::call_guard<py::gil_scoped_release>())
            .def_property("term_network", &TermContext::GetTermNetwork, &TermContext::SetTermNetwork, py::call_guard<py::gil_scoped_release>())
            .def_property("term_data_handler", &TermContext::GetTermDataHandler, &TermContext::SetTermDataHandler, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<PluginManager, PyPluginManager<>, std::shared_ptr<PluginManager>> plugin_manager(m, "PluginManager");
    plugin_manager.def(py::init<>())
            .def("register_plugin", (void(PluginManager::*)(std::shared_ptr<Plugin>))&PluginManager::RegisterPlugin)
            .def("register_plugin", (void(PluginManager::*)(const char*))&PluginManager::RegisterPlugin)
            .def("get_plugin", &PluginManager::GetPlugin);

    py::enum_<PluginManager::VersionCodeEnum>(plugin_manager, "VersionCode")
            .value("Latest", PluginManager::VersionCodeEnum::Latest)
            .export_values();

    py::class_<AppConfig, PyAppConfig<>, std::shared_ptr<AppConfig>> app_config(m, "AppConfig");
    app_config.def(py::init<>())
            .def("get_entry", &AppConfig::GetEntry)
            .def("get_entry_int64", &AppConfig::GetEntryInt64)
            .def("get_entry_uint64", &AppConfig::GetEntryUInt64)
            .def("get_entry_bool", &AppConfig::GetEntryBool)
            .def("load_from_file", &AppConfig::LoadFromFile)
            .def("load_from_string", &AppConfig::LoadFromString)
            ;
    py::class_<TermDataHandler, PyTermDataHandler<>, std::shared_ptr<TermDataHandler>> term_data_handler(m, "TermDataHandler", multiple_instance_plugin);
    term_data_handler.def(py::init<>())
            .def("on_data", &TermDataHandler::OnData, py::call_guard<py::gil_scoped_release>())
            ;

    py::class_<Task, PyTask<>, std::shared_ptr<Task>> task(m, "Task", multiple_instance_plugin);
    task.def(py::init<>())
            .def("run", &Task::Run)
            .def("cancel", &Task::Cancel)
            .def("is_cancelled", &Task::IsCancelled)
            ;
}

void init_wxglterm_interface_module()
{
    auto py_module1 = py::module::import("wxglterm_interface");

    py::print(py_module1);
}
