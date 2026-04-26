# Simple script to build the SDK
$BUILD_DIR = "..\sdk\build"
$VCPKG_PATH = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
$TRIPLET = "x64-windows-static"

# 1. Create and enter build directory
if (!(Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null
}
Set-Location -Path $BUILD_DIR

# 2. Configure Step (This is where the "Secret Sauce" goes)
cmake .. -DCMAKE_TOOLCHAIN_FILE="$VCPKG_PATH" -DVCPKG_TARGET_TRIPLET="$TRIPLET"

# 3. Build Step
cmake --build . --config Release
