project(zephyrrtos-mcpp
    LANGUAGES CXX
)
add_library(zephyrrtos-mcpp INTERFACE)
target_include_directories(zephyrrtos-mcpp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
