cmake_minimum_required(VERSION 3.18)

if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

set(SOLUTION_NAME TGTIPluginSol)

project(${SOLUTION_NAME})
set(SOURCE_ROOT ${CMAKE_CURRENT_LIST_DIR})
if (WIN32)
    set(HOME_ROOT $ENV{USERPROFILE})
    string(REPLACE "\\" "/" HOME_ROOT "${HOME_ROOT}")
else()
    set(HOME_ROOT $ENV{HOME})
endif()

set(NATID_SDK_ROOT ${HOME_ROOT}/natID.SDK)
include(${NATID_SDK_ROOT}/DevEnv/Common.cmake)

include(${NATID_SDK_ROOT}/DevEnv/natGUI.cmake)
include(${NATID_SDK_ROOT}/DevEnv/MatrixLib.cmake)
include(${NATID_SDK_ROOT}/DevEnv/SymbComp.cmake)

include(TGTIPlugin.cmake)
