cmake_minimum_required(VERSION 3.13)
project(cnes LANGUAGES C)

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.x
)
FetchContent_MakeAvailable(SDL3)

file(GLOB MAPPERS "${CMAKE_CURRENT_SOURCE_DIR}/src/mapper/*.c")

add_executable(cnes
    src/apu.c
    src/cart.c
    src/cpu.c
    src/gui.c
    src/input.c
    src/main.c
    src/nes.c
    src/ppu.c
    ${MAPPERS}
)

set_property(TARGET cnes PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set(CMAKE_OPTIMIZE_DEPENDENCIES_RELEASE TRUE)

target_compile_options(cnes PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_options(cnes PRIVATE
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    $<$<CONFIG:Debug>:-fno-optimize-sibling-calls>
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_link_options(cnes PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_link_libraries(cnes PRIVATE SDL3::SDL3 m)
