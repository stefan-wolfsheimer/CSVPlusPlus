
add_executable( 01_basic_iteration 01_basic_iteration.cpp )
add_executable( 02_parse_into_object 02_parse_into_object.cpp )
add_executable( 03_specifications  03_specifications.cpp )

set_property(TARGET 01_basic_iteration PROPERTY CXX_STANDARD 11)
set_property(TARGET 01_basic_iteration PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET 02_parse_into_object PROPERTY CXX_STANDARD 11)
set_property(TARGET 02_parse_into_object PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET 03_specifications PROPERTY CXX_STANDARD 11)
set_property(TARGET 03_specifications PROPERTY CXX_STANDARD_REQUIRED ON)
