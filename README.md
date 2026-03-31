# 🚀 Flex-ODBC: High-Performance Polyglot Driver Framework

**Flex-ODBC** is a modern, high-performance C++ framework designed to simplify the creation of native ODBC drivers. By decoupling the legacy Windows/Linux ODBC API from your database logic, it allows you to build a fully functional driver in **C++, C#, or Java** without touching a single line of raw Win32/ODBC boilerplate.

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/Status-Development-orange.svg)]()

---

## ✨ Key Features

* **Contract-First Design:** Defined by a single `driver.proto` file. All communication is typed and validated.
* **Polyglot Sidecar Support:** Write your data-fetching logic in the language of your choice (C#, Java, Python).
* **Zero-Copy Streaming:** Optimized for moving high-volume result sets from your backend to the ODBC manager.
* **Cross-Platform Ready:** Native support for Windows (MSVC), Linux (GCC), and macOS (Clang).
* **Developer Friendly:** Forget the 100+ ODBC function exports; just implement a few clean interfaces.

---

## 🏗️ Architecture at a Glance

Flex-ODBC acts as a **High-Speed Bridge**. It handles the complex state management required by the ODBC Driver Manager and forwards requests to your "Backend" via Protobuf-encoded messages.

1.  **SDK (C++):** The "Bridge" DLL that the OS loads.
2.  **Schema (Protobuf):** The shared "Contract" for requests and data.
3.  **Sidecar (Java/C#/C++):** Your custom database implementation.

> 📘 **Deep Dive:** For a full breakdown of the system design and design patterns used, see [ARCHITECTURE.md](./ARCHITECTURE.md).

---

## 🛠️ Getting Started as an External SDK

We recommend referencing this repository as a submodule or external dependency rather than forking it. This allows you to receive updates to the sdk engine without merge conflicts.

### 1. Prerequisite: Environment Setup
Run our automation script to ensure your compilers (MSVC/GCC), Java SDK, and .NET tools are synchronized:
```powershell
py .\scripts\install_tools.py
```
