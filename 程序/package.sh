#!/bin/bash

# Qt应用打包脚本 for macOS
# 使用方法: ./package.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

APP_NAME="DijkstraApp"
BUILD_DIR="build-release"
APP_BUNDLE="${BUILD_DIR}/${APP_NAME}.app"

# 检测Qt6
QT6_QMAKE="$HOME/Qt/6.10.0/macos/bin/qmake"
QT6_MACDEPLOYQT="$HOME/Qt/6.10.0/macos/bin/macdeployqt"

if [ -f "$QT6_QMAKE" ]; then
    echo "✓ 检测到Qt 6.10.0，使用Qt6编译"
    QMAKE_CMD="$QT6_QMAKE"
    MACDEPLOYQT_CMD="$QT6_MACDEPLOYQT"
else
    echo "⚠ 未找到Qt6，使用系统默认qmake"
    QMAKE_CMD="qmake"
    MACDEPLOYQT_CMD="macdeployqt"
fi

echo "=========================================="
echo "开始打包 ${APP_NAME}"
echo "=========================================="

# 1. 清理旧的构建目录
echo ""
echo "步骤 1: 清理旧的构建目录..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 2. 编译Release版本
echo ""
echo "步骤 2: 编译Release版本..."
"$QMAKE_CMD" -config release
# 移除 Makefile 中的 AGL framework 引用（AGL 在现代 macOS 中不存在）
sed -i '' 's/-framework AGL//g' Makefile 2>/dev/null || sed -i '' 's/-framework AGL//g' Makefile
sed -i '' 's|AGL.framework/Headers/||g' Makefile 2>/dev/null || sed -i '' 's|AGL.framework/Headers/||g' Makefile
make clean || true
make -j$(sysctl -n hw.ncpu)

# 查找编译好的.app文件
if [ -d "build/Qt_6_10_0_for_macOS-Release/${APP_NAME}.app" ]; then
    APP_PATH="build/Qt_6_10_0_for_macOS-Release/${APP_NAME}.app"
elif [ -d "${APP_NAME}.app" ]; then
    APP_PATH="${APP_NAME}.app"
else
    echo "错误: 找不到编译好的.app文件"
    exit 1
fi

echo "找到应用: $APP_PATH"

# 3. 复制到打包目录
echo ""
echo "步骤 3: 复制应用到打包目录..."
cp -R "$APP_PATH" "$APP_BUNDLE"

# 4. 使用macdeployqt打包Qt依赖
echo ""
echo "步骤 4: 使用macdeployqt打包Qt依赖..."
if [ -z "$MACDEPLOYQT_CMD" ] || [ ! -f "$MACDEPLOYQT_CMD" ]; then
    # 尝试查找macdeployqt
    MACDEPLOYQT_CMD=$(which macdeployqt)
    if [ -z "$MACDEPLOYQT_CMD" ]; then
        MACDEPLOYQT_CMD=$(find ~/Qt -name "macdeployqt" 2>/dev/null | head -1)
    fi
fi

if [ -z "$MACDEPLOYQT_CMD" ] || [ ! -f "$MACDEPLOYQT_CMD" ]; then
    echo "警告: 找不到macdeployqt，跳过Qt依赖打包"
    echo "应用可能无法在其他机器上运行"
else
    echo "使用: $MACDEPLOYQT_CMD"
    "$MACDEPLOYQT_CMD" "$APP_BUNDLE" -always-overwrite -verbose=2
    
    if [ $? -eq 0 ]; then
        echo "✓ Qt依赖打包成功"
    else
        echo "警告: macdeployqt执行可能有问题，但继续打包"
    fi
fi

# 5. 检查应用是否可执行
echo ""
echo "步骤 5: 验证应用..."
if [ -f "${APP_BUNDLE}/Contents/MacOS/${APP_NAME}" ]; then
    echo "✓ 应用可执行文件存在"
    # 设置可执行权限
    chmod +x "${APP_BUNDLE}/Contents/MacOS/${APP_NAME}"
else
    echo "错误: 找不到应用可执行文件"
    exit 1
fi

# 6. 创建DMG文件（可选）
echo ""
read -p "是否创建DMG安装包? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "步骤 6: 创建DMG文件..."
    DMG_NAME="${APP_NAME}_macOS.dmg"
    DMG_PATH="${BUILD_DIR}/${DMG_NAME}"
    
    # 创建临时目录用于DMG内容
    DMG_TEMP="${BUILD_DIR}/dmg_temp"
    rm -rf "$DMG_TEMP"
    mkdir -p "$DMG_TEMP"
    
    # 复制应用和创建Applications链接
    cp -R "$APP_BUNDLE" "$DMG_TEMP/"
    ln -s /Applications "$DMG_TEMP/Applications"
    
    # 创建DMG
    hdiutil create -volname "$APP_NAME" -srcfolder "$DMG_TEMP" -ov -format UDZO "$DMG_PATH"
    
    if [ $? -eq 0 ]; then
        echo "✓ DMG文件创建成功: $DMG_PATH"
        rm -rf "$DMG_TEMP"
    else
        echo "警告: DMG创建失败，但应用已打包完成"
    fi
fi

echo ""
echo "=========================================="
echo "打包完成！"
echo "=========================================="
echo ""
echo "应用位置: $APP_BUNDLE"
echo ""
echo "你可以："
echo "1. 直接双击 $APP_BUNDLE 运行"
echo "2. 将 $APP_BUNDLE 拖到 Applications 文件夹安装"
if [ -f "${BUILD_DIR}/${APP_NAME}_macOS.dmg" ]; then
    echo "3. 使用DMG文件分发: ${BUILD_DIR}/${APP_NAME}_macOS.dmg"
fi
echo ""

