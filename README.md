# 聊天客户端

## 功能介绍

已实现文本、文件、图片消息的实时及离线收发，可编辑个人资料及好友备注，并在好友端实时修改。

用户数据默认保存在用户文档的 `TaoTie\Chat\Client` 目录下。

服务端见 [TaoTieChatServer](https://github.com/guoiaoang/TaoTieChatServer)。

<div style="display: flex; gap: 10px; align-items: center;">
  <img src="docs\images\chat.png" alt="聊天界面" style="height: 300px; object-fit: cover;">
  <img src="docs\images\contact.png" alt="联系人界面" style="height: 300px; object-fit: cover;">
  <img src="docs\images\mediaview.png" alt="媒体查看器" style="height: 300px; object-fit: cover;">
</div>

## 项目主要依赖与编译

### Windows

#### 主要依赖

* MSVC 2022
* Qt 6.10.3
* CMake
    * CMake
    * CPack
    * CTest
* vcpkg
    * GRPC
        * Protobuf
    * sqlcipher
        * sqlite
* nerd-fonts ([ryanoasisnerd-fonts](https://github.com/ryanoasis/nerd-fonts)，提供图标支持)

#### Release 编译

Debug 编译同理，可在 VCCode 中打开当前项目。

##### 1. 安装 MSVC 编译依赖。

从网页端或执行命令下载安装 MSVS 2022 安装引导程序。

```shell
# 下载安装 MSVS 2022 安装引导程序
winget install Microsoft.VisualStudio.2022.BuildTools
```

勾选 `使用 C++ 的桌面开发`、`Windows SDK (10.0.26100.7705)`、`vcpkg 包管理器`、`用于 Windows 的 C++ CMake 工具`。

或导入以下配置：
```json
// .vsconfig 导出的配置
{
  "version": "1.0",
  "components": [
    "Microsoft.Component.MSBuild",
    "Microsoft.VisualStudio.Component.CoreBuildTools",
    "Microsoft.VisualStudio.Component.Roslyn.Compiler",
    "Microsoft.VisualStudio.Component.TestTools.BuildTools",
    "Microsoft.VisualStudio.Component.TextTemplating",
    "Microsoft.VisualStudio.Component.VC.ASAN",
    "Microsoft.VisualStudio.Component.VC.CMake.Project",
    "Microsoft.VisualStudio.Component.VC.CoreBuildTools",
    "Microsoft.VisualStudio.Component.VC.CoreIde",
    "Microsoft.VisualStudio.Component.VC.Redist.14.Latest",
    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
    "Microsoft.VisualStudio.Component.Vcpkg",
    "Microsoft.VisualStudio.Component.Windows10SDK",
    "Microsoft.VisualStudio.Component.Windows11SDK.26100",
    "Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core",
    "Microsoft.VisualStudio.Workload.MSBuildTools",
    "Microsoft.VisualStudio.Workload.VCTools"
  ],
  "extensions": []
}
```

##### 2. 安装 QT 6.10.3 的相关依赖。

可从阿里云镜像站下载安装程序，然后再勾选安装相关文件。

```shell
# 下载安装程序
wget https://mirrors.aliyun.com/qt/official_releases/online_installers/qt-online-installer-windows-x64-online.exe

# 执行安装命令
# 建议先无负载安装，再勾选负载安装，以降低安装失败的概率
qt-online-installer-windows-x64-online.exe --mirror https://mirrors.aliyun.com/qt

# 补充负载
# `C:\app\dev\qt` 为 QT 的安装根目录
C:\app\dev\qt\MaintenanceTool.exe --mirror https://mirrors.aliyun.com/qt
```

##### 3. 安装 vcpkg

```shell
git clone https://github.com/microsoft/vcpkg.git C:/app/dev/vcpkg
```

##### 4. 修改 `CMakeUserPresets.json` 中 `environment` 部分

```json
// CMakeUserPresets.json:
"environment": {
    // 需要查找的编译程序的完整路径，这里实际不使用
    "compilerPath": "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe",
    // 需要查找的编译 SDK 的完整路径，这里实际不使用
    "SDKRCPath"   : "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64/rc.exe",

    // ninja 编译程序所在路径
    "NINJA_ROOT"  : "C:/app/dev/ninja",
    // 指定 vcpkg 所在路径
    "VCPKG_ROOT"  : "C:/app/dev/vcpkg",
    // 指定 QT 的 CMake 路径
    "CMAKE_ROOT"  : "C:/app/dev/Qt/Tools/CMake_64",
    // 指定 QT 依赖文件所在路径
    "QT_ROOT"     : "C:/app/dev/Qt/6.10.3/msvc2022_64",
    // 指定 MSVC 编译程序相关路径
    "VCINSTALLDIR": "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC",
    // 编译 SDK 的路径
    "SDK_ROOT"    : "C:/Program Files (x86)/Windows Kits/10",
    // 用于拼接查找相关文件
    "VC_VERSION"  : "14.44.35207",
    "SDK_VERSION" : "10.0.26100.0",
    "BUILD_ARCH"  : "x64",
    "BUILD_TYPE"  : "Debug",
    // 可以设置代理地址用于 vcpkg 相关包的下载加速
    "HTTP_PROXY"  : "http://127.0.0.1:1080",
    "HTTPS_PROXY" : "http://127.0.0.1:1080"
}
```

##### 5. 执行编译

```shell
# 查看配置预设
cmake --list-presets
# 查看生成预设
cmake --build --list-presets
# 查看打包预设
cpack --list-presets
# # 查看测试预设
# ctest --list-presets

# 执行配置命令
cmake --preset user-windows-x64-release-configure

# 执行生成命令
# 编译输出文件在 `out\build\Release\TaoTieChat.exe`
cmake --build --preset user-windows-x64-release-build

# 执行打包命令。其他格式参考 `CMakeLists.txt` 中的 `CPACK_GENERATOR` 设置
# 打包文件在 `out/build/Release/setup/TaoTieChat.zip`
cpack --preset base-windows-release-package

# 清理
# echo 1 | git clean -fixd
```

#### VSCode 中打开

参考 `Release 编译` 部分部署好环境及安装 `ms-vscode.cpptools` 和 `ms-vscode.cpptools` 扩展后，即可在 CMake 窗口中选择相关预设进行编译调试。

`.vscode\launch.json` 中的 `CMake Debug (MSVC - Windows)` 可指定 Qt 的调试链接符号文件。

![VSCode 中的 CMake 预设](docs\images\vscode-cmake-user-debug-presets.png)
