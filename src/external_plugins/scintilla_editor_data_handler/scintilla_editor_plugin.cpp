#include <iostream>
#include "plugin_manager.h"
#include "scintilla_editor_impl.h"

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    (void)plugin_manager;
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateTermDataHandler()));
}
