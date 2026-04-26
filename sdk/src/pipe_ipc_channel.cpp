#include "flex_sdk/pipe_ipc_channel.hpp"
#include <iostream>
#include <vector>

namespace flexodbc {

#ifdef _WIN32
PipeIpcChannel::PipeIpcChannel(ProcessLauncher* launcher)
    : stdInWrite_(launcher->getStdInWritePipe()),
      stdOutRead_(launcher->getStdOutReadPipe()) {
    if (stdInWrite_ == NULL || stdOutRead_ == NULL) {
        throw std::runtime_error("Invalid pipe handles provided to PipeIpcChannel.");
    }
}

PipeIpcChannel::~PipeIpcChannel() {
    // Pipe handles are owned by ProcessLauncher, so we don't close them here.
}

void PipeIpcChannel::writeBytes(const void* data, size_t size) {
    DWORD bytesWritten;
    if (!WriteFile(stdInWrite_, data, static_cast<DWORD>(size), &bytesWritten, NULL)) {
        throw std::runtime_error("Failed to write to pipe (stdInWrite).");
    }
    if (bytesWritten != size) {
        throw std::runtime_error("Incomplete write to pipe (stdInWrite).");
    }
}

void PipeIpcChannel::readBytes(void* data, size_t size) {
    DWORD bytesRead;
    if (!ReadFile(stdOutRead_, data, static_cast<DWORD>(size), &bytesRead, NULL)) {
        throw std::runtime_error("Failed to read from pipe (stdOutRead).");
    }
    if (bytesRead != size) {
        throw std::runtime_error("Incomplete read from pipe (stdOutRead).");
    }
}

void PipeIpcChannel::sendMessage(const google::protobuf::Message& message) {
    std::string serialized_message;
    if (!message.SerializeToString(&serialized_message)) {
        throw std::runtime_error("Failed to serialize Protobuf message.");
    }

    // Length-prefixing: 4 bytes for message size
    uint32_t message_size = static_cast<uint32_t>(serialized_message.length());
    writeBytes(&message_size, sizeof(message_size));
    writeBytes(serialized_message.data(), message_size);
}

void PipeIpcChannel::receiveMessage(google::protobuf::Message& message) {
    // Length-prefixing: Read 4 bytes for message size
    uint32_t message_size;
    readBytes(&message_size, sizeof(message_size));

    std::vector<char> buffer(message_size);
    readBytes(buffer.data(), message_size);

    if (!message.ParseFromArray(buffer.data(), message_size)) {
        throw std::runtime_error("Failed to parse Protobuf message from pipe.");
    }
}

#else
// Implement for Linux/macOS here
PipeIpcChannel::PipeIpcChannel(ProcessLauncher* launcher) {
    throw std::runtime_error("Pipe IPC not yet implemented for this OS.");
}

PipeIpcChannel::~PipeIpcChannel() = default;
void PipeIpcChannel::sendMessage(const google::protobuf::Message& message) {
    throw std::runtime_error("Pipe IPC not yet implemented for this OS.");
}
void PipeIpcChannel::receiveMessage(google::protobuf::Message& message) {
    throw std::runtime_error("Pipe IPC not yet implemented for this OS.");
}
#endif

} // namespace flexodbc

