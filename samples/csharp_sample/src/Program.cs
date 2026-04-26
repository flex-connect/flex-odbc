using FlexOdbc.Sidecar.Proto;
using Google.Protobuf;
using System.Data.Odbc; // For local SQL server connection

namespace FlexOdbc.Sidecar.CSharp
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("C# Sidecar started. Waiting for commands...");

            // Use standard input/output for IPC communication with length-prefixing
            using (var stdin = Console.OpenStandardInput())
            using (var stdout = Console.OpenStandardOutput())
            {
                while (true)
                {
                    // Read message length (4 bytes)
                    byte[] lengthBytes = new byte[4];
                    int bytesRead = stdin.Read(lengthBytes, 0, 4);
                    if (bytesRead == 0) // End of stream, parent process disconnected
                    {
                        Console.WriteLine("Parent process disconnected. Exiting.");
                        break;
                    }
                    if (bytesRead < 4)
                    {
                        Console.Error.WriteLine($"Received {bytesRead} bytes, expected 4 for message length.");
                        // Handle error or partial read
                        break;
                    }

                    uint messageLength = BitConverter.ToUInt32(lengthBytes, 0);
                    byte[] messageBytes = new byte[messageLength];
                    bytesRead = stdin.Read(messageBytes, 0, (int)messageLength);
                    if (bytesRead < messageLength)
                    {
                        Console.Error.WriteLine($"Received {bytesRead} bytes, expected {messageLength} for message payload.");
                        // Handle error or partial read
                        break;
                    }

                    // Determine message type and deserialize
                    // For now, let's assume the first message is always DriverConnectRequest
                    // In a more robust system, you'd have a message envelope with a type ID
                    // or a way to infer the message type.
                    try
                    {
                        // Attempt to parse as DriverConnectRequest
                        DriverConnectRequest connectRequest = DriverConnectRequest.Parser.ParseFrom(messageBytes);
                        Console.WriteLine($"Received DriverConnectRequest: {connectRequest.ConnectionString}");

                        DriverConnectResponse connectResponse = HandleDriverConnect(connectRequest);
                        SendProtobufMessage(stdout, connectResponse);
                    }
                    catch (InvalidProtocolBufferException)
                    {
                        // Fallback: Attempt to parse as DisconnectRequest
                        try
                        {
                            DisconnectRequest disconnectRequest = DisconnectRequest.Parser.ParseFrom(messageBytes);
                            Console.WriteLine("Received DisconnectRequest.");
                            DisconnectResponse disconnectResponse = HandleDisconnect(disconnectRequest);
                            SendProtobufMessage(stdout, disconnectResponse);
                            // After disconnect, we typically exit
                            break;
                        }
                        catch (InvalidProtocolBufferException)
                        {
                            Console.Error.WriteLine("Received unknown Protobuf message type.");
                            // Send an error response or just break
                            break;
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.Error.WriteLine($"Error handling message: {ex.Message}");
                        break; // Exit on error
                    }
                }
            }
        }

        static DriverConnectResponse HandleDriverConnect(DriverConnectRequest request)
        {
            try
            {
                using (OdbcConnection connection = new OdbcConnection(request.ConnectionString))
                {
                    connection.Open();
                    return new DriverConnectResponse
                    {
                        Success = true,
                        SqlState = "00000", // Success
                        ErrorMessage = "Connection successful."
                    };
                }
            }
            catch (OdbcException ex)
            {
                // Get SQLState from the first OdbcError, or default to a general error
                string sqlState = "HY000";
                if (ex.Errors.Count > 0)
                {
                    sqlState = ex.Errors[0].SQLState;
                }
                return new DriverConnectResponse
                {
                    Success = false,
                    SqlState = sqlState,
                    ErrorMessage = ex.Message
                };
            }
            catch (Exception ex)
            {
                return new DriverConnectResponse
                {
                    Success = false,
                    SqlState = "HY000", // General Error
                    ErrorMessage = ex.Message
                };
            }
        }

        static DisconnectResponse HandleDisconnect(DisconnectRequest request)
        {
            Console.WriteLine("Handling Disconnect Request.");
            // Perform any cleanup here if necessary
            return new DisconnectResponse { Success = true };
        }

        static void SendProtobufMessage(Stream outputStream, IMessage message)
        {
            byte[] serializedMessage = message.ToByteArray();
            uint messageLength = (uint)serializedMessage.Length;

            // Write length prefix (4 bytes)
            byte[] lengthBytes = BitConverter.GetBytes(messageLength);
            outputStream.Write(lengthBytes, 0, 4);

            // Write actual message
            outputStream.Write(serializedMessage, 0, serializedMessage.Length);
            outputStream.Flush();
        }
    }
}

