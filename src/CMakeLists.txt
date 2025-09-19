cmake_minimum_required(VERSION 3.25.2)
project(memory_crc C)

set(CMAKE_C_STANDARD 99)

set(KP_DIR "../__KernelPatch_lib")
set(INCLUDE_DIRS . include patch/include linux/include kpm/include linux/arch/arm64/include linux/tools/arch/arm64/include)

foreach(dir ${INCLUDE_DIRS})
    include_directories(${KP_DIR}/kernel/${dir})
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/control)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/komm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/data)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/redirect)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/feature)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ptrace)

set(OUTPUT_NAME "yuuki")

set(CMAKE_C_COMPILER /opt/android-ndk-r27/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android33-clang)

set(SOURCES
        mod_main.c
        control/control.c
        komm/komm.c
        redirect/redirect.c
        feature/feature.c
        data/shared_data.c
        data/parser.c
        data/kstring.c
        ptrace/ptrace_hide.c
        symbols.h
)

add_library(${OUTPUT_NAME} SHARED ${SOURCES})

set_target_properties(${OUTPUT_NAME} PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}")

add_custom_command(TARGET ${OUTPUT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:${OUTPUT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_NAME}.kpm
)