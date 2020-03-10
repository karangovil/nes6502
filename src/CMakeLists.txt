set(NES_SOURCES Bus.cpp
                nes6502.cpp
                nes2C02.cpp
                Cartridge.cpp
                Mapper.cpp
                Mapper_000.cpp
                )

add_library(nes ${NES_SOURCES})
target_link_libraries(nes
    PUBLIC  project_options
    PUBLIC  olc_pge
    # PRIVATE project_warnings
    )
target_include_directories(nes PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(demo6502 Demo6502.cpp)
target_link_libraries(demo6502 PRIVATE nes)

add_executable(demo2C02 Demo2C02.cpp)
target_link_libraries(demo2C02 PRIVATE nes)
