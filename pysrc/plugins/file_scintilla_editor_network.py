import os
import logging
import threading
import struct

from wxglterm_interface import TermNetwork
from multiple_instance_plugin_base import MultipleInstancePluginBase

LOGGER = logging.getLogger('term_network')


class FileTermNetwork(MultipleInstancePluginBase, TermNetwork):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self, name="scintilla_editor_network_use_file",
                                            desc="It is a python version scintilla editor network use file",
                                            version=1)
        TermNetwork.__init__(self)

    def disconnect(self):
        pass

    def connect(self, host, port, user_name, password):
        self._file_path = file_path = self.get_plugin_config().get_entry("/file", "NOT FOUND")
        self._len_prefix = self.get_plugin_config().get_entry_bool("/length_prefix", True)

        if not os.path.exists(file_path):
            LOGGER.error("term data file is not exist:{}".format(file_path))

        self.start_reader()

    def start_reader(self):
        term_data_handler = self.get_plugin_context().get_term_data_handler()

        def __read_term_data():
            with open(self._file_path, 'rb') as f:
                for data in ['\x1B]file_path;', self._file_path, '\x07']:
                    data = bytes(data, 'utf-8')
                    term_data_handler.on_data(data, len(data))

                while True:
                    data = f.readline()

                    if not data:
                        LOGGER.info("end of dump data, quit")
                        data = bytes('\x1B[H', 'utf-8')
                        term_data_handler.on_data(data, len(data))
                        break

                    term_data_handler.on_data(data, len(data))
                return

        def read_term_data():
            try:
                __read_term_data()
            except:
                LOGGER.exception('read term data failed')

        self.reader_thread = reader_thread = threading.Thread(target=read_term_data)
        reader_thread.start()

    def send(self, data, n):
        term_data_handler = self.get_plugin_context().get_term_data_handler()

        term_data_handler.on_data(data, n)

    def resize(self, row, col):
        pass

def register_plugins(pm):
    pm.register_plugin(FileTermNetwork().new_instance())
