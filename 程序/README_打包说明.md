# DijkstraApp 打包说明

本文档提供在 macOS 和 Windows 上打包 `DijkstraApp` 应用程序的步骤，包括在 Mac 上交叉编译 Windows 版本的方法。

## 目录结构

```
生物医学大数据作业2/
├── 程序/                    # macOS版本源代码和脚本
│   ├── package.sh          # macOS打包脚本
│   ├── *.cpp, *.h          # 源代码文件
│   └── README_打包说明.md   # 本文件
│
└── 程序-Windows/           # Windows版本源代码和脚本
    ├── package_windows.bat              # Windows本地编译脚本
    ├── package_windows_from_mac.sh     # Mac上交叉编译脚本
    ├── create_github_actions.sh         # GitHub Actions配置脚本
    ├── *.cpp, *.h                      # 源代码文件（与程序/目录相同）
    └── README_Windows.md                # Windows版本说明
```

**注意**：`程序/` 和 `程序-Windows/` 目录中的源代码文件是相同的，只是编译脚本不同。

---

## macOS 平台打包

## 快速打包

直接运行打包脚本：

```bash
cd 程序
./package.sh
```

## 手动打包步骤

如果自动脚本有问题，可以手动执行以下步骤：

### 1. 编译Release版本

```bash
cd 程序
qmake -config release
make clean
make -j8
```

### 2. 找到编译好的应用

编译完成后，应用通常在以下位置之一：
- `build/Qt_6_10_0_for_macOS-Release/DijkstraApp.app`
- `DijkstraApp.app`

### 3. 使用macdeployqt打包Qt依赖

```bash
# 找到macdeployqt工具
which macdeployqt
# 或者
find ~/Qt -name "macdeployqt"

# 打包应用（替换路径为实际的应用路径）
macdeployqt DijkstraApp.app -always-overwrite
```

### 4. 验证应用

```bash
# 检查应用结构
ls -la DijkstraApp.app/Contents/MacOS/
# 应该能看到 DijkstraApp 可执行文件

# 测试运行
open DijkstraApp.app
```

### 5. 创建DMG安装包（可选）

```bash
# 创建临时目录
mkdir -p dmg_temp
cp -R DijkstraApp.app dmg_temp/
ln -s /Applications dmg_temp/Applications

# 创建DMG
hdiutil create -volname "DijkstraApp" -srcfolder dmg_temp -ov -format UDZO DijkstraApp_macOS.dmg

# 清理
rm -rf dmg_temp
```

## 分发应用

打包完成后，你可以：

1. **直接分发.app文件**
   - 将 `DijkstraApp.app` 压缩成zip文件
   - 用户解压后双击即可运行

2. **分发DMG文件**
   - DMG文件更专业，用户双击后拖拽到Applications即可安装
   - 适合正式发布

## 注意事项

1. **数据库文件位置**
   - 应用会在 `~/Library/Application Support/DijkstraApp/` 目录下创建数据库文件
   - 打包时不需要包含数据库文件，应用首次运行会自动创建

2. **Qt版本兼容性**
   - 打包的应用包含Qt库，可以在没有安装Qt的Mac上运行
   - 但需要确保目标Mac的系统版本兼容（通常macOS 10.13+）

3. **代码签名（可选）**
   - 如果要发布到App Store或让用户信任，需要代码签名
   - 需要Apple开发者账号
   - 命令：`codesign --deep --force --verify --verbose --sign "Developer ID Application: Your Name" DijkstraApp.app`

4. **公证（可选）**
   - 对于正式发布，建议进行公证
   - 需要Apple开发者账号
   - 命令：`xcrun notarytool submit DijkstraApp.app --keychain-profile "your-profile" --wait`

## 常见问题

**Q: 应用在其他Mac上无法运行？**
A: 确保使用了macdeployqt打包了所有Qt依赖，并且目标Mac系统版本兼容。

**Q: 找不到macdeployqt？**
A: macdeployqt通常在Qt安装目录的bin文件夹下，或者通过conda安装的Qt也会有。

**Q: 应用很大？**
A: 这是正常的，因为包含了Qt库。可以使用`strip`命令减小体积（但可能影响调试）。

---

## Windows 平台打包

### 前提条件

1. **Qt 安装**：确保你的 Windows 系统上安装了 Qt 6.x.x 版本
   - 推荐使用 Qt 6.10.0 或更高版本
   - 可以选择 MSVC 2019/2022 或 MinGW 版本
   - 确保 Qt 的 `bin` 目录在系统 PATH 环境变量中，或者手动设置脚本中的路径

2. **编译器**：
   - **MSVC版本**：需要安装 Visual Studio 2019 或 2022（包含 C++ 工具）
   - **MinGW版本**：需要安装 MinGW-w64

### 快速打包

直接运行打包脚本：

```cmd
cd 程序
package_windows.bat
```

脚本会自动：
- 检测 Qt6 安装位置（常见路径：`C:\Qt\6.10.0\msvc2019_64` 或 `C:\Qt\6.10.0\mingw_64`）
- 编译 Release 版本
- 使用 `windeployqt` 打包 Qt 依赖
- 生成可分发的应用包

### 手动打包步骤

如果自动脚本有问题，可以手动执行以下步骤：

#### 1. 打开开发者命令提示符（MSVC版本）

- 打开 "开始菜单" → "Visual Studio 2019/2022" → "Developer Command Prompt for VS"
- 或者使用 "x64 Native Tools Command Prompt"

#### 2. 编译Release版本

```cmd
cd 程序目录路径
qmake -config release
nmake clean
nmake release
```

**MinGW版本**：
```cmd
cd 程序目录路径
qmake -config release
mingw32-make clean
mingw32-make release
```

#### 3. 找到编译好的应用

编译完成后，应用通常在以下位置：
- `release\DijkstraApp.exe`
- `DijkstraApp.exe`

#### 4. 使用windeployqt打包Qt依赖

```cmd
# 找到windeployqt工具（通常在Qt安装目录的bin文件夹下）
# 例如：C:\Qt\6.10.0\msvc2019_64\bin\windeployqt.exe

# 打包应用（替换路径为实际的应用路径）
windeployqt release\DijkstraApp.exe --release --force --compiler-runtime
```

**参数说明**：
- `--release`：打包Release版本的依赖
- `--force`：强制覆盖已存在的文件
- `--compiler-runtime`：包含编译器运行时库（MSVC版本需要）

#### 5. 验证应用

```cmd
# 检查文件结构
dir release

# 应该能看到：
# - DijkstraApp.exe（主程序）
# - Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll 等（Qt库）
# - platforms\qwindows.dll（平台插件）
# - sqldrivers\qsqlite.dll（SQLite驱动）
```

### Windows 打包结果

打包完成后，`release` 目录（或你指定的构建目录）包含：
- `DijkstraApp.exe`：主程序
- Qt DLL 文件：所有必要的 Qt 库
- `platforms\`：平台插件目录
- `sqldrivers\`：数据库驱动目录
- 其他必要的依赖文件

### 分发应用

打包完成后，你可以：

1. **直接分发文件夹**
   - 将整个 `release` 目录压缩成zip文件
   - 用户解压后运行 `DijkstraApp.exe` 即可

2. **创建安装包（推荐）**
   - 使用 **NSIS**（Nullsoft Scriptable Install System）
   - 使用 **Inno Setup**
   - 使用 **Qt Installer Framework**
   - 这些工具可以创建专业的安装程序

### Windows 注意事项

1. **数据库文件位置**
   - 应用会在用户目录下创建数据库文件
   - Windows: `C:\Users\用户名\AppData\Local\DijkstraApp\` 或类似位置
   - 打包时不需要包含数据库文件，应用首次运行会自动创建

2. **Qt版本兼容性**
   - 打包的应用包含Qt库，可以在没有安装Qt的Windows上运行
   - 但需要确保目标Windows系统版本兼容（通常Windows 10+）

3. **编译器运行时（MSVC版本）**
   - 如果使用MSVC编译，需要包含Visual C++ Redistributable
   - `windeployqt --compiler-runtime` 会自动处理
   - 或者让用户安装 Visual C++ Redistributable

4. **代码签名（可选）**
   - 如果要让用户信任，建议进行代码签名
   - 需要代码签名证书（通常需要购买）
   - 使用 `signtool` 工具进行签名

### Windows 常见问题

**Q: 应用在其他Windows上无法运行？**
A: 
- 确保使用了 `windeployqt` 打包了所有Qt依赖
- MSVC版本：确保目标机器安装了 Visual C++ Redistributable
- 检查是否缺少必要的DLL文件（可以使用 Dependency Walker 工具检查）

**Q: 找不到windeployqt？**
A: windeployqt通常在Qt安装目录的bin文件夹下，例如：
- `C:\Qt\6.10.0\msvc2019_64\bin\windeployqt.exe`
- `C:\Qt\6.10.0\mingw_64\bin\windeployqt.exe`

**Q: 应用很大？**
A: 这是正常的，因为包含了Qt库。Release版本通常比Debug版本小很多。

**Q: 杀毒软件报毒？**
A: 未签名的应用可能被误报。建议进行代码签名，或者让用户添加信任。

**Q: 如何创建安装包？**
A: 推荐使用 NSIS 或 Inno Setup：
- **NSIS**: 免费开源，功能强大
- **Inno Setup**: 免费，简单易用
- **Qt Installer Framework**: Qt官方工具，适合Qt应用

---

## 在 Mac 上交叉编译 Windows 版本

如果你在 Mac 上开发，但需要打包 Windows 版本，有以下几种方法：

### 方法1: 使用 MXE (推荐，本地编译)

MXE (M cross environment) 是一个专门用于交叉编译的工具，可以在 Mac 上编译 Windows 应用。

#### 安装 MXE

```bash
# 克隆MXE仓库
git clone https://github.com/mxe/mxe.git ~/mxe
cd ~/mxe

# 编译Qt6和相关工具（这需要较长时间，可能需要1-2小时）
make qt6base qt6tools qt6svg qt6sql

# 或者编译所有Qt6模块（更慢但更完整）
make qt6
```

#### 使用脚本自动编译

```bash
cd 程序
./package_windows_from_mac.sh
```

脚本会自动检测 MXE 并编译 Windows 版本。

#### 手动编译

```bash
cd 程序
mkdir -p build-windows
cd build-windows

# 使用MXE的qmake配置项目
~/mxe/usr/bin/x86_64-w64-mingw32.shared-qmake-qt6 ../DijkstraApp.pro

# 编译
make -j8
```

编译完成后，Windows 可执行文件在 `build-windows/release/DijkstraApp.exe`。

**注意**：编译好的 Windows 版本需要手动打包 Qt 依赖。建议：
1. 将整个 `build-windows` 目录复制到 Windows 机器
2. 在 Windows 上运行 `windeployqt` 打包依赖

### 方法2: 使用 Docker

如果你安装了 Docker Desktop，可以使用 Docker 容器编译：

```bash
# 拉取MXE Docker镜像
docker pull mxe/mxe

# 运行编译脚本（脚本会自动使用Docker）
cd 程序
./package_windows_from_mac.sh
```

或者手动使用 Docker：

```bash
docker run --rm -it \
    -v "$(pwd):/source" \
    -w /source \
    mxe/mxe \
    bash -c "
        export PATH=/usr/lib/mxe/usr/bin:\$PATH
        x86_64-w64-mingw32.shared-qmake-qt6 DijkstraApp.pro
        make -j\$(nproc)
    "
```

### 方法3: 使用 GitHub Actions (推荐，云端编译)

这是最简单的方法，不需要本地配置交叉编译环境。

#### 设置 GitHub Actions

```bash
cd 程序
./create_github_actions.sh
```

这会创建 `.github/workflows/build-windows.yml` 配置文件。

#### 使用步骤

1. **推送代码到 GitHub**：
   ```bash
   git add .
   git commit -m "Add GitHub Actions for Windows build"
   git push origin main
   ```

2. **触发构建**：
   - **方式1**：创建tag自动触发
     ```bash
     git tag v1.0.0
     git push origin v1.0.0
     ```
   - **方式2**：手动触发
     - 在 GitHub 仓库页面，点击 "Actions" 标签
     - 选择 "Build Windows Release" workflow
     - 点击 "Run workflow"

3. **下载编译结果**：
   - 编译完成后，在 Actions 页面下载 `DijkstraApp-Windows.zip`
   - 这个zip文件已经包含了所有Qt依赖，可以直接分发

#### GitHub Actions 的优势

- ✅ **无需本地配置**：不需要安装交叉编译工具链
- ✅ **自动化**：每次推送tag自动编译
- ✅ **完整打包**：自动使用 `windeployqt` 打包所有依赖
- ✅ **免费**：GitHub Actions 对公开仓库免费

### 方法4: 使用 Homebrew 安装 MinGW-w64

```bash
# 安装MinGW-w64
brew install mingw-w64

# 然后需要手动配置Qt和编译环境
# 这种方法比较复杂，不推荐
```

### 各方法对比

| 方法 | 难度 | 速度 | 完整打包 | 推荐度 |
|------|------|------|----------|--------|
| **GitHub Actions** | ⭐ 简单 | ⭐⭐⭐ 快 | ✅ 是 | ⭐⭐⭐⭐⭐ |
| **MXE** | ⭐⭐ 中等 | ⭐⭐ 慢（首次） | ⚠️ 需手动 | ⭐⭐⭐⭐ |
| **Docker** | ⭐⭐ 中等 | ⭐⭐ 中等 | ⚠️ 需手动 | ⭐⭐⭐ |
| **Homebrew MinGW** | ⭐⭐⭐ 复杂 | ⭐⭐ 慢 | ⚠️ 需手动 | ⭐⭐ |

### 推荐方案

**最佳方案**：使用 **GitHub Actions**
- 最简单，无需本地配置
- 自动完整打包
- 适合持续集成

**备选方案**：使用 **MXE**
- 适合需要频繁编译的场景
- 本地编译，不依赖网络
- 但需要手动打包Qt依赖

### 注意事项

1. **Qt依赖打包**：
   - MXE 和 Docker 方法编译出的 `.exe` 文件不包含Qt依赖
   - 需要将文件复制到 Windows 机器上运行 `windeployqt` 打包
   - GitHub Actions 方法会自动完成打包

2. **测试**：
   - 交叉编译的应用建议在 Windows 上测试
   - 确保所有功能正常工作

3. **性能**：
   - MXE 首次编译需要很长时间（1-2小时）
   - 后续编译会快很多（只编译变更的文件）

