cmake_minimum_required(VERSION 3.19)
project(plan-and-track LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Charts)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(plan-and-track
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    database.h database.cpp
    utils.h utils.cpp
    addhabitdialog.h addhabitdialog.cpp addhabitdialog.ui
    addtaskdialog.h addtaskdialog.cpp addtaskdialog.ui
    delegates/datedelegate.h delegates/datedelegate.cpp
    delegates/habitfrequencydelegate.h delegates/habitfrequencydelegate.cpp
    delegates/habitstatusdelegate.h delegates/habitstatusdelegate.cpp
    delegates/plannamedelegate.h delegates/plannamedelegate.cpp
    delegates/planstatusdelegate.h delegates/planstatusdelegate.cpp
    delegates/taskstatusdelegate.h delegates/taskstatusdelegate.cpp
    models/habitmodel.h models/habitmodel.cpp
    models/planmodel.h models/planmodel.cpp
    models/taskmodel.h models/taskmodel.cpp
    resources.qrc
    photosurface.rc
)

target_link_libraries(plan-and-track
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Sql
        Qt6::Charts
)

include(GNUInstallDirs)

install(TARGETS plan-and-track
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET plan-and-track
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
