#include "flex_sdk/process_launcher.hpp"
#include <iostream>

#ifdef _WIN32
#include <vector>
#include <algorithm>
#endif

namespace flexodbc {

ProcessLauncher::ProcessLauncher(const std::string& command) : command_(command) {
#ifdef _WIN32
    ZeroMemory(&pi_, sizeof(pi_));
    ZeroMemory(&si_, sizeof(si_));
    si_.cb = sizeof(si_);
    si_.dwFlags |= STARTF_USESTDHANDLES;
#endif
}

ProcessLauncher::~ProcessLauncher() {
    kill(); // Ensure sidecar is killed on destruction
}

void ProcessLauncher::spawn() {
#ifdef _WIN32
    // Create pipes for redirection
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&stdOutRead_, &stdOutWrite_, &saAttr, 0)) {
        throw std::runtime_error("Stdout pipe creation failed.");
    }
    if (!SetHandleInformation(stdOutRead_, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("StdoutRead handle inheritance failed.");
    }

    if (!CreatePipe(&stdInRead_, &stdInWrite_, &saAttr, 0)) {
        throw std::runtime_error("Stdin pipe creation failed.");
    }
    if (!SetHandleInformation(stdInWrite_, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("StdinWrite handle inheritance failed.");
    }

    si_.hStdError = stdOutWrite_;
    si_.hStdOutput = stdOutWrite_;
    si_.hStdInput = stdInRead_;

    // Convert command to mutable char array for CreateProcessA
    std::vector<char> cmd(command_.begin(), command_.end());
    cmd.push_back('\0');

    if (!CreateProcessA(NULL,   // No module name (use command line)
                        cmd.data(), // Command line
                        NULL,           // Process handle not inheritable
                        NULL,           // Thread handle not inheritable
                        TRUE,           // Set handle inheritance to TRUE
                        0,              // No creation flags
                        NULL,           // Use parent's environment block
                        NULL,           // Use parent's starting directory
                        &si_,           // Pointer to STARTUPINFO
                        &pi_            // Pointer to PROCESS_INFORMATION
    )) {
        CloseHandle(stdInRead_);
        CloseHandle(stdInWrite_);
        CloseHandle(stdOutRead_);
        CloseHandle(stdOutWrite_);
        throw std::runtime_error("Failed to spawn sidecar process. Error: " + std::to_string(GetLastError()));
    }

    // Close the handles that are now inherited by the child process.
    // These are no longer needed by the parent process.
    CloseHandle(stdInRead_);
    CloseHandle(stdOutWrite_);
    is_spawned_ = true;

    std::cout << "[SDK] Sidecar process spawned with PID: " << pi_.dwProcessId << std::endl;

#else
    // TODO: Implement for Linux/macOS using fork/exec
    throw std::runtime_error("Process spawning not yet implemented for this OS.");
#endif
}

void ProcessLauncher::kill() {
#ifdef _WIN32
    if (is_spawned_) {
        // Terminate the process gently if possible, or forcefully
        if (TerminateProcess(pi_.hProcess, 0)) {
            std::cout << "[SDK] Sidecar process with PID " << pi_.dwProcessId << " terminated." << std::endl;
        } else {
            std::cerr << "[SDK] Failed to terminate sidecar process with PID " << pi_.dwProcessId << ". Error: " << GetLastError() << std::endl;
        }

        // Close all handles
        CloseHandle(pi_.hProcess);
        CloseHandle(pi_.hThread);
        if (stdInWrite_ != NULL) CloseHandle(stdInWrite_);
        if (stdOutRead_ != NULL) CloseHandle(stdOutRead_);

        is_spawned_ = false;
    }
#else
    // TODO: Implement for Linux/macOS
#endif
}

} // namespace flexodbc

