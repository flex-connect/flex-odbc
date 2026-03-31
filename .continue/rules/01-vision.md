---
name: Vision
alwaysApply: true
---
# Competitive Goal: flex-odbc vs Enterprise SDKs
We are building an OSS alternative to SimbaEngine and Progress DataDirect. 
Our "Secret Sauce" is:
1. **Sidecar Architecture**: Decoupling the C++ ODBC entry point from the implementation via Protobuf.
2. **Language Agnostic**: Developers shouldn't need to know C++ to build a driver for this framework.
3. **Zero-Copy Intent**: Where possible, use Protobuf's efficiency to minimize latency compared to legacy SDKs.
