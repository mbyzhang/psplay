cmake_minimum_required(VERSION 3.10)
project(PSPlay)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(EVENT REQUIRED libevent)

add_library(CPUSpinner cpu_spinner.c)
target_link_libraries(CPUSpinner PRIVATE Threads::Threads)

add_library(FSKModulator fsk.c)
target_link_libraries(FSKModulator PRIVATE ${EVENT_LIBRARIES} m)
target_link_directories(FSKModulator PUBLIC ${EVENT_LIBRARY_DIRS})
target_include_directories(FSKModulator PUBLIC ${EVENT_INCLUDE_DIRS})

add_executable(PSPlay psplay.c)
target_link_libraries(PSPlay PRIVATE Threads::Threads CPUSpinner FSKModulator)
