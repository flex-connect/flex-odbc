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

    // Freeing in reverse order
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
