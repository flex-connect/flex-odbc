#include <iostream>
#include <windows.h> // For Windows DLL loading
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>

// Macro to check ODBC return codes
#define CHECK_ODBC_SUCCESS(ret, funcName) \
    if ((ret) != SQL_SUCCESS && (ret) != SQL_SUCCESS_WITH_INFO) { \
        std::cerr << "Error in " << (funcName) << ": " << (ret) << std::endl; \
        return 1; \
    }

int main() {
    std::cout << "Starting flex-odbc integration test..." << std::endl;

    SQLHENV env = SQL_NULL_HANDLE;
    SQLHDBC dbc = SQL_NULL_HANDLE;
    SQLRETURN ret;

    // 1. Allocate Environment Handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    CHECK_ODBC_SUCCESS(ret, "SQLAllocHandle (ENV)");

    // Set ODBC version
    ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CHECK_ODBC_SUCCESS(ret, "SQLSetEnvAttr");

    // 2. Allocate Connection Handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    CHECK_ODBC_SUCCESS(ret, "SQLAllocHandle (DBC)");

    // 3. Connect to the data source using SQLDriverConnect
    SQLCHAR inConnectionString[] = "DSN=my_test_dsn;UID=testuser;PWD=testpassword;";
    SQLCHAR outConnectionString[1024];
    SQLSMALLINT outConnectionStringLength;

    std::cout << "Calling SQLDriverConnect..." << std::endl;
    ret = SQLDriverConnect(dbc,
                           NULL, // WindowHandle
                           inConnectionString, SQL_NTS, // InConnectionString, StringLength1
                           outConnectionString, sizeof(outConnectionString), &outConnectionStringLength, // OutConnectionString, StringLength2, StringLength2Ptr
                           SQL_DRIVER_COMPLETE); // DriverCompletion

    CHECK_ODBC_SUCCESS(ret, "SQLDriverConnect");
    std::cout << "SQLDriverConnect successful! Output: " << reinterpret_cast<char*>(outConnectionString) << std::endl;

    // 4. Disconnect
    std::cout << "Calling SQLDisconnect..." << std::endl;
    ret = SQLDisconnect(dbc);
    CHECK_ODBC_SUCCESS(ret, "SQLDisconnect");
    std::cout << "SQLDisconnect successful!" << std::endl;

    // 5. Free Connection Handle
    ret = SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    CHECK_ODBC_SUCCESS(ret, "SQLFreeHandle (DBC)");

    // 6. Free Environment Handle
    ret = SQLFreeHandle(SQL_HANDLE_ENV, env);
    CHECK_ODBC_SUCCESS(ret, "SQLFreeHandle (ENV)");

    std::cout << "flex-odbc integration test completed successfully." << std::endl;
    return 0;
}
