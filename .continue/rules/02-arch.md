---
name: Architecture
description: Delivery format and project structure rules
alwaysApply: true
---

# Project Structure & Deliverables
The goal of **flex-odbc** is to provide a high-performance C++ wrapper that delegates data retrieval to external processes.

## 1. The C++ S Library (The Host)
- **Output:** A shared library (`.dll` on Windows, `.so` on Linux).
- **Entry Points:** Standard ODBC 3.8 API exports (e.g., `SQLConnect`, `SQLExecute`, `SQLFetch`).
- **Responsibility:**
  - Read `driver.properties` to find the implementation path.
  - Spawn the external "Sidecar" process (Java, C#, etc.).
  - Serialize/Deserialize Protobuf messages over an IPC pipe.

## 2. The Discovery Mechanism
- Every driver instance must have a `.properties` file containing:
  - `implementation.cmd`: The shell command to start the sidecar.
  - `protocol.type`: Defaults to `protobuf`.
  - `ipc.mode`: `pipe` or `socket`.

## 3. The Implementation Samples
- Located in `/samples/java_sample` and `/samples/csharp_sample`.
- These are standalone executables that start a Protobuf server and wait for commands from the C++ Host.
