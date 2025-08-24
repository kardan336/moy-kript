cmake_minimum_required(VERSION 3.14)
project(WinCryptor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add all source files
add_executable(WinCryptor 
    main.cpp
    crypt.cpp
    crypt.h
    runpe.cpp
    runpe.h
    anti_debug.cpp
    anti_debug.h
    resource.h
)

# Link Windows libraries
if(WIN32)
    target_link_libraries(WinCryptor PRIVATE 
        user32 
        kernel32 
        gdi32 
        comdlg32 
        comctl32
        ntdll
    )
endif()

# Add resource file if on Windows
if(WIN32)
    target_sources(WinCryptor PRIVATE WinCryptor.rc)
endif()