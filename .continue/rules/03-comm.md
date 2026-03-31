---
name: Communication Logic
description: Rules for Protobuf serialization and cross-process safety
alwaysApply: true
---

# Communication & AI Instructions

## 1. Protobuf Handshake Protocol
- **Length-Prefixing:** Because IPC pipes are streams, every Protobuf message sent over the wire must be prefixed by a 4-byte integer indicating the message size.
- **Batching:** Use the `RowSet` pattern in the `.proto` file. Never send a single row across the pipe; always batch up to the `max_rows` requested by the C++ host to minimize context switching.

## 2. AI Coding Instructions (For Gemini/Qwen)
- **Memory Safety:** When writing C++, prioritize zero-copy. Use `std::string_view` or raw buffers for Protobuf payloads before they are sent.
- **Error Mapping:** Always map Protobuf `sql_state` strings to the standard ODBC `SQLGetDiagRec` output.
- **Process Lifecycle:** The C++ Host is responsible for the "Clean Kill" of the sidecar process during `SQLDisconnect`. Ensure no zombie processes are left behind.
- **Async Intent:** Use the streaming feature of the `Fetch` RPC to allow the C++ layer to overlap network I/O with data formatting.
