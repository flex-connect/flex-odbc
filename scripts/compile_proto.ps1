$PROTO_FILE = "..\schema\driver.proto"
$CPP_OUT_DIR = "..\sdk\src\proto_gen"
$CSHARP_OUT_DIR = "..\samples\csharp_sample\src\proto_gen"
$JAVA_OUT_DIR = "..\samples\java_sample\src\main\java"

# Create output directories if they don't exist
New-Item -ItemType Directory -Force -Path $CPP_OUT_DIR | Out-Null
New-Item -ItemType Directory -Force -Path $CSHARP_OUT_DIR | Out-Null
New-Item -ItemType Directory -Force -Path $JAVA_OUT_DIR | Out-Null

# Check for protoc
if (!(Get-Command "protoc" -ErrorAction SilentlyContinue)) {
    Write-Host "protoc could not be found. Please install Protocol Buffers compiler." -ForegroundColor Red
    exit 1
}

Write-Host "Compiling protobuf files..." -ForegroundColor Cyan

# Compile for C++
protoc -I=..\schema --cpp_out=$CPP_OUT_DIR $PROTO_FILE
Write-Host "Generated C++ files in $CPP_OUT_DIR"

# Compile for C#
protoc -I=..\schema --csharp_out=$CSHARP_OUT_DIR $PROTO_FILE
Write-Host "Generated C# files in $CSHARP_OUT_DIR"

# Compile for Java
protoc -I=..\schema --java_out=$JAVA_OUT_DIR $PROTO_FILE
Write-Host "Generated Java files in $JAVA_OUT_DIR"

Write-Host "Protobuf compilation completed successfully." -ForegroundColor Green
