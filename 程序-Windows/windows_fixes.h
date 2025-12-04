#ifndef WINDOWS_FIXES_H
#define WINDOWS_FIXES_H

// Windows平台特定的修复和配置

#ifdef Q_OS_WIN
#include <windows.h>
#include <io.h>
#include <fcntl.h>

// 确保Windows控制台支持UTF-8（如果使用控制台）
inline void setupWindowsConsole()
{
#ifdef _DEBUG
    // 仅在Debug模式下设置控制台
    if (AllocConsole())
    {
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        
        // 设置控制台代码页为UTF-8
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
    }
#endif
}

// Windows路径处理辅助函数
inline QString normalizeWindowsPath(const QString &path)
{
    QString normalized = path;
    // Qt会自动处理路径分隔符，但确保路径格式正确
    normalized.replace('\\', '/');
    return normalized;
}

#else
// 非Windows平台的空实现
inline void setupWindowsConsole() {}
inline QString normalizeWindowsPath(const QString &path) { return path; }
#endif

#endif // WINDOWS_FIXES_H

