#pragma once

#include "py_multiple_instance_plugin.h"
#include "term_ui.h"

template<class TermUIBase = TermUI>
class PyTermUI : public virtual PyMultipleInstancePlugin<TermUIBase> {
public:
    using PyMultipleInstancePlugin<TermUIBase>::PyMultipleInstancePlugin;

    void Refresh() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermUIBase, "refresh", Refresh, );
    }

    void Show() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermUIBase, "show", Show, );
    }

    int32_t StartMainUILoop() {
        PYBIND11_OVERLOAD_PURE_NAME(int32_t, TermUIBase, "start_main_ui_loop", StartMainUILoop, );
    }
};
