cmake_minimum_required(VERSION 3.15)
project(UcPP)

option(USE_STATIC_LINKING "Use static linking for the runtime" ON)

if (USE_STATIC_LINKING)
    if (MSVC)
        # MSVC specific static linking flag
        add_compile_options(/MT$<$<CONFIG:Debug>:d>)
    else()
        # GCC/Clang specific static linking flag
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

find_package(CBuildKit REQUIRED)
include_directories(include)
add_subdirectory(src)
add_subdirectory(vendor)
add_subdirectory(playground)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include)

install_cfgpkg(ucpp 
"
include(CMakeFindDependencyMacro)
find_dependency(CBuildKit)
find_dependency(simplistic-io)

add_library_ns(unicorn unicorn INTERFACE)
file(GLOB UNICORN_LIBS \${CMAKE_CURRENT_LIST_DIR}/*.lib)
target_link_libraries(unicorn-unicorn INTERFACE \${UNICORN_LIBS})
include(\${CMAKE_CURRENT_LIST_DIR}/ucpp-ucpp-targets.cmake)
")