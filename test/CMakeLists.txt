
add_executable(runtest 
  runtest.cpp 
  test_cell.cpp 
  test_row.cpp 
  test_reader.cpp )

set_property(TARGET runtest PROPERTY CXX_STANDARD 11)
set_property(TARGET runtest PROPERTY CXX_STANDARD_REQUIRED ON)
