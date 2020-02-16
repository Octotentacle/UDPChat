cmake_minimum_required(VERSION 3.15)
project(client)
set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ../build)
find_package(Boost 1.54 COMPONENTS system filesystem thread program_options REQUIRED)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(client client.cpp chat_message.h)
add_executable(server server.cpp chat_message.h)
target_link_libraries(client ${Boost_LIBRARIES} pthread)
target_link_libraries(server ${Boost_LIBRARIES} pthread)