from wxglterm_interface import Plugin, print_plugin_info, TermContext, TermBuffer
from context_base import ContextBase

class DefaultTermContext(ContextBase, TermContext):
    def __init__(self):
        ContextBase.__init__(self, name="default_term_context",
                             desc="It is a python version term default_term_context",
                             version=1)
        TermContext.__init__(self)

        self._term_buffer = None
        self._term_window = None
        self._term_network = None
        self._term_data_handler = None

    def get_term_buffer(self):
        return self._term_buffer

    def set_term_buffer(self, term_buffer):
        #hack way to cast the object created by pybind11
        if term_buffer.__class__ != TermBuffer:
            term_buffer.__class__ = TermBuffer

        self._term_buffer = term_buffer

    def get_term_window(self):
        return self._term_window

    def set_term_window(self, term_window):
        self._term_window = term_window

    def get_term_network(self):
        return self._term_network

    def set_term_network(self, term_network):
        self._term_network = term_network

    def get_term_data_handler(self):
        return self._term_data_handler

    def set_term_data_handler(self, term_data_handler):
        self._term_data_handler = term_data_handler

    term_buffer = property(get_term_buffer, set_term_buffer)
    term_window = property(get_term_window, set_term_window)
    term_network = property(get_term_network, set_term_network)
    term_data_handler = property(get_term_data_handler, set_term_data_handler)

def register_plugins(pm):
    ni = DefaultTermContext().new_instance()
    pm.register_plugin(ni)
