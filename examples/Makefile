SRC=csv_reader.cpp 
INCLUDE=-I ../
CXX=g++ 
CXXFLAGS=-Wall -std=c++11
DEP= ../csv/csv_common.h \
	   ../csv/specification.h \
	   ../csv/cell.h \
	   ../csv/row.h \
	   ../csv/reader.h

all: ${DEP} 01_basic_iteration.cpp 02_parse_into_object.cpp
	${CXX} ${CXXFLAGS} ${INCLUDE} 01_basic_iteration.cpp -o 01_basic_iteration
	${CXX} ${CXXFLAGS} ${INCLUDE} 02_parse_into_object.cpp -o 02_parse_into_object

