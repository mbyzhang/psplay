cmake_minimum_required(VERSION 3.18)
project(PSPlay)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(EVENT REQUIRED libevent)

find_library(CORRECT_LIBRARIES NAMES correct REQUIRED)
find_path(CORRECT_INCLUDE_DIRS NAMES correct.h REQUIRED)

add_executable(PSPlay psplay.c fsk.c cpu_spinner.c simple_tone_gen.c x8b10b.c framer.c)
target_link_directories(PSPlay PUBLIC ${EVENT_LIBRARY_DIRS})
target_include_directories(PSPlay PUBLIC ${EVENT_INCLUDE_DIRS} ${CORRECT_INCLUDE_DIRS})
target_link_libraries(PSPlay PRIVATE Threads::Threads ${EVENT_LIBRARIES} m ${CORRECT_LIBRARIES})
