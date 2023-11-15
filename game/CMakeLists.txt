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

set(SOURCES
	src/Main.cpp

	src/shared/Channel.cpp
	src/shared/Channel.h
	src/shared/messages.pb.cc
	src/shared/messages.pb.h

	src/server/ClientHandler.cpp
	src/server/ClientHandler.h
	src/server/Room.cpp
	src/server/Room.h
	src/server/Server.cpp
	src/server/Server.h
        src/client/Client.cpp
		src/shared/Builder.cpp
		src/shared/Builder.h
)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS messages.proto)
add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib protobuf)
