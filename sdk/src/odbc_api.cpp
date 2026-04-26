#include <iostream>
#include <cassert>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <algorithm>

// Forward declare the exported functions from the DLL to test them directly without DM
extern "C" {
    SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT HandleType, SQLHANDLE InputHandle, SQLHANDLE *OutputHandle);
    SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle);
    /// @brief 
    /// @param OutputHandle 
    /// @return 
    SQLRETURN SQL_API SQLAllocEnv(SQLHENV *OutputHandle);
    SQLRETURN SQL_API SQLFreeEnv(SQLHENV EnvironmentHandle);
    SQLRETURN SQL_API SQLAllocConnect(SQLHENV EnvironmentHandle, SQLHDBC *ConnectionHandle);
    SQLRETURN SQL_API SQLFreeConnect(SQLHDBC ConnectionHandle);
    SQLRETURN SQL_API SQLConnect(SQLHDBC ConnectionHandle,
                                 SQLCHAR *ServerName, SQLSMALLINT NameLength1,
                                 SQLCHAR *UserName, SQLSMALLINT NameLength2,
                                 SQLCHAR *Authentication, SQLSMALLINT NameLength3);
    SQLRETURN SQL_API SQLDriverConnect(SQLHDBC ConnectionHandle,
                                       SQLHWND WindowHandle,
                                       SQLCHAR *InConnectionString, SQLSMALLINT StringLength1,
                                       SQLCHAR *OutConnectionString, SQLSMALLINT StringLength2,
                                       SQLSMALLINT *StringLength2Ptr,
                                       SQLUSMALLINT DriverCompletion);
}

#include "proto_gen/driver.pb.h"
#include "odbc_objects.hpp"
#include "config_reader.hpp"
#include "process_launcher.hpp"
#include "pipe_ipc_channel.hpp"

extern "C" {

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT HandleType, SQLHANDLE InputHandle, SQLHANDLE *OutputHandle) {
    if (!OutputHandle) {
        return SQL_INVALID_HANDLE;
    }

    *OutputHandle = SQL_NULL_HANDLE;

    try {
        switch (HandleType) {
            case SQL_HANDLE_ENV:
                *OutputHandle = new flexodbc::Environment();
                return SQL_SUCCESS;

            case SQL_HANDLE_DBC:
                if (!InputHandle) return SQL_INVALID_HANDLE;
                *OutputHandle = new flexodbc::Connection(static_cast<flexodbc::Environment*>(InputHandle));
                return SQL_SUCCESS;

            case SQL_HANDLE_STMT:
                if (!InputHandle) return SQL_INVALID_HANDLE;
                *OutputHandle = new flexodbc::Statement(static_cast<flexodbc::Connection*>(InputHandle));
                return SQL_SUCCESS;

            // TODO: Implement SQL_HANDLE_DESC
            default:
                return SQL_ERROR;
        }
    } catch (...) {
        return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle) {
    if (!Handle) {
        return SQL_INVALID_HANDLE;
    }

    auto obj = static_cast<flexodbc::OdbcObject*>(Handle);

    // Type validation
    if (obj->GetHandleType() != HandleType) {
        return SQL_INVALID_HANDLE;
    }

    try {
        switch (HandleType) {
            case SQL_HANDLE_ENV:
                delete static_cast<flexodbc::Environment*>(obj);
                return SQL_SUCCESS;

            case SQL_HANDLE_DBC:
                delete static_cast<flexodbc::Connection*>(obj);
                return SQL_SUCCESS;

            case SQL_HANDLE_STMT:
                delete static_cast<flexodbc::Statement*>(obj);
                return SQL_SUCCESS;

            default:
                return SQL_ERROR;
        }
    } catch (...) {
        return SQL_ERROR;
    }
}

// -------------------------------------------------------------------------
// ODBC 2.x Backward Compatibility Wrappers
// Driver Managers often map SQLAllocEnv -> SQLAllocHandle internally, but
// some older applications might bypass the DM or expect these to exist.
// -------------------------------------------------------------------------

SQLRETURN SQL_API SQLAllocEnv(SQLHENV *OutputHandle) {
    return SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, OutputHandle);
}

SQLRETURN SQL_API SQLFreeEnv(SQLHENV EnvironmentHandle) {
    return SQLFreeHandle(SQL_HANDLE_ENV, EnvironmentHandle);
}

SQLRETURN SQL_API SQLAllocConnect(SQLHENV EnvironmentHandle, SQLHDBC *ConnectionHandle) {
    return SQLAllocHandle(SQL_HANDLE_DBC, EnvironmentHandle, ConnectionHandle);
}

SQLRETURN SQL_API SQLFreeConnect(SQLHDBC ConnectionHandle) {
    return SQLFreeHandle(SQL_HANDLE_DBC, ConnectionHandle);
}

SQLRETURN SQL_API SQLConnect(SQLHDBC ConnectionHandle,
                             SQLCHAR *ServerName, SQLSMALLINT NameLength1,
                             SQLCHAR *UserName, SQLSMALLINT NameLength2,
                             SQLCHAR *Authentication, SQLSMALLINT NameLength3) {
    // SQLConnect can typically be implemented by constructing a connection string
    // and calling SQLDriverConnect.
    // For simplicity, we'll just return SQL_ERROR for now or route it to SQLDriverConnect
    // with a constructed connection string.
    // TODO: Construct connection string from ServerName, UserName, Authentication
    return SQL_ERROR;
}

SQLRETURN SQL_API SQLDriverConnect(SQLHDBC ConnectionHandle,
                                   SQLHWND WindowHandle,
                                   SQLCHAR *InConnectionString, SQLSMALLINT StringLength1,
                                   SQLCHAR *OutConnectionString, SQLSMALLINT StringLength2,
                                   SQLSMALLINT *StringLength2Ptr,
                                   SQLUSMALLINT DriverCompletion) {
    if (!ConnectionHandle) {
        return SQL_INVALID_HANDLE;
    }

    auto connection = static_cast<flexodbc::Connection*>(ConnectionHandle);

    std::cout << "[SDK] SQLDriverConnect called with: " << reinterpret_cast<const char*>(InConnectionString) << std::endl;

    // 1. Read driver.properties
    try {
        flexodbc::ConfigReader config("config/driver.properties");
        std::string implementation_cmd = config.getProperty("implementation.cmd");
        std::string protocol_type = config.getProperty("protocol.type");
        std::string ipc_mode = config.getProperty("ipc.mode");

        std::cout << "[SDK] Driver Config: "
                  << "cmd='" << implementation_cmd << "' "
                  << "protocol='" << protocol_type << "' "
                  << "ipc='" << ipc_mode << "'" << std::endl;

        // 2. Spawn sidecar process
        auto launcher = std::make_unique<flexodbc::ProcessLauncher>(implementation_cmd);
        launcher->spawn();
        connection->SetSidecarProcess(std::move(launcher));
        // 3. Establish IPC communication based on ipc_mode
        if (ipc_mode == "pipe") {
            auto pipe_channel = std::make_unique<flexodbc::PipeIpcChannel>(connection->GetSidecarProcess());
            connection->SetIpcChannel(std::move(pipe_channel));
        } else if (ipc_mode == "socket") {
            // TODO: Implement SocketIpcChannel
            std::cerr << "[SDK] Socket IPC mode not yet implemented." << std::endl;
        return SQL_ERROR;
    } else {
            std::cerr << "[SDK] Unknown IPC mode: " << ipc_mode << std::endl;
        return SQL_ERROR;
    }

    } catch (const std::runtime_error& e) {
        std::cerr << "[SDK] Error in SQLDriverConnect: " << e.what() << std::endl;
        return SQL_ERROR;
    }

    flexodbc::DriverConnectRequest request;
    request.set_connection_string(std::string(reinterpret_cast<const char*>(InConnectionString), StringLength1));

    flexodbc::DriverConnectResponse response;

    try {
        connection->GetIpcChannel()->sendMessage(request);
        connection->GetIpcChannel()->receiveMessage(response);
    } catch (const std::runtime_error& e) {
        std::cerr << "[SDK] IPC communication error: " << e.what() << std::endl;
        return SQL_ERROR;
    }

    if (response.success()) {
        std::string out_conn_str = request.connection_string(); // In a real scenario, this might come from the sidecar
    if (OutConnectionString && StringLength2 > 0) {
            strncpy_s(reinterpret_cast<char*>(OutConnectionString), StringLength2, out_conn_str.c_str(), _TRUNCATE);
        if (StringLength2Ptr) {
                *StringLength2Ptr = static_cast<SQLSMALLINT>(std::min(static_cast<size_t>(StringLength2), out_conn_str.length()));
            }
        } else if (StringLength2Ptr) {
            *StringLength2Ptr = static_cast<SQLSMALLINT>(out_conn_str.length());
        }
        return SQL_SUCCESS;
    } else {
        // TODO: Map Protobuf sql_state and error_message to ODBC diagnostic records
        std::cerr << "[SDK] SQLDriverConnect failed: " << response.error_message() << " (SQLSTATE: " << response.sql_state() << ")" << std::endl;
        return SQL_ERROR;
}
}

SQLRETURN SQL_API SQLDisconnect(SQLHDBC ConnectionHandle) {
    if (!ConnectionHandle) {
        return SQL_INVALID_HANDLE;
    }

    auto connection = static_cast<flexodbc::Connection*>(ConnectionHandle);
    std::cout << "[SDK] SQLDisconnect called." << std::endl;

    if (connection->GetIpcChannel()) {
        flexodbc::DisconnectRequest request;
        flexodbc::DisconnectResponse response;
        try {
            connection->GetIpcChannel()->sendMessage(request);
            connection->GetIpcChannel()->receiveMessage(response);
            if (!response.success()) {
                std::cerr << "[SDK] Sidecar reported disconnect failure." << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "[SDK] IPC disconnect error: " << e.what() << std::endl;
        }
    }

    // Always attempt to kill the process from the host side to prevent zombies
    if (connection->GetSidecarProcess()) {
        connection->GetSidecarProcess()->kill();
        connection->SetSidecarProcess(nullptr); // Detach unique_ptr
    }

    connection->SetIpcChannel(nullptr); // Detach unique_ptr

    return SQL_SUCCESS;
}

} // extern "C"

void TestHandleAllocation() {
    std::cout << "[SDK Test] Testing SQLAllocHandle / SQLFreeHandle..." << std::endl;

    SQLHENV env = SQL_NULL_HANDLE;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    assert(ret == SQL_SUCCESS);
    assert(env != SQL_NULL_HANDLE);

    SQLHDBC dbc = SQL_NULL_HANDLE;
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    assert(ret == SQL_SUCCESS);
    assert(dbc != SQL_NULL_HANDLE);

    SQLHSTMT stmt = SQL_NULL_HANDLE;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    assert(ret == SQL_SUCCESS);
    assert(stmt != SQL_NULL_HANDLE);

    // Freeing in reverse order (Stmt -> Conn -> Env)
    ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    assert(ret == SQL_SUCCESS);

    ret = SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    assert(ret == SQL_SUCCESS);

    ret = SQLFreeHandle(SQL_HANDLE_ENV, env);
    assert(ret == SQL_SUCCESS);

    std::cout << "[SDK Test] Handle allocation passed." << std::endl;
}

void TestLegacyAllocation() {
    std::cout << "[SDK Test] Testing SQLAllocEnv / SQLFreeEnv (Legacy API)..." << std::endl;

    SQLHENV env = SQL_NULL_HANDLE;
    SQLRETURN ret


