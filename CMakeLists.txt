cmake_minimum_required(VERSION 3.20)
project(SMRadioMod VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG master
)
FetchContent_MakeAvailable(minhook)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281)
FetchContent_MakeAvailable(fmt)

add_library(${PROJECT_NAME} SHARED 
    src/main.cpp
    src/hooks.cpp
    src/hooks.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/inc
)
target_link_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/lib/x64
)

target_link_libraries(${PROJECT_NAME} PRIVATE minhook fmod_vc fmodstudio_vc fmt::fmt)