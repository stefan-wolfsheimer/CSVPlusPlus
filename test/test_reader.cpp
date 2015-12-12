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

#include "catch.hpp"
#include "csv/reader.h"
#include <iostream>
#include <limits>

struct TestStreamResult
{
  typedef std::pair<std::size_t, std::size_t>      position_type;
  typedef std::pair<std::string, position_type>    cell_type;
  typedef std::vector<std::vector<cell_type> >     data_type;
  typedef std::vector<position_type>               coordinate_type;

  data_type       _result;
  bool            _is_eof;
  std::size_t     _eof_input_line;
  std::size_t     _eof_input_column;
  coordinate_type _coordinates;

  TestStreamResult(std::size_t eof_input_line =0, std::size_t eof_input_column=0) 
  {
    _eof_input_line   = eof_input_line;
    _eof_input_column = eof_input_column;
    _is_eof           = true;
  }
  
  TestStreamResult(const data_type & data,
                   std::size_t       eof_input_line =0, 
                   std::size_t       eof_input_column=0) 
    : _result(data) 
  {
    _is_eof           = true;
    _eof_input_line   = eof_input_line;
    _eof_input_column = eof_input_column;
    std::size_t i = 0;
    for(const auto & row : _result) 
    {
      std::size_t j = 0;
      for(const auto & cell : row) 
      {
        _coordinates.push_back(std::make_pair(i,j));
        j++;
      }
      i++;
    }
  }


  bool operator==(const TestStreamResult & rhs) 
  {
    return 
      _eof_input_line   == rhs._eof_input_line && 
      _eof_input_column == rhs._eof_input_column && 
      _result           == rhs._result && 
      _is_eof           == rhs._is_eof && 
      _coordinates      == rhs._coordinates;
  }

  friend std::ostream & operator<<(std::ostream           & ost, 
                                   const TestStreamResult & obj) 
  {
    for(const auto & row : obj._result) 
    {
      bool first = true;
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
        ost << "([";
        for(auto ch : col.first) 
        {
          if(ch == '\t') ost << "\\t";
          else if(ch == '\r') ost << "\\r";
          else if(ch == '\n') ost << "\\n";
          else ost << ch;
        }
        ost << "]" << " " << col.second.first << " " << col.second.second << ")";
      }
      ost << std::endl;
    }
    ost << " is_eof=" << obj._is_eof << " ";
    ost << " eof=" << obj._eof_input_line << ":" << obj._eof_input_column << std::endl;
    bool first = true;
    for(const auto & c : obj._coordinates) 
    {
      if(first)
      {
        first = false;
      }
      else 
      {
        ost << ", ";
      }
      ost << "(" << c.first << "," << c.second << ")";
    }
    return ost;
  }
};


TestStreamResult::cell_type makeCell(const std::string & name, std::size_t r, std::size_t c)
{
  return std::make_pair(name, std::make_pair(r,c));
}

TestStreamResult parseStream(const std::string & str,
                             csv::Specification specs = 
                             csv::Specification())
{
  std::stringstream ss(str);
  TestStreamResult  ret;
  csv::Reader       reader(ss, specs);
  std::size_t       i = 0;
  for(auto row : reader) 
  {
    std::vector<TestStreamResult::cell_type>  row_vector;
    for(auto col : row) 
    {
      row_vector.push_back(std::make_pair(col.as<std::string>(),
                                          std::make_pair(col.inputLine(), 
                                                         col.inputColumn())));
      ret._coordinates.push_back(std::make_pair(col.row(),col.column()));
    }
    ret._result.push_back(row_vector);
  }
  ret._is_eof = ss.eof();
  ret._eof_input_line = reader.inputLine();
  ret._eof_input_column = reader.inputColumn();
  return ret;
}

TEST_CASE("ReadEmptyFile", "[csv_reader]")
{
  REQUIRE(parseStream("") == TestStreamResult());
}

TEST_CASE("ReadOnlyWhiteSpace", "[csv_reader]")
{
  REQUIRE(parseStream("  ") == TestStreamResult(0,2));
}

TEST_CASE("ReadOnlyEmptyLines", "[csv_reader]")
{
  REQUIRE(parseStream("\r\n\n\n   \n ") == TestStreamResult(4,1));
}

TEST_CASE("ReadOneEmptyCell", "[csv_reader]")
{
  REQUIRE(parseStream("\"\"")== TestStreamResult(
      { 
        { { "", { 0,0} } }
      },0,2));
}

TEST_CASE("ReadTwoEmptyCells", "[csv_reader]")
{

  REQUIRE(parseStream(",")== TestStreamResult(
      { 
        { makeCell("",0,0), makeCell("",0,1) }
      },0,1));
}


TEST_CASE("ReadThreeEmptyCells", "[csv_reader]")
{
  REQUIRE(parseStream(",,")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,1), makeCell("",0,2) }
  },0,2));
}


TEST_CASE("ReadThreeEmptyCellsWithWhitespaces", "[csv_reader]")
{
  REQUIRE(parseStream(" , ,   ")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,2), makeCell("",0,4) }
      },0,7));
}


TEST_CASE("ReadEmptyCellsInDifferentRows", "[csv_reader]")
{
  REQUIRE(parseStream(",,\n,")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,1), makeCell("",0,2) },
        { makeCell("",1,0), makeCell("",1,1) }
      },1,1));
}

TEST_CASE("ReadEmptyCellsInDifferentRowsWithWhitespaces", "[csv_reader]")
{
  REQUIRE(parseStream(" , ,   \n  ,")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,2), makeCell("",0,4) },
        { makeCell("",1,0), makeCell("",1,3) }
      },1,3));
}


TEST_CASE("ReadThreeEmptyQuotedAndUnquotedCells", "[csv_reader]")
{
  REQUIRE(parseStream("\"\",,")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,3), makeCell("",0,4) }
      },0,4));
  REQUIRE(parseStream(" \"\" , ,   ")== TestStreamResult({ 
        { makeCell("",0,1), makeCell("",0,5), makeCell("",0,7) }
      },0,10));
  REQUIRE(parseStream(", \"\" , ")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,2), makeCell("",0,6) }
      },0,7));
  REQUIRE(parseStream(",,\"\"")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,1), makeCell("",0,2) }
      },0,4));
  REQUIRE(parseStream(", , \"\" ")== TestStreamResult({ 
        { makeCell("",0,0), makeCell("",0,1), makeCell("",0,4) }
      },0,7));
}

TEST_CASE("ReadOneNoneEmptyCell", "[csv_reader]")
{
  REQUIRE(parseStream("abc") == TestStreamResult({ 
        { makeCell("abc",0,0) }
      },0,3));
}


TEST_CASE("ReadOneNoneEmptyCellWithWhitespace", "[csv_reader]")
{
  REQUIRE(parseStream("  \tabc\t ") == TestStreamResult({ 
        { makeCell("abc",0,3) }
      },0,8));
}


TEST_CASE("ReadOneNoneEmptyQuotedCell", "[csv_reader]")
{
  REQUIRE(parseStream("\" \tabc \"") == TestStreamResult({ 
        { makeCell(" \tabc ",0,0) }
      },0,8));
}


TEST_CASE("ReadOneNoneEmptyQuotedCellWithWhitespace", "[csv_reader]")
{
  REQUIRE(parseStream("  \" \tabc \"  ") == TestStreamResult({ 
        { makeCell(" \tabc ",0,2) }
      },0,12));
}

TEST_CASE("ReadOneRow", "[csv_reader]")
{

  REQUIRE(parseStream("abc,def,ghi") == TestStreamResult({ 
         { makeCell("abc",0,0), makeCell("def",0,4),makeCell("ghi",0,8) }
      },0,11));
  REQUIRE(parseStream(" \n abc , def,ghi  \n\n") == TestStreamResult({ 
        { makeCell("abc",1,1), makeCell("def",1,7),makeCell("ghi",1,11) }
      },3,0));
  REQUIRE(parseStream(" \n abc ,\" def\",ghi,,\"\"") == TestStreamResult({ 
        { makeCell("abc",1,1), makeCell(" def",1,6),makeCell("ghi",1,13),
          makeCell("",1,17),   makeCell("",1,18) }
      },1,20));
}

TEST_CASE("ReadOneRowFirstColumnEmpty", "[csv_reader]")
{
  REQUIRE(parseStream(",a,b,c")== TestStreamResult({ 
        {   makeCell("",0,0), makeCell("a",0,1), makeCell("b",0,3), 
            makeCell("c",0,5) } } ,0,6));
  REQUIRE(parseStream("  , a  ,  b  , c  ")== TestStreamResult({ 
        {   makeCell("",0,0), makeCell("a",0,4), makeCell("b",0,10), 
            makeCell("c",0,15) } },0,18));
  REQUIRE(parseStream("\n,a,b,c\r\n")== TestStreamResult({ 
        {   makeCell("",1,0), makeCell("a",1,1), makeCell("b",1,3), 
            makeCell("c",1,5) } },2,0));
  REQUIRE(parseStream("\"\",a,b,c")== TestStreamResult({ 
        {   makeCell("",0,0), makeCell("a",0,3), makeCell("b",0,5), 
            makeCell("c",0,7) } },0,8));
  REQUIRE(parseStream(" \"\" , a , b , c  \n")== TestStreamResult({ 
        {   makeCell("",0,1), makeCell("a",0,6), makeCell("b",0,10), 
            makeCell("c",0,14) } },1,0));
}

TEST_CASE("ReadOneRowSecondColumnEmpty", "[csv_reader]")
{
  REQUIRE(parseStream("a,,b,c")== TestStreamResult({ 
        {   makeCell("a",0,0), makeCell("",0,2), makeCell("b",0,3), 
            makeCell("c",0,5) } } ,0,6));
  REQUIRE(parseStream("  \"a\" ,  ,  b , c  ")== TestStreamResult({ 
        {   makeCell("a",0,2), makeCell("",0,7), makeCell("b",0,12), 
            makeCell("c",0,16) } },0,19));
  REQUIRE(parseStream("a,\"\",b,c")== TestStreamResult({ 
        {   makeCell("a",0,0), makeCell("",0,2), makeCell("b",0,5), 
            makeCell("c",0,7) } },0,8));
  REQUIRE(parseStream(" a , \"\" , b , c ")== TestStreamResult({ 
        {   makeCell("a",0,1), makeCell("",0,5), makeCell("b",0,10), 
            makeCell("c",0,14) } },0,16));
}


TEST_CASE("ReadOneRowLastColumnEmpty", "[csv_reader]")
{
  REQUIRE(parseStream("a,b,c,")== TestStreamResult({ 
        {   makeCell("a",0,0), makeCell("b",0,2), makeCell("c",0,4), 
            makeCell("",0,6) } } ,0,6));
  REQUIRE(parseStream("a,b,c,\n")== TestStreamResult({ 
        {   makeCell("a",0,0), makeCell("b",0,2), makeCell("c",0,4), 
            makeCell("",0,6) } } ,1,0));
  REQUIRE(parseStream("\ta , b \t, c  ,  ")== TestStreamResult({ 
        {   makeCell("a",0,1), makeCell("b",0,5), makeCell("c",0,10), 
            makeCell("",0,14) } } ,0,16));
  REQUIRE(parseStream("\ta , b \t, c  ,  \n  ")== TestStreamResult({ 
        {   makeCell("a",0,1), makeCell("b",0,5), makeCell("c",0,10), 
            makeCell("",0,14) } } ,1,2));
  REQUIRE(parseStream("a,b,c,\"\"")== TestStreamResult({ 
        {   makeCell("a",0,0), makeCell("b",0,2), makeCell("c",0,4), 
            makeCell("",0,6) } } ,0,8));
  REQUIRE(parseStream("a,b,c,\"\"\r\n")== TestStreamResult({ 
        {   makeCell("a",0,0), makeCell("b",0,2), makeCell("c",0,4), 
            makeCell("",0,6) } } ,1,0));
  REQUIRE(parseStream(" a , b , c  , \"\""  )== TestStreamResult({ 
        {   makeCell("a",0,1), makeCell("b",0,5), makeCell("c",0,9), 
            makeCell("",0,14) } } ,0,16));
  REQUIRE(parseStream(" a , b , c  , \"\"  \n"  )== TestStreamResult({ 
        {   makeCell("a",0,1), makeCell("b",0,5), makeCell("c",0,9), 
            makeCell("",0,14) } } ,1,0));
}

TEST_CASE("ReadOneQuotedCell", "[csv_reader]")
{
  REQUIRE(parseStream("\"a \"\"\"\" bc\"") == TestStreamResult({ 
        {   makeCell("a \"\" bc",0,0) }},0,11));
  REQUIRE(parseStream("\"abc\"\"\"") == TestStreamResult({ 
        {   makeCell("abc\"",0,0) }},0,7)); 
  REQUIRE(parseStream(" \n abc\"  \n\n ") == TestStreamResult({ 
        {   makeCell("abc\"",1,1) }},3,1)); 
  REQUIRE(parseStream(" \" a\nbc \t\n \" \n\n ") == TestStreamResult({ 
        {   makeCell(" a\nbc \t\n ",0,1)}},4,1)); 
}

TEST_CASE("ReadOneColumn", "[csv_reader]")
{
  REQUIRE(parseStream(" a1\r\n \n  b1  b2  \r\n\"a3\"\n\r\n") == 
          TestStreamResult({ 
            {   makeCell("a1",    0,1)},
            {   makeCell("b1  b2",2,2)},
            {   makeCell("a3",    3,0)}}, 5,0));
}

TEST_CASE("ReadMultipleCopyOnWrite", "[csv_reader]")
{
  std::stringstream ss(" a \n  b \n c \n d \n\n");
  csv::Reader::iterator first_row_iterator; 
  csv::Reader::iterator second_row_iterator; 
  csv::Reader::iterator third_row_iterator; 
  csv::Row row1,row2,row3,row4;
  {
    csv::Reader reader(ss); 
    csv::Reader::iterator itr = reader.begin();
    row1 = *itr; ++itr;
    row2 = *itr; ++itr;
    row3 = *itr; ++itr;
    row4 = *itr; ++itr;
    REQUIRE(itr == reader.end());    
  }
  auto cell1 = *row1.begin();
  auto cell2 = *row2.begin();
  auto cell3 = *row3.begin();
  auto cell4 = *row4.begin();
  REQUIRE("a" == cell1.as<std::string>());
  REQUIRE("b" == cell2.as<std::string>());
  REQUIRE("c" == cell3.as<std::string>());
  REQUIRE("d" == cell4.as<std::string>());
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
                        .withoutSeparator(',')
                        .withSeparator(';')) == 
          TestStreamResult({ 
              { makeCell("a1",0,1), makeCell("a2",0,5), makeCell("a3",0,9) },
              { makeCell("b1",1,0), makeCell("b\n2",1,6) },
              { makeCell("",  3,0), makeCell("",3,1), makeCell("",3,2), 
                makeCell("",3,3) },
              { makeCell("c1",5,1), makeCell("c2.1,c2.2",5,7) }},5,16));
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
                      .withoutSeparator(',')
                      .withSeparator(';')
                      .withUsingEmptyLines()) == 
          TestStreamResult({ 
              { makeCell("a1",0,1), makeCell("a2",0,5), makeCell("a3",0,9) },
              {},
              { makeCell("b1",2,0), makeCell("b2",2,6) },
              {},
              { makeCell("c1",4,1), makeCell("c2.1,c2.2",4,7) }},5,0));

  csv = 
    "\"a\"\r\n"
    "\n"
    "\n"
    "b "
    "\n"
    "\r\n";
  REQUIRE(parseStream(csv,
                      csv::Specification()
                      .withoutSeparator(',')
                      .withSeparator(';')
                      .withUsingEmptyLines()) == 
          TestStreamResult({ 
              { makeCell("a", 0,0) },
              {},
              {},
              { makeCell("b", 3,0) },
              {}},5,0));

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
              { makeCell("a", 0,0), makeCell("",0,4) },
              {},
              { makeCell("",  2,0), makeCell("",2,1), makeCell("",2,2) },
              { makeCell("",  3,0), makeCell("",3,1), makeCell("",3,2) },
              { makeCell("b", 4,0), makeCell("",4,2) },
              {}}, 6,0));
}

TEST_CASE("ReadOneRowWithMultipleSeparators", "[csv_reader]")
{
  REQUIRE(parseStream(" \n abc ,; def;ghi,1  \n\n",
                      csv::Specification()
                       .withSeparator(',')
                       .withSeparator(';')) ==
          TestStreamResult({ 
              { makeCell("abc", 1,1), 
                makeCell("",    1,6),
                makeCell("def", 1,8),
                makeCell("ghi", 1,12),
                makeCell("1", 1,16) }},3,0));
}


TEST_CASE("ReadWhiteSpaceAsSeparator", "[csv_reader]")
{
  auto spec = csv::Specification()
    .withoutSeparator()
    .withSeparator(' ')
    .withSeparator('\t');

  REQUIRE(parseStream("  a  ", spec) == 
          TestStreamResult({ 
              { makeCell("a", 0,2)}}, 0,5));
  REQUIRE(parseStream("  \"a\"  ", spec) == 
          TestStreamResult({ 
              { makeCell("a", 0,2)}}, 0,7));
  REQUIRE(parseStream("a b", spec) == 
          TestStreamResult({ 
              { makeCell("a", 0,0),
                makeCell("b", 0,2) }}, 0,3));
  REQUIRE(parseStream("\"a\" \"b\"", spec) == 
          TestStreamResult({ 
              { makeCell("a", 0,0),
                makeCell("b", 0,4) }}, 0,7));
  REQUIRE(parseStream("  a   b  ", spec) == 
          TestStreamResult({ 
              { makeCell("a", 0,2),
                makeCell("b", 0,6) }}, 0,9));
  REQUIRE(parseStream("  \"a b \"  ", spec) == 
          TestStreamResult({ 
              { makeCell("a b ", 0,2) }},0,10));
  REQUIRE(parseStream("\"1\" 2 3\n"
                      "  \"1\"   2   3 \n"
                      "1 \"2\" 3\r\n"
                      "1  \"2\"  3 \r\n"
                      "1 2 \"3\"\r\n"
                      "1 2 \"3\"    \n", spec) == TestStreamResult({ 
  { makeCell("1", 0,0),makeCell("2", 0,4),makeCell("3", 0,6)},
  { makeCell("1", 1,2),makeCell("2", 1,8),makeCell("3", 1,12)},
  { makeCell("1", 2,0),makeCell("2", 2,2),makeCell("3", 2,6)},
  { makeCell("1", 3,0),makeCell("2", 3,3),makeCell("3", 3,8)},
  { makeCell("1", 4,0),makeCell("2", 4,2),makeCell("3", 4,4)},
  { makeCell("1", 5,0),makeCell("2", 5,2),makeCell("3", 5,4)}},6,0));
}

TEST_CASE("ReadWhiteSpaceAsSeparatorAndUseEmptyLines", "[csv_reader]")
{

  auto spec = 
    csv::Specification()
     .withoutSeparator()
     .withSeparator(' ')
     .withSeparator('\t')
     .withUsingEmptyLines();
  REQUIRE(parseStream("\"1\" 2 3\n  \t  \n"
                      "  \"1\"   2   3 \n"
                      "1 \"2\" 3\r\n"
                      "1  \"2\"  3 \r\n"
                      "1 2 \"3\"\r\n"
                      "1 2 \"3\"    \n\r\n", spec) == TestStreamResult({ 
  { makeCell("1", 0,0),makeCell("2", 0,4),makeCell("3", 0,6)},
  {},
  { makeCell("1", 2,2),makeCell("2", 2,8),makeCell("3", 2,12)},
  { makeCell("1", 3,0),makeCell("2", 3,2),makeCell("3", 3,6)},
  { makeCell("1", 4,0),makeCell("2", 4,3),makeCell("3", 4,8)},
  { makeCell("1", 5,0),makeCell("2", 5,2),makeCell("3", 5,4)},
  { makeCell("1", 6,0),makeCell("2", 6,2),makeCell("3", 6,4)},
  {}},8,0));
}


TEST_CASE("ReadCharWithCommentSymbol", "[csv_reader]")
{
  auto spec = csv::Specification()
    .withComment('#');

  REQUIRE(parseStream("   #",   spec) == TestStreamResult(0,4));

  REQUIRE(parseStream(" \"#\"", spec) ==
          TestStreamResult({ 
              { makeCell("#", 0,1) }},0,4));

  REQUIRE(parseStream("   #\na",spec) ==
          TestStreamResult({ 
              { makeCell("a", 1,0) }},1,1));

  REQUIRE(parseStream("  a#,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
          { makeCell("a", 0,2) },
          { makeCell("1", 1,0), 
            makeCell("2", 1,2), 
            makeCell("3", 1,4)}},1,5));

  REQUIRE(parseStream("  a #,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
          { makeCell("a", 0,2) },
          { makeCell("1", 1,0), 
            makeCell("2", 1,2), 
            makeCell("3", 1,4)}},1,5));

  REQUIRE(parseStream("  \"a\"#,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
          { makeCell("a", 0,2) },
          { makeCell("1", 1,0), 
            makeCell("2", 1,2), 
            makeCell("3", 1,4)}},1,5));

  REQUIRE(parseStream("  \"a\" #,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
          { makeCell("a", 0,2) },
          { makeCell("1", 1,0), 
            makeCell("2", 1,2), 
            makeCell("3", 1,4)}},1,5));

  REQUIRE(parseStream("  \"a\" , #,b,c,d\n1,2,3",spec) ==
          TestStreamResult({ 
          { makeCell("a", 0,2),makeCell("", 0,7) },
          { makeCell("1", 1,0), 
            makeCell("2", 1,2), 
            makeCell("3", 1,4)}},1,5));


  auto spec2 = csv::Specification()
    .withoutSeparator()
    .withSeparator(' ')
    .withSeparator('\t')
    .withComment('#');
  REQUIRE(parseStream("  \"a\" # b c d\n1 2 3",spec2) ==
          TestStreamResult({ 
          { makeCell("a", 0,2) },
          { makeCell("1", 1,0), 
            makeCell("2", 1,2), 
            makeCell("3", 1,4)}},1,5));
  
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
  auto row1 = *itr; ++itr;
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
    REQUIRE(ex.inputLine()     == 0u);
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

TEST_CASE("UnexpectedEndOfFileThrows", "[csv_reader]")
{
  bool caught = false;
  try 
  {
    parseStream(",\"abc");
  }
  catch(csv::ParseError ex)
  {
    REQUIRE(ex.inputLine()   == 0);
    REQUIRE(ex.inputColumn() == 5);
    REQUIRE(ex.row()         == 0);
    REQUIRE(ex.column()      == 1);
    caught = true;
  }
  REQUIRE(caught);
}

TEST_CASE("CharacterAfterWhiteSpaceThrows", "[csv_reader]")
{
  bool caught = false;
  try 
  {
    parseStream("a\nb\n\"c1\",\"c2\" x,c3");
  }
  catch(csv::ParseError ex)
  {
    REQUIRE(ex.inputLine()   == 2);
    REQUIRE(ex.inputColumn() == 10);
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
    REQUIRE(ex.inputColumn() == 0);
    REQUIRE(ex.row() == 0);
    REQUIRE(ex.column() == 0);
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
    REQUIRE(ex.inputColumn() == 0);
    REQUIRE(ex.row() == 1);
    REQUIRE(ex.column() == 0);
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
    REQUIRE(ex.inputLine() == 2);
    REQUIRE(ex.inputColumn() == 0);
    REQUIRE(ex.row() == 2);
    REQUIRE(ex.column() == 0);
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
    REQUIRE(ex.inputColumn() == 0);
    REQUIRE(ex.row()         == 1);
    REQUIRE(ex.column()      == 0);
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
    REQUIRE(ex.inputColumn() == 0);
    REQUIRE(ex.row()         == 2);
    REQUIRE(ex.column()      == 0);
    caught2 = true;
  }
  REQUIRE(caught1);
  REQUIRE(caught2);
}

