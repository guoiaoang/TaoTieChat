# cmake 命令

```c++
// configure
cmake -S %cd% ^
    -B %cd%\out\buildx ^
    -G "Visual Studio 17 2022" ^
    --install-prefix=%cd%\out\installx ^
    -DCMAKE_BUILD_TYPE=Release ^
    "-DCMAKE_TOOLCHAIN_FILE=C:\a\t\vcpkg\scripts\buildsystems\vcpkg.cmake"
// build
cmake --build %cd%\out\buildx
// install 1.
cmake --install %cd%\out\buildx
// install 2.
cmake --build %cd%\out\buildx --target install
// ctest
ctest %cd%\out\buildx
// workflow
cmake --workflow --preset msvs2022x64_vcpkg_qt683_debug

// preset  // list presets
cmake --list-presets
cmake --build --list-presets
// configure
cmake --preset msvs2022x64_vcpkg_qt683_release
cmake --build --preset msvs2022x64_vcpkg_qt683_release
```
