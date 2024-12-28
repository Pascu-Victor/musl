cmake_minimum_required(VERSION 3.10)

set(LLVM_PATH ${CMAKE_SOURCE_DIR}../../../../tools/llvm-build/bin)
set(CMAKE_C_COMPILER "${LLVM_PATH}/clang" CACHE INTERNAL "Path to clang")
set(CMAKE_LINKER_TYPE "LLD")
set(CMAKE_ASSEMBLER "${LLVM_PATH}/clang" CACHE INTERNAL "Path to clang")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -nostartfiles -nodefaultlibs")

project(musl LANGUAGES C ASM)

set (ARCH "x86_64")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c23 -fdiagnostics-color=always -Os -pipe -ffreestanding -nostdinc -D_XOPEN_SOURCE=700")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__x86_64__")

# Directories and includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/generic
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Gather sources
file(GLOB BASE_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ldso/*.c"
    )
file(GLOB ARCH_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*/${ARCH}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/crt/${ARCH}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ldso/${ARCH}/*.c"
    )
# Gather asm sources
file(GLOB ARCH_ASM_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/\**/${ARCH}/*.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/${ARCH}/*.s"
)
list(REMOVE_DUPLICATES ARCH_SRCS)
list(REMOVE_DUPLICATES ARCH_ASM_SRCS)
set(ALL_SRCS ${BASE_SRCS} ${ARCH_SRCS})

# Static library
add_library(musl_static STATIC ${ALL_SRCS})

# Shared library
add_library(musl_shared SHARED ${ALL_SRCS})

#asm sources
target_sources(musl_static PRIVATE ${ARCH_ASM_SRCS})
target_sources(musl_shared PRIVATE ${ARCH_ASM_SRCS})

# .in files

# Set up custom commands to generate header files
set(ALLTYPES_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/bits/alltypes.h)
set(SYSCALL_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/bits/syscall.h)

add_custom_command(
    OUTPUT ${ALLTYPES_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/bits
    COMMAND sed -f ${CMAKE_CURRENT_SOURCE_DIR}/tools/mkalltypes.sed
            ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/bits/alltypes.h.in
            ${CMAKE_CURRENT_SOURCE_DIR}/include/alltypes.h.in > ${ALLTYPES_OUTPUT}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/bits/alltypes.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alltypes.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/mkalltypes.sed
)

add_custom_command(
    OUTPUT ${SYSCALL_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/bits
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/bits/syscall.h.in ${SYSCALL_OUTPUT}
    COMMAND sed -n -e s/__NR_/SYS_/p < ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/bits/syscall.h.in >> ${SYSCALL_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/bits/syscall.h.in
)

# Add a custom target to ensure headers are generated
add_custom_target(generate_headers DEPENDS ${ALLTYPES_OUTPUT} ${SYSCALL_OUTPUT})

# Make sure the libraries depend on the generated headers
add_dependencies(musl_static generate_headers)
add_dependencies(musl_shared generate_headers)

# Add generated headers directory to includes
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/src/internal
)

# generate version.h

execute_process(
    COMMAND bash -c "if [ -d .git ] && command -v git >/dev/null 2>&1; then \
        git describe --tags --match 'v[0-9]*' 2>/dev/null | sed -e 's/^v//' -e 's/-/-git-/'; \
    elif [ -d .git ]; then \
        sed 's/$/-git/' < VERSION; \
    else \
        cat VERSION; \
    fi"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE MUSL_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/src/internal/version.h "#define VERSION \"${MUSL_VERSION}\"\n")

#crt objects

# Create crt objects with dependencies
add_library(crt1 OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/crt/crt1.c")
add_library(Scrt1 OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/crt/Scrt1.c")
add_library(rcrt1 OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/ldso/dlstart.c")
add_library(dlstart OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/ldso/dlstart.c")
add_library(dynlink OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/ldso/dynlink.c")

# Add dependencies for the header files
set(CRT_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/internal/dynlink.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/reloc.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/crt_arch.h"
)

target_sources(dynlink PRIVATE ${CRT_HEADERS})

# Set dependencies
add_dependencies(rcrt1 generate_headers)
add_dependencies(dlstart generate_headers)
add_dependencies(dynlink generate_headers)

# Set position independent code for specific targets
set_target_properties(Scrt1 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(rcrt1 PROPERTIES POSITION_INDEPENDENT_CODE ON)


# Install rules
install(TARGETS musl_static DESTINATION lib)
install(TARGETS musl_shared DESTINATION lib)
install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h")

foreach(ASM_SOURCE ${ARCH_ASM_SRCS})
    set_source_files_properties(${ASM_SOURCE} PROPERTIES COMPILE_FLAGS "-Wno-unused-command-line-argument")
endforeach()
