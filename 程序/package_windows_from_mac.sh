#!/bin/bash

# 在Mac上交叉编译Windows版本的Qt应用
# 使用方法: ./package_windows_from_mac.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

APP_NAME="DijkstraApp"
BUILD_DIR="build-windows"
WINDOWS_EXE="${BUILD_DIR}/${APP_NAME}.exe"

echo "=========================================="
echo "在Mac上交叉编译Windows版本: ${APP_NAME}"
echo "=========================================="
echo ""

# 检测可用的交叉编译方法
METHOD=""

# 方法1: 检查MXE (M cross environment)
if [ -d "/opt/mxe" ] || [ -d "$HOME/mxe" ]; then
    MXE_DIR=""
    if [ -d "/opt/mxe" ]; then
        MXE_DIR="/opt/mxe"
    elif [ -d "$HOME/mxe" ]; then
        MXE_DIR="$HOME/mxe"
    fi
    
    if [ -n "$MXE_DIR" ] && [ -f "${MXE_DIR}/usr/bin/i686-w64-mingw32.shared-qmake-qt6" ] || [ -f "${MXE_DIR}/usr/bin/x86_64-w64-mingw32.shared-qmake-qt6" ]; then
        METHOD="mxe"
        echo "✓ 检测到MXE交叉编译环境"
    fi
fi

# 方法2: 检查Docker
if command -v docker &> /dev/null; then
    METHOD="docker"
    echo "✓ 检测到Docker，可以使用Docker容器编译"
fi

# 方法3: 检查GitHub Actions配置
if [ -d ".github/workflows" ]; then
    METHOD="github"
    echo "✓ 检测到GitHub Actions配置"
fi

# 如果没有检测到任何方法，提供安装指南
if [ -z "$METHOD" ]; then
    echo "⚠ 未检测到交叉编译环境"
    echo ""
    echo "请选择以下方法之一："
    echo ""
    echo "方法1: 使用MXE (推荐，本地编译)"
    echo "  1. 安装MXE:"
    echo "     git clone https://github.com/mxe/mxe.git ~/mxe"
    echo "     cd ~/mxe"
    echo "     make qt6base qt6tools qt6svg qt6sql"
    echo "  2. 然后重新运行此脚本"
    echo ""
    echo "方法2: 使用Docker (需要Docker Desktop)"
    echo "  1. 安装Docker Desktop for Mac"
    echo "  2. 运行: docker pull mxe/mxe"
    echo "  3. 然后重新运行此脚本"
    echo ""
    echo "方法3: 使用GitHub Actions (推荐，云端编译)"
    echo "  1. 推送代码到GitHub仓库"
    echo "  2. GitHub Actions会自动编译Windows版本"
    echo "  3. 从Actions页面下载编译好的应用"
    echo ""
    exit 1
fi

# 根据检测到的方法执行编译
case "$METHOD" in
    mxe)
        echo ""
        echo "使用MXE交叉编译..."
        
        # 检测MXE路径
        if [ -d "/opt/mxe" ]; then
            MXE_DIR="/opt/mxe"
        else
            MXE_DIR="$HOME/mxe"
        fi
        
        # 检测架构 (优先使用64位)
        if [ -f "${MXE_DIR}/usr/bin/x86_64-w64-mingw32.shared-qmake-qt6" ]; then
            MXE_QMAKE="${MXE_DIR}/usr/bin/x86_64-w64-mingw32.shared-qmake-qt6"
            ARCH="x86_64"
        elif [ -f "${MXE_DIR}/usr/bin/i686-w64-mingw32.shared-qmake-qt6" ]; then
            MXE_QMAKE="${MXE_DIR}/usr/bin/i686-w64-mingw32.shared-qmake-qt6"
            ARCH="i686"
        else
            echo "错误: 找不到MXE的Qt6 qmake"
            exit 1
        fi
        
        echo "使用架构: $ARCH"
        echo "MXE路径: $MXE_DIR"
        echo ""
        
        # 清理旧的构建
        echo "步骤 1: 清理旧的构建目录..."
        rm -rf "$BUILD_DIR"
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        
        # 配置Qt项目
        echo ""
        echo "步骤 2: 配置Qt项目..."
        "$MXE_QMAKE" ../DijkstraApp.pro
        
        # 编译
        echo ""
        echo "步骤 3: 编译Windows版本..."
        make -j$(sysctl -n hw.ncpu)
        
        # 查找编译好的exe
        if [ -f "release/${APP_NAME}.exe" ]; then
            EXE_PATH="release/${APP_NAME}.exe"
        elif [ -f "${APP_NAME}.exe" ]; then
            EXE_PATH="${APP_NAME}.exe"
        else
            echo "错误: 找不到编译好的${APP_NAME}.exe"
            exit 1
        fi
        
        echo ""
        echo "✓ 编译成功: $EXE_PATH"
        echo ""
        echo "注意: Windows版本需要手动打包Qt依赖"
        echo "建议在Windows机器上使用windeployqt打包，或者"
        echo "将整个build-windows目录复制到Windows上运行windeployqt"
        ;;
        
    docker)
        echo ""
        echo "使用Docker交叉编译..."
        echo ""
        echo "步骤 1: 准备Docker镜像..."
        
        # 检查是否有mxe镜像
        if ! docker images | grep -q "mxe/mxe"; then
            echo "拉取MXE Docker镜像（这可能需要一些时间）..."
            docker pull mxe/mxe
        fi
        
        echo ""
        echo "步骤 2: 在Docker容器中编译..."
        
        # 清理旧的构建
        rm -rf "$BUILD_DIR"
        mkdir -p "$BUILD_DIR"
        
        # 在Docker容器中编译
        docker run --rm -it \
            -v "$SCRIPT_DIR:/source" \
            -v "$SCRIPT_DIR/$BUILD_DIR:/build" \
            -w /source \
            mxe/mxe \
            bash -c "
                export PATH=/usr/lib/mxe/usr/bin:\$PATH
                cd /source
                mkdir -p /build
                cd /build
                x86_64-w64-mingw32.shared-qmake-qt6 ../DijkstraApp.pro
                make -j\$(nproc)
            "
        
        echo ""
        echo "✓ Docker编译完成"
        echo "检查构建目录: $BUILD_DIR"
        ;;
        
    github)
        echo ""
        echo "使用GitHub Actions编译..."
        echo ""
        echo "GitHub Actions配置已存在，请："
        echo "1. 将代码推送到GitHub仓库"
        echo "2. 在GitHub仓库的Actions页面查看编译进度"
        echo "3. 编译完成后从Actions页面下载Windows版本"
        echo ""
        echo "如果需要创建GitHub Actions配置，运行："
        echo "  ./create_github_actions.sh"
        ;;
esac

echo ""
echo "=========================================="
echo "交叉编译完成！"
echo "=========================================="
echo "构建目录: $BUILD_DIR"
echo ""

