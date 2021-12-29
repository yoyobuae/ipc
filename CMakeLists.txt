cmake_minimum_required (VERSION 3.6)

project (ipc CXX C)

set(VERSION "0.1")

list(APPEND ipc_HEADERS
    include/Ipc.hpp
    )

list(APPEND ipc_SOURCE
    src/Ipc.cpp
    )

if (CMAKE_BUILD_TYPE EQUAL "Debug")
    add_definitions (
        -g -O0 -fPIC
        )
else ()
    add_definitions (
        -fPIC
        )
endif ()

add_library (ipc SHARED
    ${ipc_SOURCE}
    ${ipc_HEADERS}
    )

target_include_directories(ipc PRIVATE
    "${CMAKE_BINARY_DIR}/"
    "${CMAKE_SOURCE_DIR}/src"
    )

target_include_directories(ipc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ipc>
    )

if (CMAKE_BUILD_TYPE EQUAL "Debug")
target_compile_options(ipc
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-g>
      )
endif()

# Needed for libipc.pc.in
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(bindir "\${exec_prefix}/bin")
set(includedir "\${prefix}/include")
set(PACKAGE_VERSION "${VERSION}")

configure_file(
    "${CMAKE_SOURCE_DIR}/libipc.pc.in"
    "${CMAKE_BINARY_DIR}/libipc.pc"
    @ONLY)


install(TARGETS ipc
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
    )
install(FILES ${ipc_HEADERS} DESTINATION include/ipc)
install(FILES "${CMAKE_BINARY_DIR}/libipc.pc" DESTINATION lib/pkgconfig)

enable_testing()
add_subdirectory(tests)
