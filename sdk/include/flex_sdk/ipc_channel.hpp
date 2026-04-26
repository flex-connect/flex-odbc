#ifndef FLEX_ODBC_IPC_CHANNEL_HPP
#define FLEX_ODBC_IPC_CHANNEL_HPP

#include <string>
#include <google/protobuf/message.h>

namespace flexodbc {

class IpcChannel {
public:
    virtual ~IpcChannel() = default;

    // Sends a Protobuf message, automatically handling length-prefixing.
    virtual void sendMessage(const google::protobuf::Message& message) = 0;

    // Receives a Protobuf message, automatically handling length-prefixing.
    // The caller is responsible for providing an empty message object of the expected type.
    virtual void receiveMessage(google::protobuf::Message& message) = 0;
};

} // namespace flexodbc

#endif // FLEX_ODBC_IPC_CHANNEL_HPP
