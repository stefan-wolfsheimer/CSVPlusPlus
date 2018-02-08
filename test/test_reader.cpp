/******************************************************************************
Copyright (c) 2015, Stefan Wolfsheimer

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/

#include <catch.hpp>
#include <csv/reader.h>
#include <iostream>
#include <limits>

struct TestStreamResult
{

  typedef std::vector<std::pair<std::size_t,std::size_t> > 
  column_position_type;

  struct row_position_type
  {
    std::size_t          input_line;
    std::size_t          row;
    column_position_type input_columns;

    row_position_type(std::size_t i, 
                      std::size_t r,
                      const column_position_type & columns = 
                      column_position_type())
      : input_line(i), row(r), input_columns(columns) 
    {}

    row_position_type(std::size_t i, 
                      const column_position_type & columns = 
                          column_position_type())
      : input_line(i), 
        row(::std::numeric_limits<std::size_t>::max()), 
        input_columns(columns)
    {}


    friend std::ostream & operator<<(std::ostream & ost, 
                                     const row_position_type & obj) 
    {
      ost << obj.input_line << " " << obj.row << ":";
      for(auto c : obj.input_columns) 
      {
        ost << " " << c.first << "/" << c.second;
      }
      ost << std::endl;
      return ost;
    };
    
    bool operator==(const row_position_type & rhs) const 
    {
      return 
        input_line    == rhs.input_line && 
        row           == rhs.row && 
        input_columns == rhs.input_columns;
    }
  };

  typedef std::vector<row_position_type>           position_type;
  typedef std::vector<std::vector<std::string> >   data_type;
  data_type     result;
  bool          isEof;
  bool          csv_cells_ok;
  position_type positions;

  TestStreamResult() 
  {
    isEof        = true;
    csv_cells_ok = true;
  }
  
  TestStreamResult(const data_type & data) 
    : result(data) 
  {
    isEof        = true;
    csv_cells_ok = true;
  }


  bool operator==(const TestStreamResult & rhs) const
  {
    return 
      result       == rhs.result && 
      isEof        == rhs.isEof && 
      csv_cells_ok == rhs.csv_cells_ok;
  }

  bool operator==(const position_type & rhs) const
  {
    if(positions.size() != rhs.size()) 
    {
      return false;
    }
    for(std::size_t i = 0; i < rhs.size(); i++) 
    {
      std::size_t lhs_row = positions[i].row;
      std::size_t rhs_row = rhs[i].row;
      if(lhs_row == ::std::numeric_limits<std::size_t>::max()) 
      {
        lhs_row = i;
      }
      if(rhs_row == ::std::numeric_limits<std::size_t>::max()) 
      {
        rhs_row = i;
      }
      if( positions[i].input_line    != rhs[i].input_line || 
          positions[i].input_columns != rhs[i].input_columns || 
          lhs_row                    != rhs_row) 
      {
        return false;
      }
    }
    return true;
  }


  friend std::ostream & operator<<(std::ostream & ost, 
                                   const TestStreamResult & obj) 
  {
    std::size_t i = 0;
    for(const auto & row : obj.result) 
    {
      bool first = true;
      if(i < obj.positions.size()) 
      {
        ost << obj.positions[i].input_line << " ";
        ost << obj.positions[i].row << " ";
      }
      std::size_t j = 0;
      for(const auto & col : row) 
      {
        if(first) 
        {
          first =false;
        }
        else 
        {
          ost << ",";
        }
        if(i < obj.positions.size() && 
           j < obj.positions[i].input_columns.size()) 
        {
          ost << obj.positions[i].input_columns[j].first << "/";
          ost << obj.positions[i].input_columns[j].second << " ";
          
        }
        ost << "[";
        for(auto ch : col) 
        {
          if(ch == '\t') ost << "\\t";
          else if(ch == '\r') ost << "\\r";
          else if(ch == '\n') ost << "\\n";
          else ost << ch;
        }
        ost << "]";
        j++;
      }
      ost << std::endl;
      i++;
    }
    ost << " isEof=" << obj.isEof << std::endl;
    return ost;
  }
};



TestStreamResult parseStream(const std::string & str,
                             csv::Specification specs = 
                             csv::Specification())
{
  typedef TestStreamResult::column_position_type column_position_type;
  std::stringstream ss(str);
  TestStreamResult ret;
  csv::Reader reader(ss, specs); 
  std::size_t i = 0;
  for(auto row : reader) 
  {
    std::vector<std::string> row_vector;
    TestStreamResult::row_position_type rpos(row.inputLine(),
                                             row.row(),
                                             column_position_type());
    std::size_t j = 0;
    for(auto col : row) 
    {
      row_vector.push_back(col.as<std::string>());
      rpos.input_columns.push_back(std::make_pair(
                                                  col.inputLine(),
                                                  col.inputColumn()));
      if(j != col.column() || 
         i != col.row() ) 
      {
        std::cout << i << " " << col.row()  << " "
                  << j << " " << col.column() << std::endl;
        ret.csv_cells_ok = false;
      }
      j++;
    }
    ret.result.push_back(row_vector);
    ret.positions.push_back(rpos);
    i++;
  }
  ret.isEof = ss.eof();
  return ret;
}



TEST_CASE("ReadEmptyFile", "[csv_reader]")
{
  REQUIRE(parseStream("") == TestStreamResult());
}


TEST_CASE("ReadOnlyWhiteSpace", "[csv_reader]")
{
  REQUIRE(parseStream("  ") == TestStreamResult());
}

TEST_CASE("ReadOnlyEmptyLines", "[csv_reader]")
{
  REQUIRE(parseStream("\r\n\n\n   \n") == TestStreamResult());
}


TEST_CASE("ReadOneEmptyCell", "[csv_reader]")
{
  REQUIRE(parseStream("\"\"")== TestStreamResult({ 
        std::vector<std::string>{ "" } 
      }));
}

TEST_CASE("ReadTwoEmptyCells", "[csv_reader]")
{

  REQUIRE(parseStream(",")== TestStreamResult(
      { 
        std::vector<std::string>{ "","" } 
      }));
}

TEST_CASE("ReadThreeEmptyCells", "[csv_reader]")
{
  REQUIRE(parseStream(",,")== TestStreamResult({ 
        std::vector<std::string>{ "","","" } 
      }));
}

TEST_CASE("ReadThreeEmptyCellsWithWhitespaces", "[csv_reader]")
{
  REQUIRE(parseStream(" , ,   ")== TestStreamResult({ 
        std::vector<std::string>{ "","","" } 
      }));
}

TEST_CASE("ReadEmptyCellsInDifferentRows", "[csv_reader]")
{
  REQUIRE(parseStream(",,\n,")== TestStreamResult({ 
        std::vector<std::string>{ "","","" },
        std::vector<std::string>{ "","" } 
      }));
}

TEST_CASE("ReadEmptyCellsInDifferentRowsWithWhitespaces", "[csv_reader]")
{
  REQUIRE(parseStream(" , ,   \n  ,")== TestStreamResult({ 
        std::vector<std::string>{ "","","" },
        std::vector<std::string>{ "","" } 
      }));
}

TEST_CASE("ReadThreeEmptyQuotedAndUnquotedCells", "[csv_reader]")
{
  REQUIRE(parseStream("\"\",,")== TestStreamResult({ 
        std::vector<std::string>{ "","","" } 
      }));
  REQUIRE(parseStream(" \"\" , ,   ")== TestStreamResult({ 
        std::vector<std::string>{ "","","" } 
      }));
  REQUIRE(parseStream(", \"\" , ")== TestStreamResult({ 
        std::vector<std::string>{ "","","" } 
      }));
  REQUIRE(parseStream(",,\"\"")== TestStreamResult({ 
        std::vector<std::string>{ "","","" } 
      }));
  REQUIRE(parseStream(", , \"\" ")== TestStreamResult({ 
        std::vector<std::string>{ "","","" } 
      }));
}


TEST_CASE("ReadOneNoneEmptyCell", "[csv_reader]")
{
  REQUIRE(parseStream("abc") == TestStreamResult({ 
      std::vector<std::string>{ "abc" } 
      }));
}

TEST_CASE("ReadOneNoneEmptyCellWithWhitespace", "[csv_reader]")
{
  REQUIRE(parseStream("  \tabc\t ") == TestStreamResult({ 
      std::vector<std::string>{ "abc" } 
      }));
}

TEST_CASE("ReadOneNoneEmptyQuotedCellWithWhitespace", "[csv_reader]")
{
  REQUIRE(parseStream("  \" \tabc \"  ") == TestStreamResult({ 
        std::vector<std::string>{ " \tabc " } 
      }));
}

TEST_CASE("ReadOneRow", "[csv_reader]")
{
  REQUIRE(parseStream(" \n abc , def,ghi  \n\n") == TestStreamResult({ 
        std::vector<std::string>{ "abc","def","ghi" } 
      }));
  REQUIRE(parseStream(" \n abc ,\" def\",ghi,,\"\"") == 
          TestStreamResult({ 
              std::vector<std::string>{ "abc"," def","ghi","","" } 
      }));
}

TEST_CASE("ReadOneRowFirstColumnEmpty", "[csv_reader]")
{

  // first empty
  REQUIRE(parseStream(",a,b,c")== TestStreamResult({ 
        std::vector<std::string>{ "","a","b","c" } 
      }));
  REQUIRE(parseStream("  , a  ,  b  , c  ")== TestStreamResult({ 
        std::vector<std::string>{ "","a","b","c" } 
      }));
  REQUIRE(parseStream("\n,a,b,c\r\n")== TestStreamResult({ 
        std::vector<std::string>{ "","a","b","c" } 
      }));
  REQUIRE(parseStream("\"\",a,b,c")== TestStreamResult({ 
        std::vector<std::string>{ "","a","b","c" } 
      }));
  REQUIRE(parseStream(" \"\" , a , b , c  \n")== TestStreamResult({ 
        std::vector<std::string>{ "","a","b","c" } 
      }));
}

TEST_CASE("ReadOneRowSecondColumnEmpty", "[csv_reader]")
{
  REQUIRE(parseStream("a,,b,c")== TestStreamResult({ 
        std::vector<std::string>{ "a","","b","c" } 
      }));
  REQUIRE(parseStream("  \"a\" ,  ,  b , c  ")== TestStreamResult({ 
        std::vector<std::string>{ "a","","b","c" } 
      }));
  REQUIRE(parseStream("a,\"\",b,c")== TestStreamResult({ 
        std::vector<std::string>{ "a","","b","c" } 
      }));
  REQUIRE(parseStream(" a , \"\" , b , c ")== TestStreamResult({ 
        std::vector<std::string>{ "a","","b","c" } 
      }));
}

TEST_CASE("ReadOneRowLastColumnEmpty", "[csv_reader]")
{
  REQUIRE(parseStream("a,b,c,")== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
  REQUIRE(parseStream("a,b,c,\n")== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
  REQUIRE(parseStream("\ta , b \t, c  ,  ")== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
  REQUIRE(parseStream("\ta , b \t, c  ,  \n  ")== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
  REQUIRE(parseStream("a,b,c,\"\"")== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
  REQUIRE(parseStream("a,b,c,\"\"\r\n")== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
  REQUIRE(parseStream(" a , b , c  , \"\""  )== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
  REQUIRE(parseStream(" a , b , c  , \"\"  \n"  )== TestStreamResult({ 
        std::vector<std::string>{ "a","b","c","" } 
      }));
}

TEST_CASE("ReadOneQuotedCell", "[csv_reader]")
{
  REQUIRE(parseStream("\"a \"\"\"\" bc\"") == TestStreamResult({ 
        std::vector<std::string>{ "a \"\" bc" } 
      }));
  REQUIRE(parseStream("\"abc\"\"\"") == TestStreamResult({ 
        std::vector<std::string>{ "abc\"" } 
      }));
  REQUIRE(parseStream(" \n abc  \n\n ") == TestStreamResult({ 
        std::vector<std::string>{ "abc" } 
      }));
  REQUIRE(parseStream(" \" a\nbc \t\n \" \n\n ") == TestStreamResult({ 
        std::vector<std::string>{ " a\nbc \t\n " } 
      }));
}

TEST_CASE("ReadOneColumn", "[csv_reader]")
{
  REQUIRE(parseStream(" a1\r\n \n  b1  b2  \r\n\"a3\"\n\r\n") == 
          TestStreamResult({ 
              std::vector<std::string>{ "a1" },
              std::vector<std::string>{ "b1  b2" },
              std::vector<std::string>{ "a3" }
      }));
}

TEST_CASE("ReadMultipleRowsWithBegin", "[csv_reader]")
{
  std::stringstream ss(" a \n  b \n c \n");
  csv::Reader::iterator first_row_iterator; 
  csv::Reader::iterator second_row_iterator; 
  csv::Reader::iterator third_row_iterator; 
  csv::Row row1,row2,row3;
  {
    csv::Reader reader(ss); 
    csv::Reader::iterator first_row_iterator  = reader.begin();
    csv::Reader::iterator second_row_iterator = reader.begin(); 
    csv::Reader::iterator third_row_iterator  = reader.begin();
    csv::Reader::iterator end_iterator        = reader.begin();
    REQUIRE(end_iterator == reader.end());
    row1 = *first_row_iterator;
    row2 = *second_row_iterator;
    row3 = *third_row_iterator;
  }
  auto cell1 = *row1.begin();
  auto cell2 = *row2.begin();
  auto cell3 = *row3.begin();
  REQUIRE("a" == cell1.as<std::string>());
  REQUIRE("b" == cell2.as<std::string>());
  REQUIRE("c" == cell3.as<std::string>());
}

TEST_CASE("ReadMatrix", "[csv_reader]")
{
  std::string csv = 
    " a1; a2; a3 \r\n"
    "b1  ; \"b\n2\"\n"
    ";;;\n"
    "\n"
    " c1 ;  c2.1,c2.2";
  REQUIRE(parseStream(csv,
                      csv::Specification()
                        .withSeparator(";")) ==
          TestStreamResult({ 
              std::vector<std::string>{ "a1","a2","a3" },
              std::vector<std::string>{ "b1","b\n2" },
              std::vector<std::string>{ "","","","" },
              std::vector<std::string>{ "c1","c2.1,c2.2" }
      }));
}

TEST_CASE("ReadWhileNotIgnoreEmptyLines", "[csv_reader]")
{
  std::string csv;
  csv = 
    " a1; a2; a3 \r\n"
    "\n"
    "b1  ; \"b2\"  \n"
    "\n"
    " c1 ;  c2.1,c2.2"
    "\r\n";
  REQUIRE(parseStream(csv,
                      csv::Specification()
                      .withSeparator(";")
                      .withUsingEmptyLines()) == 
          TestStreamResult({ 
              std::vector<std::string>{ "a1","a2","a3" },
              std::vector<std::string>{ },
              std::vector<std::string>{ "b1","b2" },
              std::vector<std::string>{ },
              std::vector<std::string>{ "c1","c2.1,c2.2" },
                }));

  csv = 
    "\"a\"\r\n"
    "\n"
    "\n"
    "b "
    "\n"
    "\r\n";
  REQUIRE(parseStream(csv,
                      csv::Specification()
                      .withSeparator(";")
                      .withUsingEmptyLines()) == 
          TestStreamResult({ 
              std::vector<std::string>{ "a" },
              std::vector<std::string>{ },
              std::vector<std::string>{ },
              std::vector<std::string>{ "b" },
              std::vector<std::string>{ }
      }));
  csv = 
    "\"a\",\r\n"
    "\n"
    ",,\n"
    ",,\"\"\n"
    "b, "
    "\n"
    "\r\n";
  REQUIRE(parseStream(csv,
                      csv::Specification()
                      .withUsingEmptyLines()) == 
          TestStreamResult({ 
              std::vector<std::string>{ "a","" },
              std::vector<std::string>{ },
              std::vector<std::string>{ "","","" },
              std::vector<std::string>{ "","","" },
              std::vector<std::string>{ "b","" },
              std::vector<std::string>{ }
      }));
}

TEST_CASE("ReadOneRowWithMultipleSeparators", "[csv_reader]")
{
  
  REQUIRE(parseStream(" \n abc ,; def;ghi,1  \n\n",
                      csv::Specification()
                       .withSeparator(",;")) ==
          TestStreamResult({ 
              std::vector<std::string>{ "abc","","def","ghi","1" } 
          }));
}

TEST_CASE("ReadWhiteSpaceAsSeparator", "[csv_reader]")
{
  auto spec = csv::Specification()
    .withSeparator(" \t");

  REQUIRE(parseStream("  a  ", spec) == 
          TestStreamResult({ 
              std::vector<std::string>{"a"} }));
  REQUIRE(parseStream(" \"a\"  ", spec) == 
          TestStreamResult({ 
              std::vector<std::string>{"a"} }));
  REQUIRE(parseStream("a b", spec) == 
          TestStreamResult({ 
              std::vector<std::string>{ "a","b"} }));
  REQUIRE(parseStream("\"a\" \"b\"", spec) == 
          TestStreamResult({ 
              std::vector<std::string>{ "a","b"} }));

  REQUIRE(parseStream("  a   b  ", spec) == 
          TestStreamResult({ 
              std::vector<std::string>{ "a","b"} }));

  REQUIRE(parseStream("  \"a b \"  ", spec) == 
          TestStreamResult({ 
              std::vector<std::string>{ "a b "} }));

  REQUIRE(parseStream("\"1\" 2 3\n"
                      "  \"1\"   2   3 \n"
                      "1 \"2\" 3\r\n"
                      "1  \"2\"  3 \r\n"
                      "1 2 \"3\"\r\n"
                      "1 2 \"3\"    \n", spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "1","2","3"},
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"} 
          }));
}

TEST_CASE("ReadWhiteSpaceAsSeparatorAndUseEmptyLines", "[csv_reader]")
{

  auto spec = 
    csv::Specification()
     .withSeparator(" \t")
     .withUsingEmptyLines();
  REQUIRE(parseStream("\"1\" 2 3\n  \t  \n"
                      "  \"1\"   2   3 \n"
                      "1 \"2\" 3\r\n"
                      "1  \"2\"  3 \r\n"
                      "1 2 \"3\"\r\n"
                      "1 2 \"3\"    \n\r\n", spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "1","2","3"},
              std::vector<std::string>{ },
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"}, 
              std::vector<std::string>{ "1","2","3"},
              std::vector<std::string>{ }
          }));

}

TEST_CASE("ReadCharWithCommentSymbol", "[csv_reader]")
{
  auto spec = csv::Specification()
    .withComment('#');
  REQUIRE(parseStream("   #",   spec) == TestStreamResult());
  REQUIRE(parseStream(" \"#\"", spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "#" }
          }));

  REQUIRE(parseStream("   #\na",spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "a" }
          }));
  REQUIRE(parseStream("  a#,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "a" },
              std::vector<std::string>{ "1","2","3" }
          }));
  REQUIRE(parseStream("  a #,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "a" },
              std::vector<std::string>{ "1","2","3" }
          }));
  REQUIRE(parseStream("  \"a\"#,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "a" },
              std::vector<std::string>{ "1","2","3" }
          }));

  REQUIRE(parseStream("  \"a\" #,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
              std::vector<std::string>{ "a" },
              std::vector<std::string>{ "1","2","3" }
          }));
}


TEST_CASE("ReadAndCopyOneRow", "[csv_reader]")
{
  csv::Cell c1, c2, c3;
  {
    csv::Row row;
    {
      std::stringstream ss(" \n abc , def,ghi  \n\n");
      csv::Reader reader(ss); 
      row = *reader.begin();
    }
    c1 = row[0];
    c2 = row[1];
    c3 = row[2];
  }
  REQUIRE(c1.as<std::string>() == "abc");
  REQUIRE(c2.as<std::string>() == "def");
  REQUIRE(c3.as<std::string>() == "ghi");
  REQUIRE(c1.inputColumn() == 1u);
  REQUIRE(c2.inputColumn() == 7u);
  REQUIRE(c3.inputColumn() == 11u);
  REQUIRE(c1.inputLine()   == 1u);
  REQUIRE(c2.inputLine()   == 1u);
  REQUIRE(c3.inputLine()   == 1u);
}

TEST_CASE("InputLineCountEmptyCells", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type    row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(0, { 
                            std::make_pair(0,0),
                            std::make_pair(0,1),
                            std::make_pair(0,2) 
                           }),
    row_position_type(1, {
                            std::make_pair(1,0),
                            std::make_pair(1,1),
                            std::make_pair(1,2) 
                           }),
    row_position_type(3, {
                            std::make_pair(3,0),
                            std::make_pair(3,1),
                            std::make_pair(3,2) 
                          })
  };
  REQUIRE(parseStream(",,\n,,\n\n,,") == required);
}

TEST_CASE("InputLineCount", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type    row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(0, { 
                             std::make_pair(0,0),
                             std::make_pair(0,2),
                             std::make_pair(0,5) }),
    row_position_type(1, {
                            std::make_pair(1,0)
                           }),
    row_position_type(2, {
                             std::make_pair(2,0)
                           })
  };
  REQUIRE(parseStream("a,a2,a3\nb\nc") == required);
}

TEST_CASE("InputLineCountWithWhiteSpace", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type    row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(0, { 
                            std::make_pair(0,2),
                            std::make_pair(0,8),
                            std::make_pair(0,15) }),
    row_position_type(1, {
                            std::make_pair(1,2)
                           }),
    row_position_type(2, {
                             std::make_pair(2,1)
                          })
  };
  REQUIRE(parseStream("  a  ,  a2  ,  a3  \n  b  \n c  \n") == required);
}

TEST_CASE("InputLineCountWithIgnoredEmptyLines", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(1, {
                            std::make_pair(1,0)
                           }),
    row_position_type(4, {
                             std::make_pair(4,0)
                         }),
    row_position_type(5,{
                           std::make_pair(5,0)
                        })
  };
  REQUIRE(parseStream("\na\n\n\nb\nc\n") == required);
}

TEST_CASE("InputLineCountWithIgnoredEmptyLinesWithSpace", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(1,{
                           std::make_pair(1,1)
                        }),
    row_position_type(4,{
                           std::make_pair(4,2)
                        }),
    row_position_type(5,{
                           std::make_pair(5,3)
                        })
  };
  REQUIRE(parseStream("   \n a\n\n\n  b\n   c   \n   ") == required);
}

TEST_CASE("InputLineCountUsingEmptyLines", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type row_position_type;
  auto spec = csv::Specification().withUsingEmptyLines();
  auto required = TestStreamResult::position_type{
    row_position_type(0),
    row_position_type(1,{
                           std::make_pair(1,0)
                        }),
    row_position_type(2),
    row_position_type(3),
    row_position_type(4, {
                           std::make_pair(4,0)
                         }),
    row_position_type(5, {
                           std::make_pair(5,0)
                         })
    // last newline ignored
  };
  REQUIRE(parseStream("\na\n\n\nb\nc\n", spec) == required);
}

TEST_CASE("InputLineCountUsingEmptyLinesWithWhiteSpaces", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type row_position_type;
  auto spec = csv::Specification().withUsingEmptyLines();
  auto required = TestStreamResult::position_type{
    row_position_type(0),
    row_position_type(1,{
                           std::make_pair(1,1)
                        }),
    row_position_type(2),
    row_position_type(3),
    row_position_type(4, {
                           std::make_pair(4,2)
                         }),
    row_position_type(5, {
                           std::make_pair(5,2)
                         })
    // last newline ignored
  };
  REQUIRE(parseStream("   \n a\n\n\n  b\n  c\n", spec) == required);
}

TEST_CASE("InputLineCountQuoted", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type    row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(0, { 
                           std::make_pair(0,0),
                           std::make_pair(0,4),
                           std::make_pair(0,9) }),
    row_position_type(1, {
                           std::make_pair(1,0),
                           std::make_pair(1,4),
                           std::make_pair(1,9)
                         }),
    row_position_type(2, {
                           std::make_pair(2,0)
                         })
  };
  REQUIRE(parseStream("\"a\",\"a2\",\"a3\"\n\"b\",\"b2\",\nc") == required);
}

TEST_CASE("InputLineCountQuotedWithWhiteSpace", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type    row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(0, { 
                           std::make_pair(0,1),
                           std::make_pair(0,5),
                           std::make_pair(0,10) }),
    row_position_type(1, {
                           std::make_pair(1,0),
                           std::make_pair(1,6),
                           std::make_pair(1,11)
                         }),
    row_position_type(2, {
                           std::make_pair(2,1)
                         })
  };
  REQUIRE(parseStream(" \"a\",\"a2\",\"a3\"\n\"b\",  \"b2\",\n \"c\"") == 
          required);
}

TEST_CASE("InputLineCountQuotedWithNewline", "[csv_reader]")
{
  typedef TestStreamResult::row_position_type    row_position_type;
  auto required = TestStreamResult::position_type{
    row_position_type(0, { 
                           std::make_pair(0,1),
                           std::make_pair(2,2),
                           std::make_pair(2,7) }),
    row_position_type(3, {
                           std::make_pair(3,0),
                           std::make_pair(3,6),
                           std::make_pair(4,3)
                         }),
    row_position_type(5, {
                           std::make_pair(5,1)
                         })
  };
  REQUIRE(parseStream(" \"a\n\n\",\"a2\",\"a3\"\n\"b\",  \"b\n2\",\n \"c\"") == 
          required);
}

TEST_CASE("ReadWithHeader", "[csv_reader]")
{

  auto spec = csv::Specification()
    .withColumn(0,"first")
    .withColumn(1,"second")
    .withColumn(2,"third")
    .withColumn(5,"sixth");
  std::stringstream ss("0,1,2,3,4,5,6,7\n8,9");
  csv::Reader reader(ss, spec); 
  auto itr = reader.begin();
  auto row1 = *itr;
  ++itr;
  auto row2 = *itr;
  REQUIRE(row1[0].name() == "first");
  REQUIRE(row1[1].name() == "second");
  REQUIRE(row1[2].name() == "third");
  REQUIRE(row1[3].name() == "");
  REQUIRE(row1[4].name() == "");
  REQUIRE(row1[5].name() == "sixth");
  REQUIRE(row1[6].name() == "");
  REQUIRE(row1[7].name() == "");
  REQUIRE(row1["first"].as<int>()  == 0);
  REQUIRE(row1["second"].as<int>() == 1);
  REQUIRE(row1["third"].as<int>()  == 2);
  REQUIRE(row1["sixth"].as<int>()  == 5);
  REQUIRE_THROWS(row1["xxx"].name());
  REQUIRE_THROWS(row1[8].name());
  REQUIRE(row2[0].name() == "first");
  REQUIRE(row2[1].name() == "second");
  REQUIRE_THROWS(row2["third"].name());
}

TEST_CASE("ReadWithHeaderFromCsvFile", "[csv_reader]")
{
  auto spec = csv::Specification().withHeader();
  std::stringstream ss("first,second,third,,,sixth\n0,1,2,3,4,5,6,7\n8,9");
  csv::Reader reader(ss, spec); 
  auto itr = reader.begin();
  auto row1 = *itr;
  ++itr;
  auto row2 = *itr;
  REQUIRE(row1[0].row()  == 1);
  REQUIRE(row1[0].name() == "first");
  REQUIRE(row1[1].name() == "second");
  REQUIRE(row1[2].name() == "third");
  REQUIRE(row1[3].name() == "");
  REQUIRE(row1[4].name() == "");
  REQUIRE(row1[5].name() == "sixth");
  REQUIRE(row1[6].name() == "");
  REQUIRE(row1[7].name() == "");
  REQUIRE(row1["first"].as<int>()  == 0);
  REQUIRE(row1["second"].as<int>() == 1);
  REQUIRE(row1["third"].as<int>()  == 2);
  REQUIRE(row1["sixth"].as<int>()  == 5);
  REQUIRE_THROWS(row1["xxx"].name());
  REQUIRE_THROWS(row1[8].name());
  REQUIRE(row2[0].name() == "first");
  REQUIRE(row2[1].name() == "second");
  REQUIRE_THROWS(row2["third"].name());
}

TEST_CASE("ReadWithHeaderWithDuplicateHeaderThrows", "[csv_reader]")
{
  auto spec = csv::Specification().withHeader();
  
  std::stringstream ss("\nfirst,second,first,,,sixth\n0,1,2,3,4,5,6,7\n8,9");
  bool caught = false;
  try 
  {
    csv::Reader reader(ss, spec); 
  }
  catch( csv::DuplicateColumnError ex) 
  {
    caught = true;
    REQUIRE(ex.inputLine()     == 1u);
    REQUIRE(ex.inputColumn()   == 13u);
    REQUIRE(ex.row()           == 0u);
    REQUIRE(ex.column()        == 2u);
    REQUIRE(ex.index()         == 0u);
  }
  REQUIRE(caught);
}

TEST_CASE("CharacterAfterClosingQuotedColumnThrows", "[csv_reader]")
{
  bool caught = false;
  try 
  {
    parseStream("a\nb\n\"c1\",\"c2\"x,c3");
  }
  catch(csv::ParseError ex)
  {
    REQUIRE(ex.inputLine()   == 2);
    REQUIRE(ex.inputColumn() == 9);
    REQUIRE(ex.row()         == 2);
    REQUIRE(ex.column()      == 1);
    caught = true;
  }
  REQUIRE(caught);
}

TEST_CASE("CellAccessorBoundChecksThrowsWithCorrectCoordinates","[csv_reader]")
{
  std::stringstream ss("0,1,2,3,4,5,6,7\n8,9\n\n10,11");
  csv::Reader reader(ss); 
  auto itr = reader.begin();
  bool caught1 = false;
  bool caught2 = false;
  bool caught3 = false;
  try 
  {
    auto row = *itr; ++itr;
    row[8];
  }
  catch(csv::CellOutOfRangeError ex) 
  {
    REQUIRE(ex.index() == 8);
    REQUIRE(ex.size()  == 8);
    REQUIRE(ex.inputLine() == 0);
    REQUIRE(ex.inputColumn() == 14);
    REQUIRE(ex.row() == 0);
    REQUIRE(ex.column() == 7);
    caught1 = true;
  }
  try
  {
    auto row = *itr; ++itr;
    row[3];
  }
  catch(csv::CellOutOfRangeError ex) 
  {
    REQUIRE(ex.index() == 3);
    REQUIRE(ex.size()  == 2);
    REQUIRE(ex.inputLine() == 1);
    REQUIRE(ex.inputColumn() == 2);
    REQUIRE(ex.row() == 1);
    REQUIRE(ex.column() == 1);
    caught2 = true;
  }
  try
  {
    auto row = *itr; ++itr;
    REQUIRE(row[1].as<int>() == 11);
    row[3];
  }
  catch(csv::CellOutOfRangeError ex) 
  {
    REQUIRE(ex.index() == 3);
    REQUIRE(ex.size()  == 2);
    REQUIRE(ex.inputLine() == 3);
    REQUIRE(ex.inputColumn() == 3);
    REQUIRE(ex.row() == 2);
    REQUIRE(ex.column() == 1);
    caught3 = true;
  }
  REQUIRE(caught1);
  REQUIRE(caught2);
  REQUIRE(caught3);
}

TEST_CASE("CellAccessorUndefinedColumns","[csv_reader]")
{
  auto spec = csv::Specification().withHeader();
  std::stringstream ss("0,1,2,3,4,5,6,7\na,b,c,d\ne,f,g");
  csv::Reader reader(ss, spec); 
  auto itr = reader.begin();
  bool caught1 = false;
  bool caught2 = false;
  try 
  {
    auto row = *itr; ++itr;
    REQUIRE(row["0"].as<std::string>() == "a");
    REQUIRE(row["0"].row()             == 1);
    row["xxx"];
  }
  catch(csv::UndefinedColumnError ex) 
  {
    REQUIRE(ex.size()        == 4);
    REQUIRE(ex.inputLine()   == 1);
    REQUIRE(ex.inputColumn() == 6);
    REQUIRE(ex.row()         == 1);
    REQUIRE(ex.column()      == 3);
    caught1 = true;
  }
  try
  {
    auto row = *itr; ++itr;
    row["7"];
  }
  catch(csv::DefinedCellOutOfRangeError ex) 
  {
    REQUIRE(ex.index()       == 7);
    REQUIRE(ex.size()        == 3);
    REQUIRE(ex.inputLine()   == 2);
    REQUIRE(ex.inputColumn() == 4);
    REQUIRE(ex.row()         == 2);
    REQUIRE(ex.column()      == 2);
    caught2 = true;
  }
  REQUIRE(caught1);
  REQUIRE(caught2);
}
