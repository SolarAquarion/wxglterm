ADD_LIBRARY(wxglterm_ui
    scope_locker.h
    draw_pane.h draw_pane.cpp
    main_dlg.cpp        main_dlg.h
    draw_pane_key.cpp draw_pane_draw.cpp
    draw_pane_mouse.cpp
)

ADD_DEPENDENCIES(wxglterm_ui pybind11)

TARGET_COMPILE_DEFINITIONS(wxglterm_ui PRIVATE
    ${wxWidgets_DEFINITIONS}
    )
TARGET_INCLUDE_DIRECTORIES(wxglterm_ui PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
    "${pybind11_INCLUDE_DIRS}"
    "${PYTHON_INCLUDE_DIRS}"
    "../../../controller"
    "../../../interface"
    "../../../utils"
    )
