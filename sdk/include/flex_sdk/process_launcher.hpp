#ifndef FLEX_ODBC_PROCESS_LAUNCHER_HPP
#define FLEX_ODBC_PROCESS_LAUNCHER_HPP

#include <string>
#include <vector>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif

namespace flexodbc {

class ProcessLauncher {
public:
    explicit ProcessLauncher(const std::string& command);
    ~ProcessLauncher();

    void spawn();
    void kill();

    // Get handles for IPC (Windows specific)
#ifdef _WIN32
    HANDLE getStdInWritePipe() const { return stdInWrite; }
    HANDLE getStdOutReadPipe() const { return stdOutRead; }
#endif

private:
    std::string command_;
#ifdef _WIN32
    PROCESS_INFORMATION pi_;
    STARTUPINFOA si_;
    HANDLE stdInRead_ = NULL;
    HANDLE stdInWrite_ = NULL;
    HANDLE stdOutRead_ = NULL;
    HANDLE stdOutWrite_ = NULL;
    bool is_spawned_ = false;
#endif

    // Prevent copying
    ProcessLauncher(const ProcessLauncher&) = delete;
    ProcessLauncher& operator=(const ProcessLauncher&) = delete;
};

} // namespace flexodbc

#endif // FLEX_ODBC_PROCESS_LAUNCHER_HPP
