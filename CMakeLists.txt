# 使用已安装的 QT 套件编译：
#   与 QtCreator 中的构建环境一致。
#   关键操作是通过 `CMAKE_TOOLCHAIN_FILE` 指向 `qt.toolchain.cmake` 所在路径来指定编译套件。
#   > cmake -S. -Bbuildx -DCMAKE_TOOLCHAIN_FILE="C:\app\dev\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6\qt.toolchain.cmake"
#   > cmake --build buildx
#   > C:\app\dev\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe buildx\Debug\TaoTieChat.exe

cmake_minimum_required(VERSION 3.19)
# project(TaoTieChat LANGUAGES CXX)

# =================================   set   ================================= #

set(PROJECT_NAME TaoTieChat)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMAKE_BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    # set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

# # add_definitions(-DSOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")
# add_compile_definitions(SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")
add_compile_definitions(PROJECT_NAME=${PROJECT_NAME})

# =============================     project      ============================= #

# project 之后才能正常 find_package
project(${PROJECT_NAME} LANGUAGES CXX)

# =============================   packages   ============================= #

# # grpc 子模块路径太长，这种方法不可用
# include(FetchContent)
# FetchContent_Declare(
#     grpc
#     GIT_REPOSITORY https://github.com/grpc/grpc.git
#     GIT_TAG        v1.76.0
# )
# FetchContent_MakeAvailable(grpc)

# find_package(gRPC CONFIG REQUIRED)
# target_link_libraries(main PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Svg SvgWidgets Network)
# find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Core Widgets LinguistTools)
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES zh_CN
)

# =============================     message      ============================= #

message(STATUS "CMAKE_TOOLCHAIN_FILE : ${CMAKE_TOOLCHAIN_FILE}")
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
# message(STATUS "ENV{CMAKE_TOOLCHAIN_FILE} : $ENV{CMAKE_TOOLCHAIN_FILE}")

# include(src/cmake/show_env.cmake)
# show_env_before_project()
# show_env_extern_before_project()
# show_env_after_project()

# =========================== sources  =========================== #

set(SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    # Utils / data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Dev.h              # 开发时的调试代码
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Dev.cpp            # 开发时的调试代码
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RegistryFactory.h  # 暂未使用
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data/data.h
    # 主窗口
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow/MainWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow/MainWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow/MainWindow.ui
    # 登录/认证窗口
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow/AuthWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow/AuthWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow/AuthWindow.ui
    # 聊天窗口，聊天/联系人列表及内容页面
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ChatWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ChatWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ChatWindow.ui
    # 消息窗口，显示聊天对话框
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/MessageWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/MessageWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/MessageWindow.ui
    # 聊天窗口的联系人列表 model/view 实现
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ContactModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ContactModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ContactDelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ContactDelegate.cpp
    # 联系人信息页面，与 ContactModel/ContactDelegate 无关
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ContactWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ContactWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow/ContactWindow.ui
    # PluginsWindow
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PluginsWindow/MediaWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PluginsWindow/MediaWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PluginsWindow/MediaWindow.ui
)

set(TS_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/${PROJECT_NAME}_zh_CN.ts)
# set(QM_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/${PROJECT_NAME}_zh_CN.qm)

# 程序中需调用 `Q_INIT_RESOURCE(...);` 资源初始化函数，防止与子项目中的资源冲突
set(QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/rc/${PROJECT_NAME}.qrc)

# if(WIN32)
#     set(RC_FILES  ${PROJECT_NAME}.rc)
#     # list(APPEND PROJECT_SOURCES ${RC_FILES})
# endif()

# set(PROJECT_SOURCES
#     ${SRC_FILE}
#     ${TS_FILE}
#     ${QRC_FILE}
# )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/data)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PluginsWindow)

# ===========================        builds         =========================== #

qt_add_executable(${PROJECT_NAME}
    WIN32 MACOSX_BUNDLE
    ${SRC_FILES}
    ${TS_FILES}
    ${QRC_FILES}
)

qt_add_translations(
    TARGETS  ${PROJECT_NAME}
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE OUTPUT_QM_FILES
    QM_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/rc/translations"
)
message(STATUS "TS_FILES             : ${TS_FILES}")
message(STATUS "OUTPUT_QM_FILES      : ${OUTPUT_QM_FILES}")

qt_add_lupdate(
    ${PROJECT_NAME}
    TS_FILES ${TS_FILES}
)
message(STATUS "update to ts command : `${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target update_translations`")

# qt_add_resources(${PROJECT_NAME} "translations"
#     PREFIX "/translations"
#     FILES ${OUTPUT_QM_FILES}
#     QT_RESOURCE_ALIAS
# )

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Svg
        Qt::SvgWidgets
        Qt::Network
)

# target_compile_definitions(TaoTieChat PUBLIC SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")

include(GNUInstallDirs)

install(
    TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET        ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# function(run_windeployqt binary_target)
#     # QT runtime error : `This application failed to start because no Qt platform plugin could be initialized. Reinstalling the application may fix this problem.`
#     # [No Qt6 platform plugin could be initialized · Issue #20101 · microsoft/vcpkg](https://github.com/microsoft/vcpkg/issues/20101#issuecomment-1077747306)
#     if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#         find_program(TOOL_WINDEPLOYQT NAMES windeployqt.debug.bat)
#     else()
#         find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
#     endif()

#     add_custom_command(TARGET ${binary_target} POST_BUILD
#         COMMAND ${TOOL_WINDEPLOYQT}
#         $<TARGET_FILE:${binary_target}>
#         COMMENT "Running windeployqt..."
#     )
# endfunction(run_windeployqt)
# set(DEPLOY_RUNTION ON CACHE BOOL "Whether to run windeployqt after build")
# if(DEPLOY_RUNTION AND WIN32)
#     run_windeployqt(${CMAKE_PROJECT_NAME})
# endif()
