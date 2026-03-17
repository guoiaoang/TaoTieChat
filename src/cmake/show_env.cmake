function(show_env_before_project)
    # 输出系统构建环境
    message(STATUS " ================== Build Environment ================== ")
    message(STATUS "     CMake execute path: ${CMAKE_COMMAND} (version:${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}.${CMAKE_TWEAK_VERSION})")
    # message(STATUS "         CMake version: ${CMAKE_VERSION}")
    message(STATUS "     CPack execute path: ${CMAKE_CPACK_COMMAND}")          # C:/Program Files/CMake/bin/cpack.exe
    message(STATUS "     CTest execute path: ${CMAKE_CTEST_COMMAND}")          # C:/Program Files/CMake/bin/ctest.exe
    message(STATUS "        CMake root path: ${CMAKE_ROOT}")                   # C:/Program Files/CMake/share/cmake-4.2
    message(STATUS "   CMake toolchain path: ${CMAKE_TOOLCHAIN_FILE}")         # C:/a/t/vcpkg/scripts/buildsystems/vcpkg.cmake
    message(STATUS "             Build type: ${CMAKE_BUILD_TYPE}")             # Debug
    message(STATUS "   Build generator name: ${CMAKE_GENERATOR}")              # Visual Studio 17 2022
    message(STATUS "          Platfrom name: ${CMAKE_HOST_SYSTEM_NAME}")       # Windows
    message(STATUS "Platfrom execute suffix: ${CMAKE_HOST_EXECUTABLE_SUFFIX}") # .exe
    message(STATUS "    Platfrom is Windows: ${CMAKE_HOST_WIN32}")             # 1
    message(STATUS "                  Win32: ${WIN32}")                        # 1
    message(STATUS "    Project source path: ${CMAKE_HOME_DIRECTORY}")         # C:/workspace/projects/tmp/TaoTieChat
    message(STATUS "             Build path: ${CMAKE_BINARY_DIR}")             # C:/workspace/projects/tmp/TaoTieChat/out/build/windows_config
    message(STATUS "    Install prefix path: ${CMAKE_INSTALL_PREFIX}")         # C:/workspace/projects/tmp/TaoTieChat/out/install/windows_config
    message(STATUS "      Project list path: ${CMAKE_CURRENT_LIST_FILE}")      # C:/workspace/projects/tmp/TaoTieChat/CMakeLists.txt
    message(STATUS " ======================================================== ")
endfunction(show_env_before_project)

function(show_env_extern_before_project)
    message(STATUS " ===================== Extern Info ===================== ")
    message(STATUS "                  CMake files dir: ${CMAKE_FILES_DIRECTORY}")            # /CMakeFiles
    message(STATUS "             CMake list file name: ${CMAKE_LIST_FILE_NAME}")             # CMakeLists.txt
    message(STATUS "          CMake current list path: ${CMAKE_CURRENT_LIST_DIR}")           # C:/workspace/projects/tmp/TaoTieChat
    message(STATUS "        CMake current source path: ${CMAKE_CURRENT_SOURCE_DIR}")         # C:/workspace/projects/tmp/TaoTieChat
    message(STATUS "                CMake source path: ${CMAKE_SOURCE_DIR}")                 # C:/workspace/projects/tmp/TaoTieChat
    message(STATUS "           CMake parent list file: ${CMAKE_PARENT_LIST_FILE}")           # C:/workspace/projects/tmp/TaoTieChat/CMakeLists.txt
    message(STATUS "            CMake cache file path: ${CMAKE_CACHEFILE_DIR}")              # C:/workspace/projects/tmp/TaoTieChat/out/build/windows_config
    message(STATUS "        CMake current binary path: ${CMAKE_CURRENT_BINARY_DIR}")         # C:/workspace/projects/tmp/TaoTieChat/out/build/windows_config
    message(STATUS "CMake find package redirects path: ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}") # C:/workspace/projects/tmp/TaoTieChat/out/build/windows_config/CMakeFiles/pkgRedirects
    message(STATUS " ======================================================== ")
endfunction(show_env_extern_before_project)

function(show_env_after_project)
    message(STATUS " ============= Project Info After project() ============= ")
    message(STATUS "\${CMAKE_VERSION...}                   : ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}.${CMAKE_PROJECT_VERSION_TWEAK}") # 0.1..
    message(STATUS "\${CMAKE_GENERATOR_INSTANCE}           : ${CMAKE_GENERATOR_INSTANCE}")           # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise
    message(STATUS "\${CMAKE_BUILD_TOOL}                   : ${CMAKE_BUILD_TOOL}")                   # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin/amd64/MSBuild.exe
    message(STATUS "\${CMAKE_MAKE_PROGRAM}                 : ${CMAKE_MAKE_PROGRAM}")                 # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin/amd64/MSBuild.exe
    message(STATUS "\${CMAKE_CXX_COMPILER}                 : ${CMAKE_CXX_COMPILER}")                 # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe
    message(STATUS "\${CMAKE_AR}                           : ${CMAKE_AR}")                           # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/lib.exe
    message(STATUS "\${CMAKE_LINKER}                       : ${CMAKE_LINKER}")                       # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/link.exe
    message(STATUS "\${CMAKE_CXX_COMPILER_LINKER}          : ${CMAKE_CXX_COMPILER_LINKER}")          # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/HostX64/x64/link.exe
    message(STATUS "\${CMAKE_LINKER}                       : ${CMAKE_LINKER}")                       # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/link.exe
    message(STATUS "\${CMAKE_LINKER_LINK}                  : ${CMAKE_LINKER_LINK}")                  # : C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/link.exe
    message(STATUS "\${CMAKE_FIND_ROOT_PATH}               : ${CMAKE_FIND_ROOT_PATH}")               # : C:/a/t/vcpkg/installed/x64-windows/debug;C:/a/t/vcpkg/installed/x64-windows
    message(STATUS "\${CMAKE_SYSTEM_PREFIX_PATH}           : ${CMAKE_SYSTEM_PREFIX_PATH}")           # : C:/Program Files;C:/Program Files (x86);C:/Program Files/CMake;C:/workspace/projects/tmp/TaoTieChat/out/install/windows_config
    message(STATUS "\${CMAKE_PROJECT_NAME}                 : ${CMAKE_PROJECT_NAME}")                 # : TaoTieChat
    message(STATUS "\${CMAKE_PROJECT_DESCRIPTION}          : ${CMAKE_PROJECT_DESCRIPTION}")          # : A chat application powered by gRPC and Qt
    message(STATUS "\${CMAKE_PROJECT_HOMEPAGE_URL}         : ${CMAKE_PROJECT_HOMEPAGE_URL}")         # : https://github.com/guoiaoang/TaoTieChat
    message(STATUS "\${TaoTieChat_BINARY_DIR}}             : ${TaoTieChat_BINARY_DIR}}")             # : C:/workspace/projects/tmp/TaoTieChat/out/build/windows_config}
    message(STATUS "\${TaoTieChat_IS_TOP_LEVEL}            : ${TaoTieChat_IS_TOP_LEVEL}")            # : ON
    message(STATUS "\${TaoTieChat_SOURCE_DIR}              : ${TaoTieChat_SOURCE_DIR}")              # : C:/workspace/projects/tmp/TaoTieChat
    message(STATUS "\${VCPKG_ROOT}                         : ${VCPKG_ROOT}")                         # :
    message(STATUS "\$ENV{VCPKG_ROOT}                      : $ENV{VCPKG_ROOT}")                      # : C:/a/t/vcpkg
    message(STATUS "\$ENV{PATH}                            : $ENV{PATH}")                            # : ...
    message(STATUS "\${VCPKG_INSTALLED_DIR}                : ${VCPKG_INSTALLED_DIR}")                # : C:/a/t/vcpkg/installed
    message(STATUS "\${TTC_PROJECT_MANIFEST_LAST_DIR}                 : ${TTC_PROJECT_MANIFEST_LAST_DIR}")                 # :
    message(STATUS "\${VCPKG_TARGET_TRIPLET}               : ${VCPKG_TARGET_TRIPLET}")               # : x64-windows
    message(STATUS "\${CMAKE_BUILD_TYPE}                   : ${CMAKE_BUILD_TYPE}")                   # : Debug
    message(STATUS "\${CMAKE_CXX_BYTE_ORDER}               : ${CMAKE_CXX_BYTE_ORDER}")               # : LITTLE_ENDIAN
    message(STATUS "\${CMAKE_FIND_LIBRARY_SUFFIXES}        : ${CMAKE_FIND_LIBRARY_SUFFIXES}")        # : .dll.lib;.lib;.a
    message(STATUS "\${CMAKE_HOST_EXECUTABLE_SUFFIX}       : ${CMAKE_HOST_EXECUTABLE_SUFFIX}")       # : .exe
    message(STATUS "\${CMAKE_CXX_COMPILER_ID}              : ${CMAKE_CXX_COMPILER_ID}")              # : MSVC
    message(STATUS "\${CMAKE_CXX_PLATFORM_LINKER_ID}       : ${CMAKE_CXX_PLATFORM_LINKER_ID}")       # : MSVC
    message(STATUS "\${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} : ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}") # : x64
    message(STATUS "\${CMAKE_SYSTEM_PROCESSOR}             : ${CMAKE_SYSTEM_PROCESSOR}")             # : AMD64
    message(STATUS "\${CMAKE_HOST_SYSTEM_PROCESSOR}        : ${CMAKE_HOST_SYSTEM_PROCESSOR}")        # : AMD64
    message(STATUS "\${CMAKE_SYSTEM_NAME}                  : ${CMAKE_SYSTEM_NAME}")                  # : Windows
    message(STATUS "\${CMAKE_HOST_SYSTEM_NAME}             : ${CMAKE_HOST_SYSTEM_NAME}")             # : Windows
    message(STATUS "\${CMAKE_CXX_PLATFORM_ID}              : ${CMAKE_CXX_PLATFORM_ID}")              # : Windows
    message(STATUS "\${CMAKE_SYSTEM}                       : ${CMAKE_SYSTEM}")                       # : Windows-10.0.26100
    message(STATUS "\${CMAKE_HOST_SYSTEM}                  : ${CMAKE_HOST_SYSTEM}")                  # : Windows-10.0.26100
    message(STATUS "\${CMAKE_SYSTEM_VERSION}               : ${CMAKE_SYSTEM_VERSION}")               # : 10.0.26100
    message(STATUS "\${CMAKE_HOST_SYSTEM_VERSION}          : ${CMAKE_HOST_SYSTEM_VERSION}")          # : 10.0.26100
    message(STATUS "\${CMAKE_CXX_COMPILER_VERSION}         : ${CMAKE_CXX_COMPILER_VERSION}")         # : 19.44.35215.0
    message(STATUS "\${CMAKE_CXX_COMPILER_LINKER_VERSION}  : ${CMAKE_CXX_COMPILER_LINKER_VERSION}")  # : 14.44.35215.0
    message(STATUS " ============================================================ ")
endfunction(show_env_after_project)

function(show_ext_variants)
    #
    message(STATUS "TS_FILES : ${TS_FILES}")
    message(STATUS "Qt6Grpc_VERSION : ${Qt6Grpc_VERSION}")
    message(STATUS "grpc : ${GRPC_VERSION}")
    #
    message(STATUS "Qt6ProtobufQtCoreTypes_DIR                  : ${Qt6ProtobufQtCoreTypes_DIR}")
    message(STATUS "Qt6ProtobufQtCoreTypes_CONFIG               : ${Qt6ProtobufQtCoreTypes_CONFIG}")
    message(STATUS "Qt6ProtobufQtCoreTypes_INCLUDE_DIRS         : ${Qt6ProtobufQtCoreTypes_INCLUDE_DIRS}")
    message(STATUS "Qt6ProtobufQtCoreTypes_PRIVATE_INCLUDE_DIRS : ${Qt6ProtobufQtCoreTypes_PRIVATE_INCLUDE_DIRS}")
    message(STATUS "Qt6ProtobufQtCoreTypes_LIBRARIES            : ${Qt6ProtobufQtCoreTypes_LIBRARIES}")
    message(STATUS "Qt6ProtobufTools_CONFIG  : ${Qt6ProtobufTools_CONFIG}")
    message(STATUS "Qt6ProtobufTools_TARGETS : ${Qt6ProtobufTools_TARGETS}")
    message(STATUS "Qt6ProtobufTools_VERSION : ${Qt6ProtobufTools_VERSION}")
    message(STATUS "Qt6Protobuf_DIR          : ${Qt6Protobuf_DIR}")
    message(STATUS "Qt6Protobuf_CONFIG       : ${Qt6Protobuf_CONFIG}")
    message(STATUS "Qt6Protobuf_VERSION      : ${Qt6Protobuf_VERSION}")
    message(STATUS "Qt6Protobuf_INCLUDE_DIRS : ${Qt6Protobuf_INCLUDE_DIRS}")
    message(STATUS "Qt6Protobuf_PRIVATE_INCLUDE_DIRS : ${Qt6Protobuf_PRIVATE_INCLUDE_DIRS}")
    message(STATUS "Qt6ProtobufQuick_DIR     : ${Qt6ProtobufQuick_DIR}")
    message(STATUS "Qt6ProtobufTools_DIR     : ${Qt6ProtobufTools_DIR}")
    message(STATUS "Qt6Grpc_CONFIG           : ${Qt6Grpc_CONFIG}")
    message(STATUS "Qt6Grpc_VERSION          : ${Qt6Grpc_VERSION}")
    message(STATUS "Qt6Grpc_DEFINITIONS      : ${Qt6Grpc_DEFINITIONS}")
    message(STATUS "Qt6Grpc_INCLUDE_DIRS     : ${Qt6Grpc_INCLUDE_DIRS}")
    message(STATUS "Qt6Grpc_PRIVATE_INCLUDE_DIRS : ${Qt6Grpc_PRIVATE_INCLUDE_DIRS}")
    message(STATUS "Qt6Grpc_LIBRARIES        : ${Qt6Grpc_LIBRARIES}")
    message(STATUS "Qt6GrpcTools_CONFIG      : ${Qt6GrpcTools_CONFIG}")
    message(STATUS "Qt6GrpcTools_VERSION     : ${Qt6GrpcTools_VERSION}")
    message(STATUS "Qt6GrpcTools_TARGETS     : ${Qt6GrpcTools_TARGETS}")
    #
    message(STATUS "Protobuf_DIR                : ${Protobuf_DIR}")
    message(STATUS "Protobuf_VERSION            : ${Protobuf_VERSION}")
    message(STATUS "Protobuf_INCLUDE_DIR        : ${Protobuf_INCLUDE_DIR}")
    message(STATUS "Protobuf_PROTOC_EXECUTABLE  : ${Protobuf_PROTOC_EXECUTABLE}")
    message(STATUS "Protobuf_CONFIG             : ${Protobuf_CONFIG}")
    message(STATUS "Protobuf_CONSIDERED_CONFIGS : ${Protobuf_CONSIDERED_CONFIGS}")
    message(STATUS "Protobuf_LIBRARY            : ${Protobuf_LIBRARY}")
    message(STATUS "Protobuf_LIBRARIES          : ${Protobuf_LIBRARIES}")
    message(STATUS "Protobuf_LITE_LIBRARY       : ${Protobuf_LITE_LIBRARY}")
    message(STATUS "Protobuf_LITE_LIBRARIES     : ${Protobuf_LITE_LIBRARIES}")
    message(STATUS "Protobuf_PROTOC_LIBRARY     : ${Protobuf_PROTOC_LIBRARY}")
    message(STATUS "Protobuf_PROTOC_LIBRARIES   : ${Protobuf_PROTOC_LIBRARIES}")
    #
    message(STATUS "PROTO_FILES                 : ${PROTO_FILES}")
    message(STATUS "PROTO_HEADERS_DIR           : ${PROTO_HEADERS_DIR}")
    message(STATUS "PROOTOBUF_VERSION           : ${PROOTOBUF_VERSION}")
    message(STATUS "PROOTOBUF_INCLUDE_DIR       : ${PROOTOBUF_INCLUDE_DIR}")
    message(STATUS "PROOTOBUF_HEADERS_DIR       : ${PROOTOBUF_HEADERS_DIR}")
    message(STATUS "PROOTOBUF_LIBRARY           : ${PROOTOBUF_LIBRARY}")
    message(STATUS "PROOTOBUF_LIBRARIES         : ${PROOTOBUF_LIBRARIES}")
    message(STATUS "PROOTOBUF_LITE_LIBRARY      : ${PROOTOBUF_LITE_LIBRARY}")
    message(STATUS "PROOTOBUF_LITE_LIBRARIES    : ${PROOTOBUF_LITE_LIBRARIES}")
    message(STATUS "PROOTOBUF_PROTOC_LIBRARY    : ${PROOTOBUF_PROTOC_LIBRARY}")
    message(STATUS "PROOTOBUF_PROTOC_LIBRARIES  : ${PROOTOBUF_PROTOC_LIBRARIES}")
    #
    message(STATUS "FIND_PACKAGE_MESSAGE_DETAILS_Protobuf          : ${FIND_PACKAGE_MESSAGE_DETAILS_Protobuf}")
    message(STATUS "FIND_PACKAGE_MESSAGE_DETAILS_Threads           : ${FIND_PACKAGE_MESSAGE_DETAILS_Threads}")
    message(STATUS "FIND_PACKAGE_MESSAGE_DETAILS_WrapAtomic        : ${FIND_PACKAGE_MESSAGE_DETAILS_WrapAtomic}")
    message(STATUS "FIND_PACKAGE_MESSAGE_DETAILS_WrapVulkanHeaders : ${FIND_PACKAGE_MESSAGE_DETAILS_WrapVulkanHeaders}")
    #
    message(STATUS "Qt6_DIR              : ${Qt6_DIR}")
    message(STATUS "Qt6Core_DIR          : ${Qt6Core_DIR}")
    message(STATUS "Qt6CoreTools_DIR     : ${Qt6CoreTools_DIR}")
    message(STATUS "Qt6Gui_DIR           : ${Qt6Gui_DIR}")
    message(STATUS "Qt6GuiTools_DIR      : ${Qt6GuiTools_DIR}")
    message(STATUS "Qt6LinguistTools_DIR : ${Qt6LinguistTools_DIR}")
    message(STATUS "Qt6Network_DIR       : ${Qt6Network_DIR}")
    message(STATUS "Qt6OpenGL_DIR        : ${Qt6OpenGL_DIR}")
    message(STATUS "Qt6Svg_DIR           : ${Qt6Svg_DIR}")
    message(STATUS "Qt6SvgWidgets_DIR    : ${Qt6SvgWidgets_DIR}")
    message(STATUS "Qt6Widgets_DIR       : ${Qt6Widgets_DIR}")
    message(STATUS "Qt6WidgetsTools_DIR  : ${Qt6WidgetsTools_DIR}")
    message(STATUS "Qt6_CONFIG           : ${Qt6_CONFIG}")
    message(STATUS "Qt6_VERSION          : ${Qt6_VERSION}")
    #
    message(STATUS "absl_DIR     : ${absl_DIR}")
    message(STATUS "absl_CONFIG  : ${absl_CONFIG}")
    message(STATUS "absl_VERSION : ${absl_VERSION}")
    #
    message(STATUS "CMAKE_INSTALL_FULL_BINDIR         : ${CMAKE_INSTALL_FULL_BINDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_DATADIR        : ${CMAKE_INSTALL_FULL_DATADIR}")
    message(STATUS "CMAKE_INSTALL_FULL_DATAROOTDIR    : ${CMAKE_INSTALL_FULL_DATAROOTDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_DOCDIR         : ${CMAKE_INSTALL_FULL_DOCDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_INCLUDEDIR     : ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_INFODIR        : ${CMAKE_INSTALL_FULL_INFODIR}")
    message(STATUS "CMAKE_INSTALL_FULL_LIBDIR         : ${CMAKE_INSTALL_FULL_LIBDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_LIBEXECDIR     : ${CMAKE_INSTALL_FULL_LIBEXECDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_LOCALSTATEDIR  : ${CMAKE_INSTALL_FULL_LOCALSTATEDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_MANDIR         : ${CMAKE_INSTALL_FULL_MANDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_OLDINCLUDEDIR  : ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_RUNSTATEDIR    : ${CMAKE_INSTALL_FULL_RUNSTATEDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_SBINDIR        : ${CMAKE_INSTALL_FULL_SBINDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_SHAREDSTATEDIR : ${CMAKE_INSTALL_FULL_SHAREDSTATEDIR}")
    message(STATUS "CMAKE_INSTALL_FULL_SYSCONFDIR     : ${CMAKE_INSTALL_FULL_SYSCONFDIR}")
    #
    message(STATUS "CMAKE_LIBRARY_PATH        : ${CMAKE_LIBRARY_PATH}")
    message(STATUS "CMAKE_LINKER              : ${CMAKE_LINKER}")
    message(STATUS "CMAKE_LINKER_LINK         : ${CMAKE_LINKER_LINK}")
    message(STATUS "CMAKE_LINKER_LLD          : ${CMAKE_LINKER_LLD}")
    message(STATUS "CMAKE_MODULE_PATH         : ${CMAKE_MODULE_PATH}")
    message(STATUS "CMAKE_PREFIX_PATH         : ${CMAKE_PREFIX_PATH}")
    message(STATUS "CMAKE_PROGRAM_PATH        : ${CMAKE_PROGRAM_PATH}")
    message(STATUS "CMAKE_SYSTEM_LIBRARY_PATH : ${CMAKE_SYSTEM_LIBRARY_PATH}")
    message(STATUS "CMAKE_SYSTEM_PREFIX_PATH  : ${CMAKE_SYSTEM_PREFIX_PATH}")
    #
    message(STATUS "CMAKE_VS_DEVENV_COMMAND                  : ${CMAKE_VS_DEVENV_COMMAND}")
    message(STATUS "CMAKE_VS_MSBUILD_COMMAND                 : ${CMAKE_VS_MSBUILD_COMMAND}")
    message(STATUS "CMAKE_VS_PLATFORM_NAME                   : ${CMAKE_VS_PLATFORM_NAME}")
    message(STATUS "CMAKE_VS_PLATFORM_TOOLSET                : ${CMAKE_VS_PLATFORM_TOOLSET}")
    message(STATUS "CMAKE_VS_VERSION_BUILD_NUMBER            : ${CMAKE_VS_VERSION_BUILD_NUMBER}")
    message(STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION : ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
    #
    message(STATUS "QT_ALL_MODULES_FOUND_VIA_FIND_PACKAGE           : ${QT_ALL_MODULES_FOUND_VIA_FIND_PACKAGE}")
    message(STATUS "QT_ALL_MODULES_VERSIONED_FOUND_VIA_FIND_PACKAGE : ${QT_ALL_MODULES_VERSIONED_FOUND_VIA_FIND_PACKAGE}")
    message(STATUS "QT_ALL_PLUGINS_FOUND_BY_FIND_PACKAGE            : ${QT_ALL_PLUGINS_FOUND_BY_FIND_PACKAGE}")
    message(STATUS "QT_ALL_PLUGINS_VERSIONED_FOUND_BY_FIND_PACKAGE  : ${QT_ALL_PLUGINS_VERSIONED_FOUND_BY_FIND_PACKAGE}")
    message(STATUS "QT_ALL_PLUGINS_TYPES_FOUND_VIA_FIND_PACKAGE     : ${QT_ALL_PLUGINS_TYPES_FOUND_VIA_FIND_PACKAGE}")
    message(STATUS "QT_DEPLOY_SUPPORT                               : ${QT_DEPLOY_SUPPORT}")
    #
    message(STATUS "VCPKG_CMAKE_FIND_ROOT_PATH                      : ${VCPKG_CMAKE_FIND_ROOT_PATH}")
    message(STATUS "Z_VCPKG_EXECUTABLE                              : ${Z_VCPKG_EXECUTABLE}")
    message(STATUS "Z_VCPKG_INSTALLED_DIR_INITIAL_VALUE             : ${Z_VCPKG_INSTALLED_DIR_INITIAL_VALUE}")
    message(STATUS "Z_NATIVE_VCPKG_MANIFEST_INSTALL_LOGFILE         : ${Z_NATIVE_VCPKG_MANIFEST_INSTALL_LOGFILE}")
    message(STATUS "Z_VCPKG_MANIFEST_INSTALL_LOGFILE                : ${Z_VCPKG_MANIFEST_INSTALL_LOGFILE}")
    message(STATUS "Z_VCPKG_TOOLCHAIN_DIR                           : ${Z_VCPKG_TOOLCHAIN_DIR}")
    #
    # message(FATAL_ERROR "FATAL_ERROR: check above messages for details")
endfunction()
