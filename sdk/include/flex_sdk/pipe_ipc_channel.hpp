#ifndef FLEX_ODBC_PIPE_IPC_CHANNEL_HPP
#define FLEX_ODBC_PIPE_IPC_CHANNEL_HPP

#include "flex_sdk/ipc_channel.hpp"
#include "flex_sdk/process_launcher.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace flexodbc {

class PipeIpcChannel : public IpcChannel {
public:
    // Constructor takes the process launcher to get pipe handles
    explicit PipeIpcChannel(ProcessLauncher* launcher);
    ~PipeIpcChannel() override;

    void sendMessage(const google::protobuf::Message& message) override;
    void receiveMessage(google::protobuf::Message& message) override;

private:
#ifdef _WIN32
    HANDLE stdInWrite_;
    HANDLE stdOutRead_;
#endif

    // Helper for writing raw bytes with length prefix
    void writeBytes(const void* data, size_t size);

    // Helper for reading raw bytes with length prefix
    void readBytes(void* data, size_t size);
};

} // namespace flexodbc

#endif // FLEX_ODBC_PIPE_IPC_CHANNEL_HPP

