cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_NAME proxy)
project(${PROJECT_NAME} LANGUAGES CXX)

file(GLOB SOURCE_FILES ${CMAKE_SOURCE_DIR}/source/*.cpp)
#file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/header/*.hpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} pthread)
