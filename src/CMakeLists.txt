set(NES_SOURCES Bus.cpp nes6502.cpp)

add_library(nes6502 ${NES_SOURCES})
target_link_libraries(nes6502
    PUBLIC  project_options
    PRIVATE project_warnings)
target_include_directories(nes6502 PUBLIC ${CMAKE_SOURCE_DIR}/include)

set(DEMO_SOURCES Demo6502.cpp)
add_executable(demo ${DEMO_SOURCES})
target_link_libraries(demo PRIVATE nes6502 olc_pge)
