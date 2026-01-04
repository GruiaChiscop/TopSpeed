cmake_minimum_required(VERSION 3.20)
project(TopSpeed VERSION 3.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Options
option(TOPSPEED_BUILD_SERVER "Build the race server" ON)
option(TOPSPEED_BUILD_CLIENT "Build the client game" ON)
option(TOPSPEED_ENABLE_NETWORKING "Enable network play" ON)
option(TOPSPEED_ENABLE_SCREEN_READER "Enable screen reader support" ON)

# Find required packages
find_package(SDL3 REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Ogg REQUIRED)

# Download and include miniaudio (header-only)
include(FetchContent)
FetchContent_Declare(
    miniaudio
    URL https://github.com/mackron/miniaudio/releases/download/0.11.18/miniaudio.h
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(miniaudio)

# Create miniaudio include target
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${miniaudio_SOURCE_DIR})

# Common library
add_library(TopSpeedCommon STATIC
    vs_projects/common/src/common.cpp
)
target_include_directories(TopSpeedCommon PUBLIC
    vs_projects/common/include
)

# DX Common library (wrapper for DirectX functionality)
add_library(DXCommon STATIC
    vs_projects/dxcommon/src/dxcommon.cpp
    src/platform/SDL3Wrapper.cpp
)
target_include_directories(DXCommon PUBLIC
    vs_projects/dxcommon/include
    src/platform
    ${SDL3_INCLUDE_DIRS}
)
target_link_libraries(DXCommon PUBLIC
    TopSpeedCommon
    SDL3::SDL3
)

# Audio library (miniaudio + Vorbis)
add_library(TopSpeedAudio STATIC
    src/audio/AudioEngine.cpp
    src/audio/VorbisDecoder.cpp
)
target_include_directories(TopSpeedAudio PUBLIC
    src/audio
    ${VORBIS_INCLUDE_DIR}
    ${OGG_INCLUDE_DIR}
)
target_link_libraries(TopSpeedAudio PUBLIC
    miniaudio
    Vorbis::Vorbis
    Vorbis::VorbisFile
    Ogg::Ogg
)

# Screen Reader library
add_library(TopSpeedScreenReader STATIC
    src/accessibility/ScreenReaderManager.cpp
)
target_include_directories(TopSpeedScreenReader PUBLIC
    src/accessibility
)
if(WIN32)
    target_link_libraries(TopSpeedScreenReader PUBLIC
        ole32
        oleaut32
    )
endif()

# Networking library (UDP/TCP)
add_library(TopSpeedNetworking STATIC
    src/network/NetworkManager.cpp
    src/network/UDPSocket.cpp
    src/network/TCPSocket.cpp
)
target_include_directories(TopSpeedNetworking PUBLIC
    src/network
)
if(WIN32)
    target_link_libraries(TopSpeedNetworking PUBLIC
        ws2_32
        mswsock
        advapi32
    )
else()
    target_link_libraries(TopSpeedNetworking PUBLIC
        pthread
    )
endif()

# Main game client
if(TOPSPEED_BUILD_CLIENT)
    add_executable(TopSpeed
        vs_projects/topspeed/TopSpeed.cpp
        vs_projects/topspeed/TopSpeedDlg.cpp
        vs_projects/topspeed/Game.cpp
        vs_projects/topspeed/Menu.cpp
        vs_projects/topspeed/Car.cpp
        vs_projects/topspeed/Track.cpp
        vs_projects/topspeed/Level.cpp
        vs_projects/topspeed/LevelSingleRace.cpp
        vs_projects/topspeed/LevelTimeTrial.cpp
        vs_projects/topspeed/LevelMultiplayer.cpp
        vs_projects/topspeed/ComputerPlayer.cpp
        vs_projects/topspeed/RaceInput.cpp
        vs_projects/topspeed/RaceClient.cpp
        vs_projects/topspeed/RaceSettings.cpp
    )
    
    target_include_directories(TopSpeed PRIVATE
        vs_projects/topspeed
        src
        ${SDL3_INCLUDE_DIRS}
    )
    
    target_link_libraries(TopSpeed PRIVATE
        TopSpeedCommon
        DXCommon
        TopSpeedAudio
        TopSpeedScreenReader
        TopSpeedNetworking
        SDL3::SDL3
    )
    
    if(MSVC)
        set_target_properties(TopSpeed PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()
endif()

# Race server
if(TOPSPEED_BUILD_SERVER)
    add_executable(TopSpeedServer
        vs_projects/topspeed/RaceServer.cpp
        vs_projects/topspeed/RaceSettings.cpp
        src/server/ServerMain.cpp
    )
    
    target_include_directories(TopSpeedServer PRIVATE
        vs_projects/topspeed
        src
    )
    
    target_link_libraries(TopSpeedServer PRIVATE
        TopSpeedCommon
        TopSpeedNetworking
    )
endif()

# Testing
enable_testing()
# Add tests subdirectory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

# Print build info
message(STATUS "TopSpeed ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Client: ${TOPSPEED_BUILD_CLIENT}")
message(STATUS "Build Server: ${TOPSPEED_BUILD_SERVER}")
message(STATUS "Enable Networking: ${TOPSPEED_ENABLE_NETWORKING}")
message(STATUS "Enable Screen Reader: ${TOPSPEED_ENABLE_SCREEN_READER}")
