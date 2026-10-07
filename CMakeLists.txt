cmake_minimum_required(VERSION 3.28)
project(PoliAble VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt installation path
set(CMAKE_PREFIX_PATH "E:/JUNK/6.11.2/msvc2022_64")

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Create executable
add_executable(PoliAble
    src/main.cpp
)

# Link Qt libraries
target_link_libraries(PoliAble PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# Set Windows subsystem
if(WIN32)
    set_target_properties(PoliAble PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Install target
install(TARGETS PoliAble
    RUNTIME DESTINATION bin
)
