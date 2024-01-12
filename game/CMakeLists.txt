cmake_minimum_required(VERSION 3.20.4)
project(boomberman)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

find_package(Protobuf REQUIRED)
find_package(Boost 1.70.0 REQUIRED log_setup log)
find_package(raylib)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 4.2.0
    )
    FetchContent_MakeAvailable(raylib)
endif ()

set(SOURCES
        src/Main.cpp

        src/shared/msg/Connection.cpp
        src/shared/msg/Connection.h

        src/server/Room.cpp
        src/server/Room.h
        src/server/Server.cpp
        src/server/Server.h
        src/client/Client.cpp
        src/shared/Util.cpp
        src/shared/Util.h
        src/shared/proto/messages.pb.cc

        src/server/entities/Player.cpp
        src/server/entities/Player.h
        src/server/entities/Primitives.h
        src/shared/game/Map.cpp
        src/shared/game/Map.h
        src/client/Boomberman.cpp
        src/client/Boomberman.h
        src/shared/game/Bomb.cpp
        src/shared/game/Bomb.h
        src/client/EntityHandler.cpp
        src/client/EntityHandler.h
        src/shared/game/TileOnFire.cpp
        src/shared/game/TileOnFire.h
        src/shared/proto/messages.pb.h
        src/client/ServerHandler.cpp
        src/client/ServerHandler.h
        src/client/RoomBox.cpp
        src/client/RoomBox.h
)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS messages.proto)
add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-g -Og")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib protobuf Boost::log_setup Boost::log)
target_link_options(${PROJECT_NAME} PUBLIC "-Wl,--copy-dt-needed-entries")
