# SRC= \
# 	  test_cell.cpp \
# 	  test_reader.cpp \
# 	  test_specification.cpp \
# 	  runtest.cpp 

SRC=test_cell.cpp \
	  test_row.cpp	\
	  test_reader.cpp \
	  test_specification.cpp \
	  runtest.cpp 

CXX=g++ -O2
CXXFLAGS=-Wall -std=c++11

INCLUDE=-I../
HEADER=	../csv/csv_common.h \
	      ../csv/specification.h \
		    ../csv/cell.h \
		    ../csv/row.h \
		    ../csv/reader.h

all: ${SRC} ${HEADER} 
	${CXX} ${CXXFLAGS} ${INCLUDE} ${SRC} -o runtest

benchmark: benchmark.cpp ${HEADER}
	${CXX} ${CXXFLAGS} ${INCLUDE} benchmark.cpp -o benchmark


