# Windows版本构建说明

## Windows平台特定修改

本目录中的代码已经针对Windows平台进行了优化和修改：

### 1. 路径处理
- ✅ 使用 `QDir::separator()` 替代硬编码的 "/"
- ✅ 使用 `QStandardPaths::AppDataLocation` 获取Windows标准数据目录
- ✅ Windows数据目录：`C:\Users\用户名\AppData\Local\DijkstraApp\`

### 2. 文件编码
- ✅ 文件读取使用UTF-8编码（Qt6默认）
- ✅ 确保中文路径和文件名正确处理

### 3. 项目配置（DijkstraApp.pro）
- ✅ Windows平台特定配置
- ✅ UTF-8编码支持（MSVC和MinGW）
- ✅ GUI应用配置（无控制台窗口）

### 4. Windows特定功能
- ✅ 应用程序信息设置（用于资源管理器显示）
- ✅ 控制台支持（Debug模式下可选）

## 数据库文件位置

Windows版本的数据文件存储在：
```
C:\Users\<用户名>\AppData\Local\DijkstraApp\graph.db
```

这是Windows标准应用程序数据目录，符合Windows应用规范。

## 编译要求

### MSVC版本
- Visual Studio 2019 或 2022
- Qt 6.10.0 或更高版本（MSVC版本）

### MinGW版本
- MinGW-w64
- Qt 6.10.0 或更高版本（MinGW版本）

## 已知问题

1. **文件路径中的中文**：Qt6已正确处理UTF-8编码，应该没有问题
2. **长路径**：Windows 10+ 支持长路径，如果遇到问题可以启用长路径支持

## 测试建议

在Windows上测试时，请验证：
1. ✅ 数据库文件正确创建在AppData目录
2. ✅ 文件对话框能正确显示中文路径
3. ✅ 导入/导出功能正常工作
4. ✅ 所有UI元素正常显示

