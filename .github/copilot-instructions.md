# WinCryptor

WinCryptor is a Windows cryptor application written in C++ that creates encrypted stubs for PE/EXE files with anti-analysis features. The application uses Visual Studio for the main GUI and MinGW-w64 for runtime stub compilation.

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

## Working Effectively

### Required Dependencies
- **CRITICAL**: Install MinGW-w64 cross-compiler for stub generation:
  - `sudo apt update && sudo apt install -y mingw-w64`
  - NEVER CANCEL: Package installation takes 5-10 minutes. Set timeout to 15+ minutes.
- Verify MinGW installation:
  - `which x86_64-w64-mingw32-g++` (should return `/usr/bin/x86_64-w64-mingw32-g++`)
  - `which i686-w64-mingw32-g++` (should return `/usr/bin/i686-w64-mingw32-g++`)

### Build and Test Process
- **Test stub compilation environment**:
  - Create test stub: 
    ```bash
    cat > /tmp/test_stub.cpp << 'EOF'
    #include <windows.h>
    int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { 
        return 0; 
    }
    EOF
    ```
  - Compile: `x86_64-w64-mingw32-g++ -std=c++17 -O2 -mwindows /tmp/test_stub.cpp -o /tmp/test_stub.exe`
  - Expected time: ~1 second. NEVER CANCEL: Set timeout to 30+ seconds.
- **Fix missing header issue**: The `randomized_stub.cpp` file is missing `#include <algorithm>` which is required for `std::shuffle`. Always add this include after the other headers.
- **Test individual components**:
  - `x86_64-w64-mingw32-g++ -std=c++17 -c plugin_manager.cpp -o /tmp/plugin_manager.o`
  - `x86_64-w64-mingw32-g++ -std=c++17 -c randomized_stub.cpp -o /tmp/randomized_stub.o` (after fixing header)
  - Expected time: 1-2 seconds per file. NEVER CANCEL: Set timeout to 60+ seconds.

### Main Application Build
- **IMPORTANT**: The main GUI application requires Visual Studio and Windows SDK to build completely
- **DO NOT attempt to build the complete application on Linux** - it uses Windows-specific APIs and resources
- **Focus on testing stub generation components** which use MinGW cross-compilation

## Validation

### Manual Testing Scenarios
- **Always test stub compilation** after making changes to `randomized_stub.cpp` or `stub_template.cpp`
- **Verify MinGW cross-compilation works** by compiling simple Windows programs
- **Test stub generation timing** - compilation should complete in under 2 seconds for simple stubs
- **Cannot run the Windows GUI application** in this Linux environment - focus on testing compilation of individual components

### Build Time Expectations
- **MinGW package installation**: 5-10 minutes. NEVER CANCEL. Set timeout to 15+ minutes.
- **Simple stub compilation**: 1-2 seconds. NEVER CANCEL. Set timeout to 30+ seconds.
- **Individual C++ file compilation**: 1-2 seconds. NEVER CANCEL. Set timeout to 60+ seconds.
- **Full application build**: Not possible on Linux - requires Windows/Visual Studio

## Common Tasks

The following are outputs from frequently run commands. Reference them instead of viewing, searching, or running bash commands to save time.

### Repository Structure
```
WinCryptor.sln              # Visual Studio solution file
WinCryptor.vcxproj          # Visual Studio project file
stub_template.cpp           # Template for generated stubs (has placeholders)
randomized_stub.cpp/.h      # Stub generation logic (needs #include <algorithm>)
main.cpp                    # Application entry point
lang.cpp/.h                 # Localization support (Russian/English)
plugin_manager.cpp/.h       # Plugin loading system
thirdparty_integration.cpp  # Third-party tool integration
main_window.cpp/.h          # GUI implementation
special_crypt_tab.cpp/.h    # Encryption tab GUI
resource.h                  # Resource definitions
cryptor.rc                  # Windows resource file
WinCryptor.zip             # Archive with alternative project structure
```

### Key Dependencies and Limitations
- **MinGW-w64**: Required for stub compilation at runtime (`g++` command in stub generation)
- **Windows APIs**: All GUI code uses Windows.h and Win32 APIs
- **Visual Studio**: Required for complete application build (not available in Linux environment)
- **Third-party tools**: Application expects `tools/` directory with:
  - `tools/ConfuserEx/Confuser.CLI.exe`
  - `tools/EnigmaProtector/Enigma.exe`
  - `tools/DotNetToJScript/DotNetToJScript.py`

### Stub Generation Process
The core functionality involves:
1. Reading `stub_template.cpp` 
2. Replacing placeholders like `/*JUNK1*/`, `//ANTIDEBUG`, `/*PAYLOAD*/` with generated code
3. Compiling the modified template using: `g++ -std=c++17 -O2 -mwindows stub_build.cpp -o stub_build.exe`
4. Reading the compiled executable as bytes

### Known Issues
- `randomized_stub.cpp` missing `#include <algorithm>` for `std::shuffle`
- Cannot build complete GUI application on Linux (Windows-only)
- Third-party tools directory (`tools/`) not included in repository

### Testing Commands
```bash
# Test MinGW availability
which x86_64-w64-mingw32-g++ i686-w64-mingw32-g++

# Test simple Windows program compilation
cat > /tmp/test.cpp << 'EOF'
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { 
    return 0; 
}
EOF
x86_64-w64-mingw32-g++ -std=c++17 -O2 -mwindows /tmp/test.cpp -o /tmp/test.exe

# Test individual component compilation (after fixing headers)
x86_64-w64-mingw32-g++ -std=c++17 -c plugin_manager.cpp -o /tmp/plugin_manager.o
x86_64-w64-mingw32-g++ -std=c++17 -c randomized_stub.cpp -o /tmp/randomized_stub.o
```

## Critical Reminders
- **NEVER CANCEL BUILDS**: All compilation should complete in under 2 minutes
- **ALWAYS fix the algorithm header** in `randomized_stub.cpp` before testing
- **DO NOT attempt Visual Studio builds** on Linux
- **FOCUS on stub generation testing** - this is the core cross-platform functionality
- **VERIFY MinGW installation** before making any code changes