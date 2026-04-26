# Building flex-odbc

To build **flex-odbc**, follow these steps:

## Prerequisites
1. Install CMake.
2. Install a compatible compiler (e.g., GCC, Clang).

## Building the Project
1. Clone the repository:
   ```sh
   git clone https://github.com/your-repo/flex-odbc.git
   cd flex-odbc
   ```
2. Create a build directory:
   ```sh
   mkdir build && cd build
   ```
3. Run CMake to configure the project:
   ```sh
   cmake ..
   ```
4. Build the project using your preferred build system (e.g., `make`, `ninja`):
   ```sh
   make  # or ninja, depending on your configuration
   ```

## Building Samples
To build the implementation samples (Java and C#), follow these steps:

1. Navigate to the sample directory:
   ```sh
   cd samples/java_sample  # or csharp_sample
   ```
2. Build the sample using your preferred build tool (e.g., Maven for Java, MSBuild for C#).

## Running Tests
To run tests, follow these steps:

1. Ensure that all dependencies are installed.
2. Run the test suite using your build system:
   ```sh
   make tests  # or ninja tests, depending on your configuration
   ```

## Troubleshooting
If you encounter any issues during the build process, refer to the [README](README.md) for additional information or seek help from the community.