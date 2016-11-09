cmake_minimum_required(VERSION 2.6)
SET(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -march=native -O3")

include_directories(src)
set(SLIP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/parcxx/src/combinators.h
    ${CMAKE_SOURCE_DIR}/src/parcxx/src/parcxx.h
    ${CMAKE_SOURCE_DIR}/src/parcxx/src/optional.h
    ${CMAKE_SOURCE_DIR}/src/slip.h
    ${CMAKE_SOURCE_DIR}/src/impl/typecheck.h
    ${CMAKE_SOURCE_DIR}/src/impl/context.h
    ${CMAKE_SOURCE_DIR}/src/impl/function.h
    ${CMAKE_SOURCE_DIR}/src/impl/eval.h
    ${CMAKE_SOURCE_DIR}/src/impl/print.h
    ${CMAKE_SOURCE_DIR}/src/impl/ast.h
    ${CMAKE_SOURCE_DIR}/src/impl/mangler.h
    ${CMAKE_SOURCE_DIR}/src/impl/parser.h
    ${CMAKE_SOURCE_DIR}/src/impl/function-impl.h)



add_subdirectory(tests)
