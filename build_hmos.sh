#!/bin/bash

# QEMU HarmonyOS Build Script
# Cross-compiles QEMU for HarmonyOS aarch64 with HMV accelerator support.
# Produces both shared library (.so) and standalone executable (ELF).

set -e

NDK_PATH="/mnt/e/HMOS_SDK/linux/native"
LLVM_PATH="$NDK_PATH/llvm"
SYSROOT="$NDK_PATH/sysroot"

# ─── Toolchain Paths (Full Paths) ───
export CC="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-clang"
export CXX="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-clang++"
export AR="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-ar"
export NM="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-nm"
export RANLIB="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-ranlib"
export STRIP="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-strip"
export OBJCOPY="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-objcopy"
export AS="$LLVM_PATH/bin/aarch64-unknown-linux-ohos-as"
export LD="$LLVM_PATH/bin/ld.lld"

# ─── Dependency paths ───
DEPS_INSTALL_DIR="/mnt/e/projects/qemu-hmos/third_party/deps/install-ohos"

# ─── pkg-config ───
export PKG_CONFIG="/usr/bin/pkg-config"
export PKG_CONFIG_PATH="$DEPS_INSTALL_DIR/lib/pkgconfig"
export PKG_CONFIG_LIBDIR="$DEPS_INSTALL_DIR/lib/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR="/"

# Add toolchain to PATH (for any tools resolved by short name)
export PATH="$LLVM_PATH/bin:$PATH"

# ─── Compilation Flags ───
TARGET="aarch64-linux-ohos"
CFLAGS="--target=$TARGET --sysroot=$SYSROOT -O2 -g -I$DEPS_INSTALL_DIR/include -D__user="
LDFLAGS="--target=$TARGET --sysroot=$SYSROOT -L$DEPS_INSTALL_DIR/lib"

# ─── Step 1: Clean previous build ───
echo "=== Step 1: Cleaning previous build ==="
rm -rf build
echo "Build directory cleaned."

# ─── Step 2: Fix line endings (WSL/Windows interop) ───
echo "=== Step 2: Converting scripts to LF ==="
tr -d '\r' < configure > configure.tmp && mv configure.tmp configure
chmod +x configure
find scripts -type f -exec bash -c "tr -d '\r' < {} > {}.tmp && mv {}.tmp {}" \;
find . -maxdepth 1 -name "*.sh" -exec bash -c "tr -d '\r' < {} > {}.tmp && mv {}.tmp {}" \;
chmod +x scripts/*.py 2>/dev/null || true
echo "Done."

# ─── Step 3: Configure ───
echo "=== Step 3: Configuring QEMU for HarmonyOS ==="
./configure \
    --cross-prefix=aarch64-unknown-linux-ohos- \
    --cpu=aarch64 \
    --target-list=aarch64-softmmu \
    --enable-hmv \
    --disable-vhost-user \
    --disable-vhost-net \
    --disable-keyring \
    --disable-libkeyutils \
    --disable-guest-agent \
    --disable-vhost-user-blk-server \
    --disable-virtfs \
    --disable-werror \
    --extra-cflags="$CFLAGS" \
    --extra-ldflags="$LDFLAGS" \
    --cc="$CC" \
    --cxx="$CXX" \
    --host-cc=gcc

if [ $? -ne 0 ]; then
    echo "❌ Configuration failed."
    exit 1
fi

echo "✅ Configuration successful."

# ─── Step 4: Build ───
echo "=== Step 4: Building QEMU ==="
ninja -C build -j$(nproc)

if [ $? -ne 0 ]; then
    echo "❌ Build failed."
    exit 1
fi

# ─── Step 5: Summary ───
echo ""
echo "=========================================="
echo "  ✅ BUILD SUCCESSFUL"
echo "=========================================="
echo ""
echo "Build artifacts:"
echo "────────────────────────────────────────"

if [ -f build/libqemu-system-aarch64.so ]; then
    echo "  [SO]  $(ls -lh build/libqemu-system-aarch64.so | awk '{print $5}')  build/libqemu-system-aarch64.so"
    file build/libqemu-system-aarch64.so
fi

if [ -f build/qemu-system-aarch64-elf ]; then
    echo "  [ELF] $(ls -lh build/qemu-system-aarch64-elf | awk '{print $5}')  build/qemu-system-aarch64-elf"
    file build/qemu-system-aarch64-elf
fi

if [ -f build/libqemu-img.so ]; then
    echo "  [SO]  $(ls -lh build/libqemu-img.so | awk '{print $5}')  build/libqemu-img.so"
fi

echo "────────────────────────────────────────"
echo "HMV symbols check:"
strings build/libqemu-system-aarch64.so | grep -c 'hmv' | xargs -I{} echo "  {} HMV-related strings found"
echo ""
