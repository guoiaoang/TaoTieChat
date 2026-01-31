cmake_minimum_required(VERSION 3.19)
project(TaoTieChat LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_BUILD_TYPE     : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_GENERATOR      : ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_CXX_FLAGS_INIT : ${CMAKE_CXX_FLAGS_INIT}")
message(STATUS "CMAKE_COMMAND        : ${CMAKE_COMMAND}")
message(STATUS "CMAKE_CPACK_COMMAND  : ${CMAKE_CPACK_COMMAND}")
message(STATUS "CMAKE_CTEST_COMMAND  : ${CMAKE_CTEST_COMMAND}")
message(STATUS "CMAKE_MAKE_PROGRAM   : ${CMAKE_MAKE_PROGRAM}")
message(STATUS "CMAKE_C_COMPILER     : ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER   : ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_PREFIX_PATH    : ${CMAKE_PREFIX_PATH}")
message(STATUS "QT_QMAKE_EXECUTABLE  : ${QT_QMAKE_EXECUTABLE}")
message(STATUS "QT_MAINTENANCE_TOOL  : ${QT_MAINTENANCE_TOOL}")
# message(STATUS "CMAKE_BUILD_TOOL     : ${CMAKE_BUILD_TOOL}") # after project()

# include(src/cmake/show_env.cmake)
# show_env_before_project()
# show_env_extern_before_project()
# show_env_after_project()

# # add_definitions(-DSOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")
# add_compile_definitions(SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")

# if (MSVC)
#     # warning level 4
#     add_compile_options(/W4)
# else()
#     # additional warnings
#     add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow)

qt_add_executable(TaoTieChat
    WIN32 MACOSX_BUNDLE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow/MainWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow/MainWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow/MainWindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow/AuthWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow/AuthWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow/AuthWindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ChatWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ChatWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ChatWindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemChat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemChat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemChat.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemContent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemContent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemContent.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/MessageWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/MessageWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/MessageWindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemMessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ItemMessage.ui
)

qt_add_translations(
    TARGETS TaoTieChat
    TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/TaoTieChat_zh_CN.ts
)

target_link_libraries(TaoTieChat
    PRIVATE
        Qt::Core
        Qt::Widgets
)

# target_compile_definitions(TaoTieChat PUBLIC SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")

include(GNUInstallDirs)

install(TARGETS TaoTieChat
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET TaoTieChat
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
