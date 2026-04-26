$PROTO_FILE = "driver.proto"
$SCHEMA_DIR = ".\sdk\src\schema"
$CPP_OUT_DIR_ABS = (Resolve-Path ".\sdk\src\proto_gen")
$CSHARP_OUT_DIR_ABS = (Resolve-Path ".\samples\csharp_sample\src\proto_gen")
$JAVA_OUT_DIR_ABS = (Resolve-Path ".\samples\java_sample\src\main\java")

# Create output directories if they don't exist
New-Item -ItemType Directory -Force -Path $CPP_OUT_DIR_ABS | Out-Null
New-Item -ItemType Directory -Force -Path $CSHARP_OUT_DIR_ABS | Out-Null
New-Item -ItemType Directory -Force -Path $JAVA_OUT_DIR_ABS | Out-Null

# Check for protoc
if (!(Get-Command "protoc" -ErrorAction SilentlyContinue)) {
    Write-Host "protoc could not be found. Please install Protocol Buffers compiler." -ForegroundColor Red
    exit 1
}

Write-Host "Compiling protobuf files..." -ForegroundColor Cyan

# Change directory to schema to make proto paths relative
Push-Location $SCHEMA_DIR

# Compile for C++
protoc -I . --cpp_out=$CPP_OUT_DIR_ABS $PROTO_FILE
Write-Host "Generated C++ files in $CPP_OUT_DIR_ABS"

# Compile for C#
protoc -I . --csharp_out=$CSHARP_OUT_DIR_ABS $PROTO_FILE
Write-Host "Generated C# files in $CSHARP_OUT_DIR_ABS"

# Compile for Java
protoc -I . --java_out=$JAVA_OUT_DIR_ABS $PROTO_FILE
Write-Host "Generated Java files in $JAVA_OUT_DIR_ABS"

# Pop back to original directory
Pop-Location

Write-Host "Protobuf compilation completed successfully." -ForegroundColor Green


