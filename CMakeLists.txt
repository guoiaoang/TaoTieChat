# 使用已安装的 QT 套件编译：
#   与 QtCreator 中的构建环境一致。
#   关键操作是通过 `CMAKE_TOOLCHAIN_FILE` 指向 `qt.toolchain.cmake` 所在路径来指定编译套件。
#   > cmake -S. -Bbuildx -DCMAKE_TOOLCHAIN_FILE="C:\app\dev\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6\qt.toolchain.cmake"
#   > cmake --build buildx
#   > C:\app\dev\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe buildx\Debug\TaoTieChat.exe
#
# 客户端执行 `cmake --workflow ...` 时可能需要手动执行 `install` 操作

cmake_minimum_required(VERSION 3.10)

# =================================   set   ================================= #

# 设置项目名称
set(PROJECT_NAME       TaoTieChat)
set(PROJECT_VERSION    "0.1.0")
set(PROJECT_DESC       "TaoTieChat")
# set(CLIENT_NAME      ${PROJECT_NAME})
set(LIB_PROTO_NAME     LibProtoTaoTieChat)
set(LIB_QT_GRPC_NAME   LibQtGrpcTaoTieChat)
set(PROTO_FILES        "${CMAKE_CURRENT_SOURCE_DIR}/src/protos/taotie_chat.proto")
set(PROTO_HEADERS_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src/protos/include/")

# QT 文件自动编译
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD          20)
# 要求使用指定的 C++ 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMAKE_BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    # set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    add_compile_definitions(_DEBUG)
endif()

# # add_definitions(-DSOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")
# add_compile_definitions(SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")
add_compile_definitions(PROJECT_NAME=${PROJECT_NAME})
add_compile_definitions(PROJECT_VERSION=${PROJECT_VERSION})
add_compile_definitions(ENABLE_CLIENT=ON)

# Policy CMP0087 is not set: Install CODE|SCRIPT allow the use of generator expressions.
# Run "cmake --help-policy CMP0087" for policy details.
if(POLICY CMP0087)
    cmake_policy(SET CMP0087 NEW)
endif()

# MSVC 编译器特定设置
if(MSVC)
    # add_definitions(-D_WIN32_WINNT=0x600)
    add_compile_definitions(_WIN32_WINNT=0x600)
    # 强制编译器以 UTF-8 编码读取所有源文件
    add_compile_options(/utf-8)
endif()

set(SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    # Utils / data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Headers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Dev.h              # 开发时的调试代码
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Dev.cpp            # 开发时的调试代码
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RegistryFactory.h  # 暂未使用
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data/data.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data/Accounts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data/Accounts.cpp
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
    # # 搜索页面
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/SearchWindow/SearchWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/SearchWindow/SearchWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/SearchWindow/SearchWindow.ui
    # network 与服务器之间的通信服务
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network/NetworkClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network/NetworkClient.cpp
    # # third
    # ${CMAKE_CURRENT_SOURCE_DIR}/third/simpleQtLogger/simpleQtLogger.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/third/simpleQtLogger/simpleQtLogger.cpp
)

set(TS_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/${PROJECT_NAME}_zh_CN.ts)
# set(QM_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/${PROJECT_NAME}_zh_CN.qm)

# 程序中需调用 `Q_INIT_RESOURCE(...);` 资源初始化函数，防止与子项目中的资源冲突
set(QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/rc/${PROJECT_NAME}.qrc)

# if(WIN32)
#     set(RC_FILES  ${PROJECT_NAME}.rc)
#     # list(APPEND PROJECT_SOURCES ${RC_FILES})
# endif()

# 设置 proto 文件路径和生成的头文件目录
set(PROTO_FILES         "${CMAKE_CURRENT_SOURCE_DIR}/src/protos/taotie_chat.proto")
set(PROTO_HEADERS_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/src/protos/include/")

# 定义源文件列表
set(SOURCE_FILES
    ${SRC_FILES}
    ${TS_FILES}
    ${QRC_FILES}
)

# 添加源码目录，便于 #include 引用
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/data)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/network)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AuthWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ChatWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PluginsWindow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui/SearchWindow)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third/simpleQtLogger)
# 将生成的头文件目录添加到 include 路径中，以便在代码中引用生成的 .pb.h 文件
include_directories(${PROTO_HEADERS_DIR})

# =============================     project      ============================= #

# 项目初始化 # vcpkg 在执行 project 之后才能正常 find_package
project(${PROJECT_NAME}
    LANGUAGES CXX
    VERSION   "${PROJECT_VERSION}"
)

# =============================   find_package   ============================= #

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

find_package(Qt6 6.5 REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets
        Network
        LinguistTools
        SvgWidgets
        Svg
        Protobuf
        ProtobufQtCoreTypes
        ProtobufQuick
        Grpc
        GrpcQuick
)
# `find_package(QT NAMES Qt6 Qt5 REQUIRED)` 必须，用于设置 `QT_VERSION_MAJOR`，便于部署脚本的判断，否则不会执行 `qt_generate_deploy_app_script`
find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Core Widgets LinguistTools)
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools)
# find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Svg SvgWidgets Network Grpc Protobuf)

# 查找依赖的包
find_package(fmt           CONFIG REQUIRED)
find_package(absl          CONFIG REQUIRED)
find_package(spdlog        CONFIG REQUIRED)
find_package(glog          CONFIG REQUIRED)
find_package(sqlcipher     CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost         CONFIG REQUIRED COMPONENTS uuid)
# find_package(protobuf      CONFIG REQUIRED) # 使用 Qt 实现的 protobuf
# find_package(gRPC          CONFIG REQUIRED) # 使用 Qt 实现的 gRPC

# ===========================    qt_add_    =========================== #

# qt_standard_project_setup(REQUIRES 6.8)
qt_standard_project_setup(
    REQUIRES
        6.8
    I18N_SOURCE_LANGUAGE
        en
    I18N_TRANSLATED_LANGUAGES
        zh_CN
)

# ============================= add_executable ============================= #

# 主可执行程序
qt_add_executable(${PROJECT_NAME}
    WIN32
    MACOSX_BUNDLE
    ${SOURCE_FILES}
)

# ========================== target_link_libraries ========================== #

# 链接依赖库
target_link_libraries(${PROJECT_NAME}   PRIVATE   fmt::fmt)
target_link_libraries(${PROJECT_NAME}   PRIVATE   absl::any absl::log absl::base absl::bits)  # 需要根据实际使用的 absl 模块调整
target_link_libraries(${PROJECT_NAME}   PRIVATE   spdlog::spdlog)
target_link_libraries(${PROJECT_NAME}   PRIVATE   glog::glog)
target_link_libraries(${PROJECT_NAME}   PRIVATE   sqlcipher::sqlcipher)
target_link_libraries(${PROJECT_NAME}   PRIVATE   nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME}   PRIVATE   Boost::uuid)
# target_link_libraries(${PROJECT_NAME} PRIVATE   protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
# target_link_libraries(${PROJECT_NAME} PRIVATE   gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

# target_compile_definitions(TaoTieChat PUBLIC SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src")

# =============================     message      ============================= #

message(STATUS "")
message(STATUS "# ==== ==== ==== ==== messages ==== ==== ==== ==== #")
message(STATUS "    CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
message(STATUS "     CMAKE_GENERATOR : ${CMAKE_GENERATOR}")
message(STATUS "    CMAKE_BUILD_TOOL : ${CMAKE_BUILD_TOOL}")
message(STATUS "  CMAKE_MAKE_PROGRAM : ${CMAKE_MAKE_PROGRAM}")
message(STATUS "       CMAKE_COMMAND : ${CMAKE_COMMAND}")
message(STATUS " CMAKE_CPACK_COMMAND : ${CMAKE_CPACK_COMMAND}")
message(STATUS " CMAKE_CTEST_COMMAND : ${CMAKE_CTEST_COMMAND}")
message(STATUS "    CMAKE_C_COMPILER : ${CMAKE_C_COMPILER}")
message(STATUS "  CMAKE_CXX_COMPILER : ${CMAKE_CXX_COMPILER}")
message(STATUS "        CMAKE_LINKER : ${CMAKE_LINKER}")
message(STATUS "    CMAKE_LINKER_LLD : ${CMAKE_LINKER_LLD}")
message(STATUS "CMAKE_TOOLCHAIN_FILE : ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "  CMAKE_PROGRAM_PATH : ${CMAKE_PROGRAM_PATH}")
message(STATUS "   CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")
message(STATUS "         Qt6_VERSION : ${Qt6_VERSION}")
message(STATUS "             Qt6_DIR : ${Qt6_DIR}")
message(STATUS " QT_QMAKE_EXECUTABLE : ${QT_QMAKE_EXECUTABLE}")
message(STATUS " QT_MAINTENANCE_TOOL : ${QT_MAINTENANCE_TOOL}")
message(STATUS "update to ts command : `${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target update_translations`")
message(STATUS "     install command : `${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix=${CMAKE_INSTALL_PREFIX}`")
message(STATUS "                      Qt6Protobuf_CONFIG : ${Qt6Protobuf_CONFIG}")
message(STATUS "                          Qt6Grpc_CONFIG : ${Qt6Grpc_CONFIG}")
message(STATUS "                            Protobuf_DIR : ${Protobuf_DIR}")
message(STATUS "              Protobuf_PROTOC_EXECUTABLE : ${Protobuf_PROTOC_EXECUTABLE}")
message(STATUS "                         Protobuf_CONFIG : ${Protobuf_CONFIG}")
message(STATUS "                      Z_VCPKG_EXECUTABLE : ${Z_VCPKG_EXECUTABLE}")
message(STATUS "                   Z_VCPKG_TOOLCHAIN_DIR : ${Z_VCPKG_TOOLCHAIN_DIR}")
message(STATUS "     Z_VCPKG_INSTALLED_DIR_INITIAL_VALUE : ${Z_VCPKG_INSTALLED_DIR_INITIAL_VALUE}")
message(STATUS " Z_NATIVE_VCPKG_MANIFEST_INSTALL_LOGFILE : ${Z_NATIVE_VCPKG_MANIFEST_INSTALL_LOGFILE}")
# message(STATUS "CMAKE_VS_DEVENV_COMMAND                  : ${CMAKE_VS_DEVENV_COMMAND}")
# message(STATUS "CMAKE_VS_MSBUILD_COMMAND                 : ${CMAKE_VS_MSBUILD_COMMAND}")
# message(STATUS "CMAKE_VS_PLATFORM_NAME                   : ${CMAKE_VS_PLATFORM_NAME}")
# message(STATUS "CMAKE_VS_PLATFORM_TOOLSET                : ${CMAKE_VS_PLATFORM_TOOLSET}")
# message(STATUS "CMAKE_VS_VERSION_BUILD_NUMBER            : ${CMAKE_VS_VERSION_BUILD_NUMBER}")
# message(STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION : ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

# include(src/cmake/show_env.cmake)
# show_env_before_project()
# show_env_extern_before_project()
# show_env_after_project()
# show_ext_variants()

# ======================= 生成 gRPC 和 Protobuf 代码 ======================= #

qt_add_protobuf(${LIB_PROTO_NAME}
    # GENERATE_PACKAGE_SUBFOLDERS
    OUTPUT_DIRECTORY
        ${PROTO_HEADERS_DIR}
    PROTO_FILES
        ${PROTO_FILES}
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufQtCoreTypes,QT_PROTO_INCLUDES>
        ${PROTO_HEADER_DIR}
)

install(TARGETS ${LIB_PROTO_NAME})

qt_add_grpc(${LIB_QT_GRPC_NAME}   CLIENT
    # GENERATE_PACKAGE_SUBFOLDERS
    OUTPUT_DIRECTORY
        ${PROTO_HEADERS_DIR}
    PROTO_FILES
        ${PROTO_FILES}
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufQtCoreTypes,QT_PROTO_INCLUDES>
        ${PROTO_HEADER_DIR}
)

qt_add_translations(
    TARGETS
        ${PROJECT_NAME}
    TS_FILES
        ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE
        OUTPUT_QM_FILES
    # QM_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/rc/translations"
)
message(STATUS "OUTPUT_QM_FILES      : ${OUTPUT_QM_FILES}")

qt_add_lupdate(
    ${PROJECT_NAME}
    TS_FILES
        ${TS_FILES}
)

# qt_add_resources(${PROJECT_NAME}   "translations"
#     PREFIX   "/translations"
#     FILES    ${OUTPUT_QM_FILES}
#     QT_RESOURCE_ALIAS
# )

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Svg
        Qt::SvgWidgets
        Qt::Network
        Qt::Grpc
        Qt::Protobuf
        ${LIB_PROTO_NAME}
        ${LIB_QT_GRPC_NAME}
)

# =============================== 安装打包 =============================== #

include(GNUInstallDirs)

install(
    TARGETS
        ${PROJECT_NAME}
    BUNDLE
        DESTINATION   .
    RUNTIME
        DESTINATION   ${CMAKE_INSTALL_BINDIR}
    LIBRARY
        DESTINATION   ${CMAKE_INSTALL_LIBDIR}
)

# install 与 package 步骤并不相关

# 复制 build 目录下生成的文件 # 复制 grpc 运行时需要的相关依赖文件
if(WIN32)
    # Some dependencies are copied by the IDE/toolchain into the per-config
    # output folder (for example: out/build/.../Debug). Include them in install.

    # 复制 .dll / .pdb 文件到可执行文件目录
    install(
        DIRECTORY   "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        FILES_MATCHING      # 过滤条件
            PATTERN "*.dll" # 复制 .dll 文件
            PATTERN "*.pdb" # 复制 .pdb 文件
    )
endif()

# 复制系统运行时库到可执行文件目录
if(WIN32)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE) # 跳过系统运行时库的安装 # 貌似没有区别
    set(CMAKE_INSTALL_UCRT_LIBRARIES           TRUE) # 安装 UCP_UCRT_LIBRARIES 库

    # 安装系统运行时库
    include(InstallRequiredSystemLibraries)

    # 复制系统运行时库到可执行文件目录
    if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endif()

# 生成部署脚本 # 执行 win_deploy_qt 功能
if(WIN32 AND QT_VERSION_MAJOR EQUAL 6)
    message(STATUS "execute win_deploy_qt")
    qt_generate_deploy_app_script(
        TARGET
            ${PROJECT_NAME}
        OUTPUT_SCRIPT
            qt_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${qt_deploy_script})
else()
    message(STATUS "not WIN32 OR QT6, will not execute qt_deploy_script!")
endif()

# 完成可执行文件的构建
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

# 打包项目
set(CPACK_PACKAGE_NAME                "${PROJECT_NAME}"          ) # 包名
set(CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}"       ) # 版本号
set(CPACK_PACKAGE_VENDOR              "${PROJECT_NAME}"          ) # 包供应商
set(CPACK_PACKAGE_INSTALL_DIRECTORY   "${PROJECT_NAME}"          ) # 安装目录
set(CPACK_PACKAGE_DIRECTORY           "${CMAKE_BINARY_DIR}/setup") # 安装文件保存路径
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESC}"          ) # 包摘要

# 包含许可证文件
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()

# 生成压缩包
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()
# Generators
#   7Z        = 7-Zip file format
#   DEB       = Debian packages
#   External  = CPack External packages
#   IFW       = Qt Installer Framework
#   INNOSETUP = Inno Setup packages
#   NSIS      = Null Soft Installer
#   NSIS64    = Null Soft Installer (64-bit)
#   NuGet     = NuGet packages
#   STGZ      = Self extracting Tar GZip compression
#   TAR       = Tar no compression
#   TBZ2      = Tar BZip2 compression
#   TGZ       = Tar GZip compression
#   TXZ       = Tar XZ compression
#   TZ        = Tar Compress compression
#   TZST      = Tar Zstandard compression
#   WIX       = MSI file format via WiX tools
#   ZIP       = ZIP file format

# vcpkg 相关依赖
# 如果是 Windows，通常建议包含这个以处理依赖
if(WIN32)
    set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
endif()

include(CPack)
