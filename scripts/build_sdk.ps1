# Simple script to build the SDK

$BUILD_DIR = "..\sdk\build"

New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null
Set-Location -Path $BUILD_DIR

cmake ..
cmake --build .
