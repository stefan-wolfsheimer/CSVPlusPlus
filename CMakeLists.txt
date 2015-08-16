cmake_minimum_required(VERSION 3.3)
project(CSV++)


include_directories(BEFORE .)
add_subdirectory(test)
add_subdirectory(examples)

install(DIRECTORY csv DESTINATION include/csv )
