cmake_minimum_required(VERSION 3.14)
project(WinCryptor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(WinCryptor WinCryptor/WinCryptor.cpp WinCryptor/WinCryptor.h)
add_executable(WinCryptorApp main.cpp)
target_link_libraries(WinCryptorApp PRIVATE WinCryptor)

# Тесты
enable_testing()
add_subdirectory(tests)