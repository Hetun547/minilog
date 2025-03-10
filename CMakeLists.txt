cmake_minimum_required(VERSION 3.0)
project(minilog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

FILE(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
FILE(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)

add_executable(${PROJECT_NAME}
    ${header_files}
    ${src_files}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

