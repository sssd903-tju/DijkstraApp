@echo off
REM Qt应用打包脚本 for Windows
REM 使用方法: package_windows.bat

setlocal enabledelayedexpansion

set APP_NAME=DijkstraApp
set BUILD_DIR=build-release
set APP_EXE=%BUILD_DIR%\%APP_NAME%.exe

REM 检测Qt6安装路径（常见位置）
set QT6_QMAKE=
set QT6_WINDEPLOYQT=

REM 尝试查找Qt6
if exist "C:\Qt\6.10.0\msvc2019_64\bin\qmake.exe" (
    set QT6_QMAKE=C:\Qt\6.10.0\msvc2019_64\bin\qmake.exe
    set QT6_WINDEPLOYQT=C:\Qt\6.10.0\msvc2019_64\bin\windeployqt.exe
    set QT6_DIR=C:\Qt\6.10.0\msvc2019_64
)
if exist "C:\Qt\6.10.0\mingw_64\bin\qmake.exe" (
    set QT6_QMAKE=C:\Qt\6.10.0\mingw_64\bin\qmake.exe
    set QT6_WINDEPLOYQT=C:\Qt\6.10.0\mingw_64\bin\windeployqt.exe
    set QT6_DIR=C:\Qt\6.10.0\mingw_64
)
if exist "%ProgramFiles%\Qt\6.10.0\msvc2019_64\bin\qmake.exe" (
    set QT6_QMAKE=%ProgramFiles%\Qt\6.10.0\msvc2019_64\bin\qmake.exe
    set QT6_WINDEPLOYQT=%ProgramFiles%\Qt\6.10.0\msvc2019_64\bin\windeployqt.exe
    set QT6_DIR=%ProgramFiles%\Qt\6.10.0\msvc2019_64
)

REM 如果没找到，尝试使用PATH中的qmake
if "%QT6_QMAKE%"=="" (
    where qmake >nul 2>&1
    if !errorlevel! equ 0 (
        for /f "delims=" %%i in ('where qmake') do set QT6_QMAKE=%%i
        for /f "delims=" %%i in ('where windeployqt') do set QT6_WINDEPLOYQT=%%i
    )
)

if "%QT6_QMAKE%"=="" (
    echo 错误: 未找到Qt6安装，请手动设置QT6_QMAKE和QT6_WINDEPLOYQT变量
    echo 或者将Qt6的bin目录添加到系统PATH环境变量中
    exit /b 1
)

echo ==========================================
echo 开始打包 %APP_NAME% (Windows)
echo ==========================================
echo 使用Qt: %QT6_QMAKE%
echo.

REM 1. 清理旧的构建目录
echo 步骤 1: 清理旧的构建目录...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

REM 2. 编译Release版本
echo.
echo 步骤 2: 编译Release版本...
"%QT6_QMAKE%" -config release
if errorlevel 1 (
    echo qmake失败
    exit /b 1
)

REM 清理并编译
echo 正在编译...
nmake clean >nul 2>&1
if errorlevel 1 (
    mingw32-make clean >nul 2>&1
)

nmake release
if errorlevel 1 (
    echo 尝试使用mingw32-make...
    mingw32-make release
    if errorlevel 1 (
        echo 编译失败，请检查错误信息
        exit /b 1
    )
)

REM 查找编译好的exe文件（可能在release目录或当前目录）
set EXE_FOUND=0
if exist "release\%APP_NAME%.exe" (
    set "EXE_PATH=release\%APP_NAME%.exe"
    set EXE_FOUND=1
)
if exist "%APP_NAME%.exe" (
    set "EXE_PATH=%APP_NAME%.exe"
    set EXE_FOUND=1
)

if !EXE_FOUND! equ 0 (
    echo 错误: 找不到编译好的%APP_NAME%.exe
    echo 请检查编译输出中的错误信息
    exit /b 1
)

echo 找到应用: !EXE_PATH!

REM 3. 复制到打包目录
echo.
echo 步骤 3: 复制应用到打包目录...
copy "!EXE_PATH!" "%APP_EXE%"
if errorlevel 1 (
    echo 复制失败
    exit /b 1
)

REM 4. 使用windeployqt打包Qt依赖
echo.
echo 步骤 4: 使用windeployqt打包Qt依赖...
if "%QT6_WINDEPLOYQT%"=="" (
    echo 警告: 找不到windeployqt，跳过Qt依赖打包
    echo 应用可能无法在其他机器上运行
) else (
    echo 使用: %QT6_WINDEPLOYQT%
    "%QT6_WINDEPLOYQT%" "%APP_EXE%" --release --force --compiler-runtime
    if errorlevel 1 (
        echo windeployqt执行失败，但应用可能仍然可用
    )
)

REM 5. 复制必要的文件
echo.
echo 步骤 5: 复制必要的文件...
if exist "*.dll" copy "*.dll" "%BUILD_DIR%\" >nul 2>&1

echo.
echo ==========================================
echo 打包完成！
echo ==========================================
echo 应用位置: %BUILD_DIR%\%APP_NAME%.exe
echo.
echo 你可以将整个 %BUILD_DIR% 目录分发给其他用户
echo 或者创建一个安装包（使用NSIS、Inno Setup等工具）
echo.

pause

