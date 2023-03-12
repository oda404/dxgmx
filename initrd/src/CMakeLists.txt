cmake_minimum_required(VERSION 3.23)
project(main)

add_executable(${CMAKE_PROJECT_NAME} main.c)

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/../bin/)
