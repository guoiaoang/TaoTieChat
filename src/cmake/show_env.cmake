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
