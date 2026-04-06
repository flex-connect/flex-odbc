using Grpc.Core;
using flexodbc;
using System.Data.Odbc; // For local SQL server connection

namespace Sidecar
{
    class OdbcDriverImpl : OdbcDriver.OdbcDriverBase
    {
        public override Task<ConnectResponse> Connect(ConnectRequest request, ServerCallContext context)
        {
            try
            {
                // Attempt to connect to a local SQL server using the provided connection string
                using (OdbcConnection connection = new OdbcConnection(request.InConnectionString))
                {
                    connection.Open();
                    // If connection is successful, return success
                    return Task.FromResult(new ConnectResponse
                    {
                        OutConnectionString = request.InConnectionString,
                        OutConnectionStringLength = request.InConnectionString.Length,
                        SqlState = "00000", // Success
                        NativeError = 0,
                        MessageText = "Connection successful."
                    });
                }
            }
            catch (OdbcException ex)
            {
                // Handle ODBC specific errors
                return Task.FromResult(new ConnectResponse
                {
                    OutConnectionString = "",
                    OutConnectionStringLength = 0,
                    SqlState = ex.SQLState,
                    NativeError = ex.NativeError,
                    MessageText = ex.Message
                });
            }
            catch (Exception ex)
            {
                // Handle any other general exceptions
                return Task.FromResult(new ConnectResponse
                {
                    OutConnectionString = "",
                    OutConnectionStringLength = 0,
                    SqlState = "HY000", // General Error
                    NativeError = -1, // Indicate a non-ODBC specific error
                    MessageText = ex.Message
                });
            }
        }
    }

    class Program
    {
        const int Port = 50051; // This port should be configured to be dynamic later

        public static void Main(string[] args)
        {
            Server server = new Server
            {
                Services = { OdbcDriver.BindService(new OdbcDriverImpl()) },
                Ports = { new ServerPort("localhost", Port, ServerCredentials.Insecure) }
            };
            server.Start();

            Console.WriteLine("Sidecar server listening on port " + Port);
            Console.WriteLine("Press any key to stop the server...");
            Console.ReadKey();

            server.ShutdownAsync().Wait();
        }
    }
}
