cmake_minimum_required(VERSION 3.16)

# -------------------------
# Project
# -------------------------
project(ScheduleManagement LANGUAGES CXX C)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# -------------------------
# Qt6
# -------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# -------------------------
# Sources & Headers
# -------------------------
set(SOURCES
    main.cpp
    gui.cpp
    Work.cpp
    employee.cpp
)

set(HEADERS
    gui.h
    Work.h
    Employee.h
)

# -------------------------
# Executable
# -------------------------
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# -------------------------
# Manually link libharu (vcpkg)
# -------------------------
# Set the path to your vcpkg installation and triplet
set(VCPKG_ROOT "C:/Users/meira/vcpkg")
set(VCPKG_TRIPLET "x64-mingw-dynamic")

# Include and library directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/include"
)

target_link_directories(${PROJECT_NAME} PRIVATE
    "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib"
)

# Link the libharu library (name of library file is hpdf)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    hpdf
)

# -------------------------
# Copy Qt DLLs on Windows
# -------------------------
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()
