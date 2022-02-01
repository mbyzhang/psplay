cmake_minimum_required(VERSION 3.18)
project(PSPlay)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(EVENT REQUIRED libevent)

find_library(CORRECT_LIBRARIES NAMES correct REQUIRED)
find_path(CORRECT_INCLUDE_DIRS NAMES correct.h REQUIRED)

add_library(PSPlayLib fsk.c cpu_spinner.c simple_tone_gen.c x8b10b.c framer.c bitstream.c)
target_include_directories(PSPlayLib PUBLIC ${EVENT_INCLUDE_DIRS} ${CORRECT_INCLUDE_DIRS})
target_link_directories(PSPlayLib PUBLIC ${EVENT_LIBRARY_DIRS})
target_link_libraries(PSPlayLib Threads::Threads ${EVENT_LIBRARIES} m ${CORRECT_LIBRARIES})

add_executable(PSPlay psplay.c)
target_link_libraries(PSPlay PSPlayLib)

enable_testing()
add_executable(PSPlayTests tests/bitstream.cc)
target_link_libraries(PSPlayTests gtest_main)
target_link_libraries(PSPlayTests PSPlayLib)
include(GoogleTest)
gtest_discover_tests(PSPlayTests)
