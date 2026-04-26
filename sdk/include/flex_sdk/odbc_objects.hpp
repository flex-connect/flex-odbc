#pragma once

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <mutex>
#include <unordered_set>
#include <memory> // For std::unique_ptr

#include "flex_sdk/process_launcher.hpp" // Include for ProcessLauncher
#include "flex_sdk/ipc_channel.hpp"      // Include for IpcChannel

namespace flexodbc {

// Base class for all ODBC objects to ensure type safety when freeing handles
class OdbcObject {
public:
    virtual ~OdbcObject() = default;
    virtual SQLSMALLINT GetHandleType() const = 0;
};

class Connection; // Forward declaration
class Statement;  // Forward declaration

// Environment (SQL_HANDLE_ENV)
// Manages global state and tracks active connections
class Environment : public OdbcObject {
private:
    SQLINTEGER attr_odbc_version_ = SQL_OV_ODBC3; // Default to ODBC 3
    std::mutex mutex_;
    std::unordered_set<Connection*> connections_;

public:
    Environment() = default;
    ~Environment() override = default;

    SQLSMALLINT GetHandleType() const override { return SQL_HANDLE_ENV; }

    void AddConnection(Connection* conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.insert(conn);
    }

    void RemoveConnection(Connection* conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.erase(conn);
    }
};

// Connection (SQL_HANDLE_DBC)
// Represents a session with a data source
class Connection : public OdbcObject {
private:
    Environment* env_;
    std::mutex mutex_;
    std::unordered_set<Statement*> statements_;
    std::unique_ptr<ProcessLauncher> sidecar_process_;
    std::unique_ptr<IpcChannel> ipc_channel_;

public:
    explicit Connection(Environment* env) : env_(env) {
        if (env_) {
            env_->AddConnection(this);
        }
    }
    
    ~Connection() override {
        if (env_) {
            env_->RemoveConnection(this);
        }
        // ProcessLauncher and IpcChannel destructors will handle their cleanup
    }

    SQLSMALLINT GetHandleType() const override { return SQL_HANDLE_DBC; }
    Environment* GetEnvironment() const { return env_; }

    void AddStatement(Statement* stmt) {
        std::lock_guard<std::mutex> lock(mutex_);
        statements_.insert(stmt);
    }

    void RemoveStatement(Statement* stmt) {
        std::lock_guard<std::mutex> lock(mutex_);
        statements_.erase(stmt);
    }

    // Method to set the sidecar process for this connection
    void SetSidecarProcess(std::unique_ptr<ProcessLauncher> launcher) {
        sidecar_process_ = std::move(launcher);
    }

    // Method to get the sidecar process (for IPC handle access)
    ProcessLauncher* GetSidecarProcess() const {
        return sidecar_process_.get();
    }

    // Method to set the IPC channel for this connection
    void SetIpcChannel(std::unique_ptr<IpcChannel> channel) {
        ipc_channel_ = std::move(channel);
    }

    // Method to get the IPC channel
    IpcChannel* GetIpcChannel() const {
        return ipc_channel_.get();
    }
};

// Statement (SQL_HANDLE_STMT)
// Represents a SQL statement and its execution state
class Statement : public OdbcObject {
private:
    Connection* conn_;

public:
    explicit Statement(Connection* conn) : conn_(conn) {
        if (conn_) {
            conn_->AddStatement(this);
        }
    }
    
    ~Statement() override {
        if (conn_) {
            conn_->RemoveStatement(this);
        }
    }

    SQLSMALLINT GetHandleType() const override { return SQL_HANDLE_STMT; }
    Connection* GetConnection() const { return conn_; }
};

} // namespace flexodbc


