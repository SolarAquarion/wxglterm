#pragma once

#include "py_plugin.h"
#include "term_ui.h"

template<class TermUIBase = TermUI>
class PyTermUI : public PyPlugin<TermUIBase> {
public:
    using PyPlugin<TermUIBase>::PyPlugin;

    void Refresh() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermUIBase, "refresh", Refresh, );
    }
};
