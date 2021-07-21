CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

project (autofix-mkcc
    VERSION "0.0.1"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ext/argh)

add_executable (autofix-mkcc source.cpp)

target_include_directories(autofix-mkcc PUBLIC ${argh_INCLUDE_DIR})
target_link_libraries(autofix-mkcc PUBLIC argh)