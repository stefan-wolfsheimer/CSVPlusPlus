include_directories (${PROJECT_SOURCE_DIR}/Catch2/single_include)
add_library(Catch INTERFACE)

add_executable(runtest 
  runtest.cpp 
  test_cell.cpp 
  test_row.cpp 
  test_reader.cpp
  test_builder.cpp )

target_link_libraries(runtest Catch)
set_property(TARGET runtest PROPERTY CXX_STANDARD 11)
set_property(TARGET runtest PROPERTY CXX_STANDARD_REQUIRED ON)
