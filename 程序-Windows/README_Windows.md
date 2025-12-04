# Windows 版本编译说明

本目录包含 Windows 版本的源代码和编译脚本。

## 目录结构

```
程序-Windows/
├── README_Windows.md              # 本文件，Windows版本说明
├── .gitignore                     # Git忽略文件配置
├── package_windows.bat            # Windows本地编译脚本
├── package_windows_from_mac.sh    # Mac上交叉编译Windows版本脚本
├── create_github_actions.sh       # 创建GitHub Actions配置脚本
├── DijkstraApp.pro                # Qt项目文件
├── main.cpp                        # 主程序入口
├── mainwindow.cpp/h/ui            # 主窗口相关文件
├── visualizationwindow.cpp/h      # 可视化窗口相关文件
├── datamanagementwindow.cpp/h     # 数据管理窗口相关文件
├── databasemanagementwindow.cpp/h # 数据库管理窗口相关文件
├── dijkstra.cpp/h                 # Dijkstra算法实现
├── dijkstra_loader.cpp/h          # 文件加载器
└── graphdatabase.cpp/h            # 数据库操作
```

## 快速开始

### 在 Windows 上编译

```cmd
cd 程序-Windows
package_windows.bat
```

### 在 Mac 上交叉编译 Windows 版本

```bash
cd 程序-Windows
./package_windows_from_mac.sh
```

### 使用 GitHub Actions 自动编译

```bash
cd 程序-Windows
./create_github_actions.sh
# 然后推送代码到GitHub并创建tag
```

## 详细说明

请参考主目录下的 `README_打包说明.md` 文件，其中包含：
- Windows 平台打包详细步骤
- Mac 上交叉编译 Windows 版本的多种方法
- 常见问题解答

## 注意事项

1. **Qt版本要求**：需要 Qt 6.10.0 或更高版本
2. **编译器**：支持 MSVC 2019/2022 或 MinGW-w64
3. **数据库**：使用 SQLite，无需额外配置

## 编译输出

编译完成后，Windows 可执行文件位于：
- `build-release/DijkstraApp.exe`（使用 package_windows.bat）
- `build-windows/release/DijkstraApp.exe`（使用交叉编译）

