enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Core Test)

set(TEST_SOURCES
    Node_test.cpp
    Document_test.cpp
    Command_test.cpp
    Serializer_test.cpp
)

add_executable(poliable_model_test ${TEST_SOURCES})

target_include_directories(poliable_model_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(poliable_model_test PRIVATE
    Qt6::Core
    Qt6::Test
    poliable_model
)

add_test(NAME poliable.model_tests COMMAND poliable_model_test)
