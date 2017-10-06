#include <pybind11/embed.h>

#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "app.h"

#include "controller.h"
#include "wxglterm_interface.h"
#include "app_config_impl.h"

#include "term_context.h"
#include "term_network.h"

#include "default_term_ui.h"
#include "default_term_buffer.h"

#include <iostream>
namespace py = pybind11;

wxGLTermApp::~wxGLTermApp()
{
    Cleanup();
}

bool wxGLTermApp::Run(int /*argc*/, char ** /*argv*/)
{
    bool result = false;
    try
    {
        result = DoInit();
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error Initialize:"
                  << std::endl
                  << e.what()
                  << std::endl;
        PyErr_Print();
    }
    catch(...)
    {
        std::cerr << "!!Error Initialize"
                  << std::endl;
        PyErr_Print();
    }

    if (!result)
    {
        Cleanup();
    }

    return result;
}

void wxGLTermApp::Cleanup()
{
    g_AppConfig.reset((AppConfig*)nullptr);
}

bool wxGLTermApp::DoInit()
{
    m_PyInterpreter = std::make_shared<py::scoped_interpreter>();

    init_wxglterm_interface_module();

    g_AppConfig->LoadFromFile("wxglterm.json");

    std::cout << "termcap_dir:"
              << g_AppConfig->GetEntry("/term/termcap_dir", "NOT FOUND!!!")
              << ","
              << g_AppConfig->GetEntry("/term/termcap_dir", "NOT FOUND!!! with path start with /")
              << std::endl
              << "use_system_ssh_config:" << g_AppConfig->GetEntryBool("use_system_ssh_config", false)
              << std::endl;

    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    f.AppendDir("plugins");
    std::string appPath(f.GetPath());

    std::cout << "app path:" << appPath << std::endl;

    std::string plugins_dir = g_AppConfig->GetEntry("plugins/dir", appPath.c_str());

    py::object sys_m = py::module::import("sys");
    sys_m.attr("path").attr("append")(appPath);

    auto plugin_manager = LoadAllPlugins(appPath.c_str());

    if (plugins_dir != appPath)
    {
        sys_m.attr("path").attr("append")(py::cast(plugins_dir));
        LoadAllPlugins(plugin_manager, plugins_dir.c_str());
    }

    m_PluginManager.swap(plugin_manager);

    InitDefaultPlugins();

    auto term_context = CreateTermContext();
    auto term_ui = CreateTermUI(term_context);
    auto term_network = CreateTermNetwork(term_context);

    if (term_ui && term_network)
    {
        term_context->SetTermUI(term_ui);
        term_context->SetTermNetwork(term_network);

        term_network->Connect("", 0, "", "");
        m_TermUIList.push_back(term_ui);

        term_ui->StartMainUILoop();
        return true;
    }
    else
    {
        return false;
    }
}

TermUIPtr wxGLTermApp::CreateTermUI(TermContextPtr term_context)
{
    std::string ui_plugin_name = g_AppConfig->GetEntry("plugins/ui/name", "default_term_ui");
    uint64_t ui_plugin_version = g_AppConfig->GetEntryUInt64("plugins/ui/version", PluginManager::Latest);
    std::string ui_plugin_config = g_AppConfig->GetEntry("plugins/ui/config", "{}");

    auto plugin_term_ui = std::dynamic_pointer_cast<TermUI>(m_PluginManager->GetPlugin(ui_plugin_name.c_str(), ui_plugin_version));

    if (!plugin_term_ui)
    {
        //TODO: error or create default
        return TermUIPtr{};
    }

    std::cout << "ui plugin config:"
              << ui_plugin_config
              << std::endl;

    auto new_instance = plugin_term_ui->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(ui_plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<TermUI>(new_instance);
}

void wxGLTermApp::InitDefaultPlugins()
{
    m_PluginManager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultTermUI()));
    m_PluginManager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultTermBuffer()));
}

TermContextPtr wxGLTermApp::CreateTermContext()
{
    std::string context_plugin_name = g_AppConfig->GetEntry("plugins/context/name", "default_term_context");
    uint64_t context_plugin_version = g_AppConfig->GetEntryUInt64("plugins/context/version", PluginManager::Latest);
    std::string context_plugin_config = g_AppConfig->GetEntry("plugins/context/config", "{}");

    auto plugin_term_context = std::dynamic_pointer_cast<TermContext>(m_PluginManager->GetPlugin(context_plugin_name.c_str(), context_plugin_version));

    if (!plugin_term_context)
    {
        //TODO: error or create default
        return TermContextPtr{};
    }

    auto new_instance = plugin_term_context->NewInstance();

    return std::dynamic_pointer_cast<TermContext>(new_instance);
}

TermNetworkPtr wxGLTermApp::CreateTermNetwork(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/network/name", "default_term_nextwork");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/network/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/network/config", "{}");

    std::cout << "network plugin config:"
              << plugin_config
              << std::endl;

    auto plugin = std::dynamic_pointer_cast<TermNetwork>(m_PluginManager->GetPlugin(plugin_name.c_str(), plugin_version));

    if (!plugin)
    {
        //TODO: error or create default
        return TermNetworkPtr{};
    }

    auto new_instance = plugin->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<TermNetwork>(new_instance);
}

int main(int argc, char ** argv) {
    wxGLTermApp app;

    app.Run(argc, argv);
    return 0;
}
