#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include "windows_fixes.h"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
#ifdef Q_OS_WIN
    // Windows平台特定设置
    setupWindowsConsole();
    
    // 设置应用程序信息（用于Windows资源管理器显示）
    app.setApplicationName("DijkstraApp");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DijkstraApp");
    app.setOrganizationDomain("dijkstraapp.local");
#endif

    MainWindow window;
    window.show();

    return app.exec();
}

