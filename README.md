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

- C++11 compiler (tested with gcc 4.8.3, gcc 5.4.0)
- (optional) cmake to build examples and unit tests

Credits
-------
- [A modern, C++-native, header-only, test framework for unit-tests, TDD and BDD](https://github.com/catchorg/Catch2)


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

### Object mapping
```c++
#include <vector>
#include <csv/reader.h>
#include <csv/object_reader.h>

struct Planet
{
  int         index;
  std::string name;
  float       diameter;
  float       mass;
  float       orbital_period;
  float       rotation_period;
  Planet() : index(0), diameter(0.0f), mass(0.0f),
             orbital_period(0.0f), rotation_period(0.0f) {}
};

std::vector<Planet> readCsv(std::istream & ist)
{
  auto planetBuilder = csv::Builder<Planet>()
    .member<int>(&Planet::index, "Number", 0)
    .member<std::string>(&Planet::name, "Name", "")
    .member<float>(&Planet::diameter, "Diameter", 0.0)
    .member<float>(&Planet::mass, "Mass", 0.0)
    .member<float>(&Planet::orbital_period, "Orbital period", 0.0)
    .member<float>(&Planet::rotation_period, "Rotation period", 0.0);

  csv::ObjectReader<Planet> reader(planetBuilder,
                                   ist,
                                   csv::Specification().withHeader());
  std::vector<Planet> planets(reader.begin(), reader.end());
  return planets;
}
```


Building examples and unit tests
--------------------------------
* checkout dependent submodule:
```
git submodule update --init --recursive
```
* create working directory and build
```
CSVPlusPlus> mkdir build
CSVPlusPlus/build> cd build
CSVPlusPlus/build> cmake ../
CSVPlusPlus/build> make
```

* run the tests
```
CSVPlusPlus/build> test/runtests
```


### Specification

Instances of the Specification object define the details of the CSV dialect.
It is possible to configure the column separator, the locale (for parsing numbers), whether or not using the
first row as header, defining a character for indicating comment lines (the rest of the line is being ignored) and
whether or not empty lines should be ignored in the stream.

| property       | construction                                                    | test                               | default
|----------------|-------------------------------------------------------|------------------------------------|---------
|Separator       | `withSeparator(const string_type & seps)`             | `bool isSeparator(char_type)`      | ,
|Locale          | `withLocale(std::locale l)`                           | `std::locale locale()`             | system locale
|DecimalSeparatpr| `withDecimalSeparator(char_type ch)`                  |                                    | from seystem locale
|Header          | `withHeader()`, `withoutHeader()`                     | `bool hasHeader()`                 | false
|Column          | `withColumn(size_t, string_type)`                     |                                    | 
|Comment         | `withComment(char_type ch)`, `withoutComment()`       | `bool isComment(char_type)`        | false
|UsingEmptyLines | `withUsingEmptyLines()`, `withoutUsingEmptyLines()`   | `bool isUsingEmptyLines()`         | false

The following example (from example/04_specification.cpp) constructs a csv::Reader for which all column names are
determined from the first line except for column 4 and 5 (0 based counting). The decimal separator is configured as `,`.
Both `|` and `;` are accepted as column separators. Line content after `#` is ignored and empty lines are not considered.

```c++
  csv::Reader reader(ist,
                     csv::Specification()
                     .withHeader()
                     .withDecimalSeparator(',')
                     .withSeparator("|;")
                     .withComment('#')
                     .withoutUsingEmptyLines()
                     .withColumn(4, "Orbital period")
                     .withColumn(5, "Rotation period"));
```

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


Implementation Details
----------------------
### Class structure
![](doc/classdiagram.png?raw=true "Class Diagram")
TO BE COMPLETED

### Exceptions
![](doc/exceptions.png?raw=true "Exceptions")

### Parser's state machine 
![](doc/statediagram.png?raw=true "State machine")


