cmake_minimum_required(VERSION 3.21)

project(PoliAble
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set Qt paths - prioritize E:/Qt which has the real installation
set(QT6_ROOT "E:/Qt/6.12.0/msvc2022_64" CACHE PATH "Qt6 installation root")
set(CMAKE_PREFIX_PATH "${QT6_ROOT}" ${CMAKE_PREFIX_PATH})

# Try to find Qt6 for UI support
set(WITH_QT OFF)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Verify Qt6 is usable (has real headers, not just stubs)
# Qt6_ROOT is set by user in cache; use it to check for real headers
set(QT6_REAL_INCLUDE_DIR "${QT6_ROOT}/include/QtWidgets")
if(EXISTS "${QT6_REAL_INCLUDE_DIR}/QVBoxLayout")
    set(WITH_QT ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    message(STATUS "✅ Qt6 FOUND at: ${Qt6_DIR} - Building WITH Qt6 UI")
else()
    message(STATUS "⚠️  Qt6 stub detected (missing QVBoxLayout header) - Building HEADLESS (core only)")
    set(WITH_QT OFF)
endif()

if(NOT WITH_QT)
    message(STATUS "✅ Building HEADLESS (core only) - no real Qt6 installation found")
endif()

# Core library (no Qt dependency)
add_library(poliable_core STATIC
    src/model/Node.cpp
    src/model/Document.cpp
    src/model/Command.cpp
    src/model/UndoStack.cpp
    src/model/Serializer.cpp
    src/model/Styling.cpp
    src/layout/LayoutEngine.cpp
)
target_include_directories(poliable_core PUBLIC src)
target_compile_features(poliable_core PRIVATE cxx_std_20)

# Main executable
if(WITH_QT)
    add_executable(PoliAble WIN32
        src/main.cpp
        src/MainWindow.cpp
        src/panels/LayersPanel.cpp
        src/panels/AssetsPanel.cpp
        src/panels/PropertiesPanel.cpp
        src/panels/TimelinePanel.cpp
        src/panels/ConsolePanel.cpp
        src/theme/Theme.cpp
        src/canvas/CanvasView.cpp
        src/canvas/CanvasItem.cpp
    )
    target_include_directories(PoliAble PRIVATE src)
    target_link_libraries(PoliAble PRIVATE poliable_core Qt6::Core Qt6::Gui Qt6::Widgets)
else()
    message(STATUS "Building HEADLESS (core only)")
    add_executable(PoliAble
        src/main_headless.cpp
    )
    target_include_directories(PoliAble PRIVATE src)
    target_link_libraries(PoliAble PRIVATE poliable_core)
endif()

# Tests
add_executable(poliable_tests
    test/model/Node_test.cpp
)
target_include_directories(poliable_tests PRIVATE src)
target_link_libraries(poliable_tests PRIVATE poliable_core)
enable_testing()
add_test(NAME poliable_tests COMMAND poliable_tests)

message(STATUS "")
message(STATUS "✅ PoliAble Configuration Complete:")
message(STATUS "   Core Engine: READY")
if(WITH_QT)
    message(STATUS "   Qt6 Detected: YES - Building WITH Qt6 UI")
else()
    message(STATUS "   Qt6 Detected: NO  - Building HEADLESS")
endif()
message(STATUS "")
