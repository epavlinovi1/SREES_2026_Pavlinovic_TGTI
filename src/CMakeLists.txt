cmake_minimum_required(VERSION 3.18)

if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# Convenience entry point for IDEs/CMake GUI.
# The natID-style project files live in src/.
add_subdirectory(src)
