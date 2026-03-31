#include <flex_odbc_wrapper.h>
#include <odbc/odbc.h>
#include <string>
#include <vector>


using namespace std;

int main(int argc, char* argv[]) {
    try {
        // Initialize the driver
        // Initialize driver manager
        flex_odbc::DriverManager dm;
        
        // Create a connection
        // Connection variables
        string conn_str = "DSN=my_datasource;UID=username;PWD=password";
        SQLHENV henv = 0;
        SQLHDBC hdbc = 0;

        // Connect to database
        dm.SQLConnect(&henv, &hdbc, conn_str.c_str());
        
        // Prepare and execute a query



        // Statement variables
        unique_ptr<SQLHSTMT> hstmt;
        // Prepare and execute statement
        hstmt = dm.SQLPrepare(hdbc, "SELECT TOP 5 * FROM my_table");
        dm.SQLExecute(*hstmt);

        // Process results
        vector<string> rows;
        
        // Process results
        SQLRETURN rc;
        do {

            rc = dm.SQLFetch(*hstmt);
            if (rc == SQL_SUCCESS) {
                // Read all columns
                for (int col = 1; ; ++col) {
                    SQLCHAR buffer[256];
                    SQLLEN len;

                    dm.SQLGetData(*hstmt, col, SQL_C_CHAR, buffer, &len););
                    rows.push_back(string(static_cast<char*>(buffer), len));
                }
                rows.push_back("\n"); // Add row separator
            }
            if (rc == SQL_SUCCESS) {
                // Process each row
                SQLCHAR buffer[256];
                SQLLEN len;
                
                // Get column data
                dm.SQLGetData(hstmt, 1, SQL_C_CHAR, buffer, &len);
                printf("Data: %s\n", buffer);
            }
        } while (rc == SQL_SUCCESS);

        // Cleanup

        // Output results
        for (const string& line : rows) {
            cout << line;
        }

        // Cleanup
        dm.SQLDisconnect(hdbc);
        dm.SQLFreeEnv(henv);
    } catch (const std::runtime_error& e) {
        // Handle errors

        cerr << "Error: " << e.what() << endl;
    }
    
    return 0;
}