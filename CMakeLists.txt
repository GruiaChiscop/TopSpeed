cmake_minimum_required(VERSION 3.20)
project(TopSpeed VERSION 3.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

# Options
option(TOPSPEED_BUILD_SERVER "Build the race server" ON)
option(TOPSPEED_BUILD_CLIENT "Build the client game" ON)
option(TOPSPEED_ENABLE_NETWORKING "Enable network play" ON)
option(TOPSPEED_ENABLE_SCREEN_READER "Enable screen reader support" ON)

# Find required packages
find_package(SDL3 REQUIRED)

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
    vs_projects/dxcommon/src/SDL3Wrapper.cpp
)
target_include_directories(DXCommon PUBLIC
    vs_projects/dxcommon/include
    ${SDL3_INCLUDE_DIRS}
)
target_link_libraries(DXCommon PUBLIC
    TopSpeedCommon
    SDL3::SDL3
)

# Audio library (miniaudio)
add_library(TopSpeedAudio STATIC
    src/audio/AudioEngine.cpp
    src/audio/VorbisDecoder.cpp
)
target_include_directories(TopSpeedAudio PUBLIC
    src/audio
)
target_link_libraries(TopSpeedAudio PUBLIC
    ogg
    vorbis
)

# Screen Reader library
add_library(TopSpeedScreenReader STATIC
    src/accessibility/ScreenReaderManager.cpp
)
target_include_directories(TopSpeedScreenReader PUBLIC
    src/accessibility
)

# Networking library
add_library(TopSpeedNetworking STATIC
    src/network/NetworkManager.cpp
    src/network/UDPSocket.cpp
    src/network/TCPSocket.cpp
)
target_include_directories(TopSpeedNetworking PUBLIC
    src/network
)

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
    )
    
    target_link_libraries(TopSpeedServer PRIVATE
        TopSpeedCommon
        TopSpeedNetworking
    )
endif()

# Testing
enable_testing()
add_subdirectory(tests)
