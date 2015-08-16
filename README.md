C++11 CSV Parser library
========================

CSV++ is a simple library for parsing CSV formatted values.

Features
--------

- Support of custom separators
- Quoted and multiline cells  
- Input iterator over rows 
- Random access iterator for columns
- Access columns by name
- Extract header information from first input line
- Conversion of cell content to C++ types 

Requirements & Installation
---------------------------

This is a header only library. Just include the subdirectory "csv" in your 
project.

- C++11 compiler (tested so far with gcc version 4.8.3)
- (optional) unix make or cmake to build examples and unit tests

Credits
-------
 
- [philsquared/Catch forwork for unit-tests](https://github.com/philsquared/Catch), see LICENSE_3RD_PARTY


Usage at a glance
-----------------

### Iteration over row and cells
```c++
  #include "csv/reader.h"

  void readCsv(std::istream & ist) 
  {
    csv::Reader reader(ist);
    for(auto row : reader) 
    {
      for(auto cell : row) 
      {
        std::cout << cell.row() << ":" << cell.column();
        std::cout << "=" << cell.as<std::string>() << std::endl;
      }
    }
  }
```

### Iteration over row accessing cells by name
```c++
  #include "csv/reader.h"

  void readCsv(std::istream & ist) 
  {
    csv::Reader reader(ist,csv::Specification().withHeader());
    for(auto row : reader) 
    {
      std::cout << row["id"].as<int>() << "->" 
                << row["name"].as<std::string>();
    }
  }
```



### Specification

TODO: more details here

| property       | construction                                                    | test                               |
|----------------|-----------------------------------------------------------------|------------------------------------|
|Separator       | `withSeparator(char_type ch)`, `withoutSeparator(char_type ch)` | `bool isSeparator(char_type)`      |
|Locale          | `withLocale(std::locale l)`                                     | `std::locale locale()`             |
|Header          | `withHeader()`, `withoutHeader()`                               | `bool hasHeader()`                 |
|Column          | `withColumn(size_t, string_type)`                               |                                    |
|Comment         | `withComment(char_type ch)`, `withoutComment()`                 | `bool isComment(char_type)`        |
|UsingEmptyLines | `withUsingEmptyLines()`, `withoutUsingEmptyLines()`             | `bool hasUsingEmptyLines()`        |



### Exceptions

The library throws the following type of exceptions:

- `ParseError`: when the input stream cannot be parsed 
  (e.g. not a valid CSV format)
- `DuplicateColumnError`: when attempting to define a duplicate column name,
  e.g. reading the CSV file (using the WithHeader option):
  `column_name1,column_name_2,column_name1`
- `CellOutOfRangeError`: when accessing a column of a row 
   by index which is out of range.
- `UndefinedColumnError`: when accessing a column of a row 
   by name which is not defined. 
- `DefinedCellOutOfRangeError`: when accessing a column of a row
   by name. The name is defined and assigned to an index which is 
   out of range for the row.
- `ConversionException`: when a cell value cannot be converted into 
   a C++ type (`cell->as<TYPE>()`).

All Exceptions are derived from `CsvException` which is derived from 
`std::exception`.

*Methods:*

| return        | method            | implemented in        | description                           |
|---------------|-------------------|-----------------------|---------------------------------------|
| `const char*` | `what()`          | all `CsvException`    | Description of the error.             |
| `size_t`      | `inputLine()`     | all `CsvException`    | Line number of the raw input stream   |
| `size_t`      | `inputColumn()`   | all `CsvException`    | Column number of the raw input stream |
| `size_t`      | `row()`           | all `CsvException`    | Row index of the CSV table            |
| `size_t`      | `column()`        | all `CsvException`    | Column index of the CSV table         |
| `size_t`      | `index()`         | `CellOutOfRangeError` `DefinedCellOutOfRangeError`, `DuplicateColumnError`| The index of a column.                |
| `size_t`      | `size()`          | `CellOutOfRangeError` `UndefinedColumnError` `DefinedCellOutOfRangeError` | The number of columns of the row.     |
| `::std::type_index` | `typeIndex()` | `ConversionError`  | The target type when attempting to convert a cell to a C++ type |

### The Row class 

TODO more details 

### The Cell class 

TODO more details


Implementation Details
----------------------
### Class structure

### Parser's state machine 
![State machine](https://github.com/stefan-wolfsheimer/CSVPlusPlus/tree/master/doc/statediagram.png "State machine")


