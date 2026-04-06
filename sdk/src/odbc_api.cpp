#include <iostream>
#include <cassert>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

// Forward declare the exported functions from the DLL to test them directly without DM
extern "C" {
    SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT HandleType, SQLHANDLE InputHandle, SQLHANDLE *OutputHandle);
    SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle);
    SQLRETURN SQL_API SQLAllocEnv(SQLHENV *OutputHandle);
    SQLRETURN SQL_API SQLFreeEnv(SQLHENV EnvironmentHandle);
    SQLRETURN SQL_API SQLAllocConnect(SQLHENV EnvironmentHandle, SQLHDBC *ConnectionHandle);
    SQLRETURN SQL_API SQLFreeConnect(SQLHDBC ConnectionHandle);
}

#include "odbc_objects.hpp"

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
    SQLRETURN ret = SQLAllocEnv(&env);
    assert(ret == SQL_SUCCESS);
    assert(env != SQL_NULL_HANDLE);

    SQLHDBC dbc = SQL_NULL_HANDLE;
    ret = SQLAllocConnect(env, &dbc);
    assert(ret == SQL_SUCCESS);
    assert(dbc != SQL_NULL_HANDLE);

    ret = SQLFreeConnect(dbc);
    assert(ret == SQL_SUCCESS);

    ret = SQLFreeEnv(env);
    assert(ret == SQL_SUCCESS);

    std::cout << "[SDK Test] Legacy Handle allocation passed." << std::endl;
}

int main() {
    std::cout << "Starting SDK internal tests..." << std::endl;

    TestHandleAllocation();
    TestLegacyAllocation();

    std::cout << "SDK internal tests completed successfully." << std::endl;
    return 0;
}