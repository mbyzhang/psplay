cmake_minimum_required(VERSION 3.18)
project(PSPlay)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(EVENT REQUIRED libevent)

find_library(CORRECT_LIBRARIES NAMES correct REQUIRED)
find_path(CORRECT_INCLUDE_DIRS NAMES correct.h REQUIRED)

add_library(PSPlayLib fsk.c cpu_spinner.c simple_tone_gen.c x8b10b.c framer.c bitstream.c)
target_include_directories(PSPlayLib PUBLIC ${EVENT_INCLUDE_DIRS} ${CORRECT_INCLUDE_DIRS})
target_link_directories(PSPlayLib PUBLIC ${EVENT_LIBRARY_DIRS})
target_link_libraries(PSPlayLib Threads::Threads ${EVENT_LIBRARIES} m ${CORRECT_LIBRARIES})
target_compile_options(PSPlayLib PUBLIC -march=native)

if (DEFINED CPU_SPINNER_KERNEL)
target_compile_options(PSPlayLib PUBLIC -DCPU_SPINNER_KERNEL=${CPU_SPINNER_KERNEL})
endif()

add_executable(PSPlay psplay.c)
target_link_libraries(PSPlay PSPlayLib)

enable_testing()
add_executable(PSPlayTests tests/bitstream.cc)

target_include_directories(PSPlayTests PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(PSPlayTests ${GTEST_MAIN_LIBRARIES})
target_link_libraries(PSPlayTests PSPlayLib)
gtest_discover_tests(PSPlayTests)
