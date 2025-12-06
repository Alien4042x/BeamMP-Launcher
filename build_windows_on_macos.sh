#!/bin/bash

set -e

echo "🔧 BeamMP Launcher - Cross-compile for Windows on macOS"
echo "========================================================="

# Try to find llvm-mingw in common locations
if [ -d "$HOME/llvm-mingw" ]; then
    MINGW_PATH="$HOME/llvm-mingw"
elif [ -d "/usr/local/llvm-mingw" ]; then
    MINGW_PATH="/usr/local/llvm-mingw"
elif [ -d "/opt/llvm-mingw" ]; then
    MINGW_PATH="/opt/llvm-mingw"
else
    echo "❌ llvm-mingw not found!"
    echo ""
    echo "Please install llvm-mingw to one of these locations:"
    echo "  - $HOME/llvm-mingw (recommended)"
    echo "  - /usr/local/llvm-mingw"
    echo "  - /opt/llvm-mingw"
    echo ""
    echo "Download from: https://github.com/mstorsjo/llvm-mingw/releases"
    exit 1
fi

export PATH="$MINGW_PATH/bin:$PATH"
echo "✅ Using llvm-mingw from: $MINGW_PATH"

if ! command -v cmake &> /dev/null; then
    echo "❌ CMake is not installed!"
    echo "Install with: brew install cmake"
    exit 1
fi

if [ ! -d "vcpkg" ]; then
    echo "📦 Downloading vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    cd ..
else
    echo "✅ vcpkg already exists"
fi

echo "📦 Installing Windows dependencies..."
./vcpkg/vcpkg install --triplet=x64-mingw-static

echo "🔨 Configuring Windows build..."
rm -rf build_windows
mkdir -p build_windows
cd build_windows

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER="$MINGW_PATH/bin/x86_64-w64-mingw32-clang" \
    -DCMAKE_CXX_COMPILER="$MINGW_PATH/bin/x86_64-w64-mingw32-clang++" \
    -DCMAKE_RC_COMPILER="$MINGW_PATH/bin/x86_64-w64-mingw32-windres" \
    -DCMAKE_FIND_ROOT_PATH="$MINGW_PATH/x86_64-w64-mingw32" \
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY

echo "🔧 Patching httplib for mingw..."
python3 << 'PYEOF'
httplib_path = "vcpkg_installed/x64-mingw-static/include/httplib.h"
try:
    with open(httplib_path, 'r') as f:
        content = f.read()
    content = content.replace(
        '      if (cancel_handle) { ::GetAddrInfoExCancel(&cancel_handle); }',
        '      // Disabled for mingw: if (cancel_handle) { /*::GetAddrInfoExCancel(&cancel_handle);*/ }'
    )
    with open(httplib_path, 'w') as f:
        f.write(content)
    print("✅ httplib.h patched")
except Exception as e:
    print(f"⚠️ Failed to patch httplib: {e}")
PYEOF

echo "🔨 Compiling for Windows..."
cmake --build . --config Release -j$(sysctl -n hw.ncpu)

cd ..
mkdir -p release
cp build_windows/BeamMP-Launcher.exe release/

# Copy all required DLL dependencies from llvm-mingw
echo "📦 Copying DLL dependencies..."
DLL_SRC="$MINGW_PATH/x86_64-w64-mingw32/bin"
cp "$DLL_SRC"/*.dll release/ 2>/dev/null || echo "⚠️ No DLLs found in $DLL_SRC"
echo "✅ DLL copy complete"

echo ""
echo "✅ Build completed successfully!"
echo "📁 Windows launcher: release/BeamMP-Launcher.exe"
echo "📊 Launcher size: $(du -h release/BeamMP-Launcher.exe | cut -f1)"
echo ""
echo "Run with Wine: wine release/BeamMP-Launcher.exe"
