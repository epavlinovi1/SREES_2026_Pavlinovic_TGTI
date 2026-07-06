set(TGTI_PLUGIN_NAME tgti)

file(GLOB TGTI_PLUGIN_SOURCES ${CMAKE_CURRENT_LIST_DIR}/plugin/*.cpp)
file(GLOB TGTI_PLUGIN_INCS ${CMAKE_CURRENT_LIST_DIR}/plugin/*.h)
file(GLOB TGTI_INC_GUI ${NATID_SDK_INC}/gui/*.h)
file(GLOB TGTI_INC_TD ${NATID_SDK_INC}/td/*.h)
file(GLOB TGTI_INC_CNT ${NATID_SDK_INC}/cnt/*.h)
file(GLOB TGTI_INC_MU ${NATID_SDK_INC}/mu/*.h)
file(GLOB TGTI_INC_MEM ${NATID_SDK_INC}/mem/*.h)
file(GLOB TGTI_INC_FO ${NATID_SDK_INC}/fo/*.h)
file(GLOB TGTI_INC_SC ${NATID_SDK_INC}/sc/*.h)
file(GLOB TGTI_INC_SYST ${NATID_SDK_INC}/syst/*.h)
file(GLOB TGTI_INC_DENSE ${NATID_SDK_INC}/dense/*.h)
file(GLOB TGTI_INC_SPARSE ${NATID_SDK_INC}/sparse/*.h)
file(GLOB TGTI_INC_ARCH ${NATID_SDK_INC}/arch/*.h)

add_library(${TGTI_PLUGIN_NAME} SHARED
    ${TGTI_PLUGIN_SOURCES}
    ${TGTI_PLUGIN_INCS}
    ${TGTI_INC_GUI}
    ${TGTI_INC_TD}
    ${TGTI_INC_CNT}
    ${TGTI_INC_MU}
    ${TGTI_INC_MEM}
    ${TGTI_INC_FO}
    ${TGTI_INC_SC}
    ${TGTI_INC_SYST}
    ${TGTI_INC_DENSE}
    ${TGTI_INC_SPARSE}
    ${TGTI_INC_ARCH}
)

if (NOT EXISTS "${NATID_SDK_INC}/arch/MemoryOut.h")
    target_include_directories(${TGTI_PLUGIN_NAME} BEFORE PRIVATE ${CMAKE_CURRENT_LIST_DIR}/compat)
endif()

source_group("inc" FILES ${TGTI_PLUGIN_INCS})
source_group("inc\\gui" FILES ${TGTI_INC_GUI})
source_group("inc\\td" FILES ${TGTI_INC_TD})
source_group("inc\\cnt" FILES ${TGTI_INC_CNT})
source_group("inc\\mu" FILES ${TGTI_INC_MU})
source_group("inc\\mem" FILES ${TGTI_INC_MEM})
source_group("inc\\fo" FILES ${TGTI_INC_FO})
source_group("inc\\sc" FILES ${TGTI_INC_SC})
source_group("inc\\syst" FILES ${TGTI_INC_SYST})
source_group("inc\\dense" FILES ${TGTI_INC_DENSE})
source_group("inc\\sparse" FILES ${TGTI_INC_SPARSE})
source_group("inc\\arch" FILES ${TGTI_INC_ARCH})
source_group("src" FILES ${TGTI_PLUGIN_SOURCES})

option(TGTI_LINK_RELEASE_NATID_RUNTIME "Link release natID runtime libraries for dTwin host compatibility." ON)

if (TGTI_LINK_RELEASE_NATID_RUNTIME)
    target_link_libraries(${TGTI_PLUGIN_NAME}
        ${MU_LIB_RELEASE}
        ${MATRIX_LIB_RELEASE}
        ${NATGUI_LIB_RELEASE}
        ${SYMBCOMP_LIB_RELEASE}
    )
else()
    target_link_libraries(${TGTI_PLUGIN_NAME}
        debug ${MU_LIB_DEBUG} optimized ${MU_LIB_RELEASE}
        debug ${MATRIX_LIB_DEBUG} optimized ${MATRIX_LIB_RELEASE}
        debug ${NATGUI_LIB_DEBUG} optimized ${NATGUI_LIB_RELEASE}
        debug ${SYMBCOMP_LIB_DEBUG} optimized ${SYMBCOMP_LIB_RELEASE}
    )
endif()

target_compile_definitions(${TGTI_PLUGIN_NAME} PUBLIC PLUGIN_EXPORTS)

if (MSVC)
    set_property(TARGET ${TGTI_PLUGIN_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    target_compile_options(${TGTI_PLUGIN_NAME} PRIVATE /MD)
    target_compile_definitions(${TGTI_PLUGIN_NAME} PRIVATE _ITERATOR_DEBUG_LEVEL=0)
endif()

if (COMMAND setIDEPropertiesForLib)
    setIDEPropertiesForLib(${TGTI_PLUGIN_NAME})
endif()
