cmake_minimum_required(VERSION 2.8)

project(block_matching)

set(CMAKE_CXX_FLAGS "-std=c++14 -I../src")

# Find libraryies
find_library(CPP_REDIS cpp_redis)
find_library(TACOPIE tacopie)
find_package(Threads REQUIRED)

# Add cpp_redis
set(LIBRARIES ${CPP_REDIS} ${TACOPIE})

# Look for source files and headers
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE INCLUDE_FILES "src/*.hpp" "src/*.h")

# Configure executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${INCLUDE_FILES})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)

# Link the executable to the libraries.
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# Thread selection logic
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(${PROJECT_NAME} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(${PROJECT_NAME} "${CMAKE_THREAD_LIBS_INIT}")
endif()