# TopSpeed Test Suite CMake Configuration

cmake_minimum_required(VERSION 3.16)

# Test executable configurations
set(TEST_SOURCES
    test_audio_effects.cpp
    test_sound_mixer.cpp
    test_input_manager.cpp
    test_game_engine.cpp
)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Find required packages
find_package(SDL3 REQUIRED)
find_package(miniaudio REQUIRED)

# Create test executables
foreach(TEST_SOURCE ${TEST_SOURCES})
    # Get test name from filename
    string(REPLACE ".cpp" "" TEST_NAME ${TEST_SOURCE})
    
    # Create executable
    add_executable(${TEST_NAME} ${TEST_SOURCE}
        ../src/audio/Effects.cpp
        ../src/audio/SoundMixer.cpp
        ../src/input/InputManager.cpp
        ../src/engine/GameClock.cpp
        ../src/engine/GameState.cpp
        ../src/engine/GameEngine.cpp
        ../src/audio/AudioEngine.cpp
        ../src/network/NetworkManager.cpp
        ../src/accessibility/AccessibilityManager.cpp
    )
    
    # Link libraries
    target_link_libraries(${TEST_NAME}
        SDL3::SDL3
        miniaudio
        pthread
    )
    
    # Set C++ standard
    set_target_properties(${TEST_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    # Add to CTest
    add_test(
        NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # Add verbose output
    set_tests_properties(${TEST_NAME} PROPERTIES
        FAIL_REGULAR_EXPRESSION "SOME TESTS FAILED"
        PASS_REGULAR_EXPRESSION "ALL TESTS PASSED"
    )
endforeach()

# Create a master test target
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -V
    DEPENDS ${TEST_EXECUTABLES}
    COMMENT "Running all tests..."
)

# Print test configuration
message(STATUS "Tests configured:")
foreach(TEST_SOURCE ${TEST_SOURCES})
    string(REPLACE ".cpp" "" TEST_NAME ${TEST_SOURCE})
    message(STATUS "  - ${TEST_NAME}")
endforeach()
