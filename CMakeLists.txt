cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11) # Requires C11 standard

project(MyBrowser VERSION 0.1 LANGUAGES CXX)

find_package(raylib 3.0 REQUIRED) # Requires at least version 3.0


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} src/main.cpp src/url.cpp src/parser.cpp src/layout.cpp src/render.cpp)

target_link_libraries(${PROJECT_NAME} raylib curl)

# Replace "your_browser_name" with the actual name of your target
target_compile_options(${PROJECT_NAME} PRIVATE 
    -g 
    -fno-omit-frame-pointer
    -fsanitize=address
)

target_link_options(${PROJECT_NAME} PRIVATE 
    -fsanitize=address
)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
