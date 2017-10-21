import os
import logging
import json

from wxglterm_interface import TermColorTheme, TermColor, TermCell
from multiple_instance_plugin_base import MultipleInstancePluginBase

LOGGER = logging.getLogger('term_color_theme')


class DefaultTermColorTheme(MultipleInstancePluginBase, TermColorTheme):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self,
                                            name="default_term_color_theme",
                                            desc="It is a python version term color theme",
                                            version=1)
        TermColorTheme.__init__(self)
        self._theme_name = ''
        self._values = {}

        self.__init_colors_map()

    def load(self, name):
        color_theme_dir = self.plugin_context.app_config.get_entry("/term/color_theme_dir", "data/color_theme")

        f_path = os.path.join(color_theme_dir, name+".json")

        if not os.path.isfile(f_path):
            return False

        with open(f_path, 'r') as f:
            values = f.read()

            return self.load_with_values(name, values)

    def load_with_values(self, name, values):
        self._theme_name = name
        self._values.update(self.__to_colors_map(json.loads(values)))

        return True

    def get_color(self, index):
        key = str(index)

        try:
            return self._values[key]
        except:
            return TermColor()

    def __to_colors_map(self, values):
        colors_map = {}

        for key in values:
            v = values[key]

            if len(v) < 6:
                continue

            c = TermColor()
            c.r = int(v[:2], 16)
            c.g = int(v[2:4], 16)
            c.b = int(v[4:6], 16)
            c.a = int(v[6:8], 16) if len(v) >= 8 else 0xFF

            colors_map[key] = c

        return colors_map

    def __from_array_to_color(self, v):
        c = TermColor()

        c.r, c.g, c.b, c.a = int(v[0]) & 0xFF, int(v[1]) & 0xFF, int(v[2]) & 0xFF, int(v[3]) & 0xFF

        return c

    def __init_colors_map(self):
        COLOR_SET_0_RATIO = 0x44
        COLOR_SET_1_RATIO = 0xaa

        #ansi color
        COLOR_TABLE = [
            [0, 0, 0, 0xFF], #BLACK
            [COLOR_SET_0_RATIO, 0, 0, 0xFF], #RED
            [0, COLOR_SET_0_RATIO, 0, 0xFF], #GREEN
            [COLOR_SET_0_RATIO, COLOR_SET_0_RATIO, 0, 0xFF], #BROWN
            [0, 0, COLOR_SET_0_RATIO, 0xFF], #BLUE
            [COLOR_SET_0_RATIO, 0, COLOR_SET_0_RATIO, 0xFF], #MAGENTA
            [0, COLOR_SET_0_RATIO, COLOR_SET_0_RATIO, 0xFF], #CYAN
            [COLOR_SET_0_RATIO, COLOR_SET_0_RATIO, COLOR_SET_0_RATIO, 0xFF], #LIGHT GRAY
            [COLOR_SET_1_RATIO, COLOR_SET_1_RATIO, COLOR_SET_1_RATIO, 0xFF], #DARK_GREY
            [0xFF, COLOR_SET_1_RATIO, COLOR_SET_1_RATIO, 0xFF], #RED
            [COLOR_SET_1_RATIO, 0xFF, COLOR_SET_1_RATIO, 0xFF], #GREEN
            [0xFF, 0xFF, COLOR_SET_1_RATIO, 0xFF], #YELLOW
            [COLOR_SET_1_RATIO, COLOR_SET_1_RATIO, 0xFF, 0xFF], #BLUE
            [0xFF, COLOR_SET_1_RATIO, 0xFF, 0xFF], #MAGENTA
            [COLOR_SET_1_RATIO, 0xFF, 0xFF, 0xFF], #CYAN
            [0xFF, 0xFF, 0xFF, 0xFF], #WHITE
        ]

        self._values[str(TermCell.ColorIndex.DefaultForeColorIndex)] = self.__from_array_to_color([0x00,0x00,0x00,0x88])
        self._values[str(TermCell.ColorIndex.DefaultBackColorIndex)] = self.__from_array_to_color([0xdd,0xdd,0xdd,0xFF])
        self._values[str(TermCell.ColorIndex.DefaultCursorColorIndex)] = self.__from_array_to_color([0x00,0x00,0x00,0x88])

        for i in range(len(COLOR_TABLE)):
            self._values[str(i)] = self.__from_array_to_color(COLOR_TABLE[i])

        for i in range(240):
            if i < 216:
                r = i / 36
                g = (i / 6) % 6
                b = i % 6
                self._values[str(i + 16)] = self.__from_array_to_color(
                    [r * 40 + 55 if r > 0 else 0,
                     g * 40 + 55 if g > 0 else 0,
                     b * 40 + 55 if b > 0 else 0,
                     0xFF])
            else:
                shade = (i - 216) * 10 + 8
                self._values[str(i + 16)] = self.__from_array_to_color(
                    [shade,
                     shade,
                     shade,
                     0xFF])

def register_plugins(pm):
    pm.register_plugin(DefaultTermColorTheme().new_instance())
