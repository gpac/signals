cmake_minimum_required(VERSION 2.8.12)
project(async)
enable_testing()

# C++11 support (a better portable way will be available in the next version of CMake)
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
endif()

# Thread support
find_package(Threads)
if( NOT Threads_FOUND )
  message( FATAL "Could NOT find the thread library" )
endif()


# Headers
include_directories("signals")


# Build executables
macro(ADD_ASYNC_EXECUTABLE NAME)
  add_executable( ${NAME} ${ARGN} )
  target_link_libraries( ${NAME} ${CMAKE_THREAD_LIBS_INIT} )
endmacro(ADD_ASYNC_EXECUTABLE)

ADD_ASYNC_EXECUTABLE(signals        tests/signals.cpp)


# Build unit tests
macro(ADD_ASYNC_UNIT_TEST NAME)
  add_executable( ${NAME} ${ARGN} )
  target_compile_definitions( ${NAME} PRIVATE -DUNIT )
  target_link_libraries( ${NAME} ${CMAKE_THREAD_LIBS_INIT} )
endmacro(ADD_ASYNC_UNIT_TEST)

ADD_ASYNC_UNIT_TEST(signals_unit_result tests/signals_unit_result.cpp)
ADD_ASYNC_UNIT_TEST(signals_simple      tests/signals_simple.cpp)
ADD_ASYNC_UNIT_TEST(signals_perf        tests/signals_perf.cpp)
ADD_ASYNC_UNIT_TEST(signals_module      tests/signals_module.cpp)
ADD_ASYNC_UNIT_TEST(signals_async       tests/signals_async.cpp)


# Run unit tests
add_test(Exec_signals                  signals)
add_test(Unit_test_signals_unit_result signals_unit_result)
add_test(Unit_test_signals_simple      signals_simple)
add_test(Unit_test_signals_perf        signals_perf)
add_test(Unit_test_signals_module      signals_module)
add_test(Unit_test_signals_async       signals_async)


