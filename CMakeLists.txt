cmake_minimum_required(VERSION 3.24)
project(boomberman)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
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

set(SOURCES src/Main.cpp src/server/Server.h src/server/Server.cpp src/client/Client.cpp src/client/Client.h src/server/Room.cpp src/server/Room.h src/shared/Protocol.cpp src/shared/Protocol.h src/shared/Message.h src/shared/messages/server/GameJoin.h src/server/ClientHandler.cpp src/server/ClientHandler.h src/shared/messages/client/IMove.h src/shared/messages/client/IPlaceBomb.h src/shared/messages/client/ILeave.h src/shared/messages/server/GotHit.h src/shared/messages/server/GameWait.h src/shared/messages/server/OtherBombPlace.h src/shared/messages/server/OtherMove.h src/shared/messages/server/OtherLeave.h src/shared/messages/server/GameWon.h
        src/shared/messages.pb.cc)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS messages.proto)
add_executable(${PROJECT_NAME} ${SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib protobuf)
