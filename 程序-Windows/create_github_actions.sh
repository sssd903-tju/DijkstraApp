#!/bin/bash

# 创建GitHub Actions配置文件，用于自动编译Windows版本
# 使用方法: ./create_github_actions.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GITHUB_DIR="${SCRIPT_DIR}/../.github/workflows"

mkdir -p "$GITHUB_DIR"

cat > "${GITHUB_DIR}/build-windows.yml" << 'EOF'
name: Build Windows Release

on:
  push:
    tags:
      - 'v*'
  workflow_dispatch:

jobs:
  build-windows:
    runs-on: windows-latest
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v4
      
    - name: Setup Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: '6.10.0'
        modules: 'qtbase qttools qtsvg qtsql'
        arch: 'x64'
        
    - name: Configure project
      working-directory: 程序
      run: |
        qmake -config release
        
    - name: Build application
      working-directory: 程序
      run: |
        nmake release
        
    - name: Deploy Qt dependencies
      working-directory: 程序
      run: |
        windeployqt release\DijkstraApp.exe --release --force --compiler-runtime
        
    - name: Create release archive
      working-directory: 程序
      run: |
        powershell Compress-Archive -Path release\* -DestinationPath DijkstraApp-Windows.zip -Force
        
    - name: Upload artifact
      uses: actions/upload-artifact@v4
      with:
        name: DijkstraApp-Windows
        path: 程序/DijkstraApp-Windows.zip
        
    - name: Create Release
      if: startsWith(github.ref, 'refs/tags/')
      uses: softprops/action-gh-release@v1
      with:
        files: 程序/DijkstraApp-Windows.zip
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
EOF

chmod +x "${GITHUB_DIR}/build-windows.yml"

echo "✓ GitHub Actions配置文件已创建: .github/workflows/build-windows.yml"
echo ""
echo "使用方法："
echo "1. 将代码推送到GitHub仓库"
echo "2. 创建tag触发构建: git tag v1.0.0 && git push origin v1.0.0"
echo "3. 或者手动触发: 在GitHub仓库的Actions页面点击 'Run workflow'"
echo "4. 编译完成后从Actions页面下载Windows版本"
echo ""

