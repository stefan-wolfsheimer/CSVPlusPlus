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
#include <csv/row.h>

csv::Row createEmptyRow()
{
  return csv::Row();
};

TEST_CASE("EmptyRowIteratorTest", "[csv_row]")
{
  csv::Row empty_row(std::vector<std::string>{});
  REQUIRE       ( empty_row.size()   == 0u );
  REQUIRE       ( empty_row.begin()  == empty_row.end()  );  
  REQUIRE       ( empty_row.begin()  == empty_row.cend() );  
  REQUIRE       ( empty_row.cbegin() == empty_row.end()  );  
  REQUIRE       ( empty_row.cbegin() == empty_row.cend() );  

  REQUIRE_FALSE ( empty_row.begin()  != empty_row.end()  );  
  REQUIRE_FALSE ( empty_row.begin()  != empty_row.cend() );  
  REQUIRE_FALSE ( empty_row.cbegin() != empty_row.end()  );  
  REQUIRE_FALSE ( empty_row.cbegin() != empty_row.cend() );  

  REQUIRE       ( empty_row.begin()  <= empty_row.end()  );  
  REQUIRE       ( empty_row.begin()  <= empty_row.cend() );  
  REQUIRE       ( empty_row.cbegin() <= empty_row.end()  );  
  REQUIRE       ( empty_row.cbegin() <= empty_row.cend() );  

  REQUIRE       ( empty_row.begin()  >= empty_row.end()  );  
  REQUIRE       ( empty_row.begin()  >= empty_row.cend() );  
  REQUIRE       ( empty_row.cbegin() >= empty_row.end()  );  
  REQUIRE       ( empty_row.cbegin() >= empty_row.cend() );  

  REQUIRE_FALSE ( empty_row.begin()  <  empty_row.end()  );  
  REQUIRE_FALSE ( empty_row.begin()  <  empty_row.cend() );  
  REQUIRE_FALSE ( empty_row.cbegin() <  empty_row.end()  );  
  REQUIRE_FALSE ( empty_row.cbegin() <  empty_row.cend() );  

  REQUIRE_FALSE ( empty_row.begin()  >  empty_row.end()  );  
  REQUIRE_FALSE ( empty_row.begin()  >  empty_row.cend() );  
  REQUIRE_FALSE ( empty_row.cbegin() >  empty_row.end()  );  
  REQUIRE_FALSE ( empty_row.cbegin() >  empty_row.cend() );  

  // test assignment operator
  csv::Row empty_row2;
  empty_row2 = empty_row;
  REQUIRE       ( empty_row2.begin()  == empty_row2.end()  );  



  // test copy constructor
  csv::Row empty_row3(empty_row);
  REQUIRE       ( empty_row3.begin()  == empty_row3.end()  );  

  csv::Row empty_row4(std::move(empty_row));
  REQUIRE       ( empty_row4.begin()  == empty_row4.end()  );  
}

TEST_CASE("OneColumnRow" ,"[csv_row]")
{
  csv::Row row(std::vector<std::string>{"abc"});
  REQUIRE       ( row.size()   == 1u );

  REQUIRE_FALSE ( row.begin()  == row.end()  );  
  REQUIRE_FALSE ( row.begin()  == row.cend() );  
  REQUIRE_FALSE ( row.cbegin() == row.end()  );  
  REQUIRE_FALSE ( row.cbegin() == row.cend() );  

  REQUIRE       ( row.begin()  != row.end()  );  
  REQUIRE       ( row.begin()  != row.cend() );  
  REQUIRE       ( row.cbegin() != row.end()  );  
  REQUIRE       ( row.cbegin() != row.cend() );  

  REQUIRE       ( row.begin()  <= row.end()  );  
  REQUIRE       ( row.begin()  <= row.cend() );  
  REQUIRE       ( row.cbegin() <= row.end()  );  
  REQUIRE       ( row.cbegin() <= row.cend() );  

  REQUIRE_FALSE ( row.begin()  >= row.end()  );  
  REQUIRE_FALSE ( row.begin()  >= row.cend() );  
  REQUIRE_FALSE ( row.cbegin() >= row.end()  );  
  REQUIRE_FALSE ( row.cbegin() >= row.cend() );  

  REQUIRE       ( row.begin()  <  row.end()  );  
  REQUIRE       ( row.begin()  <  row.cend() );  
  REQUIRE       ( row.cbegin() <  row.end()  );  
  REQUIRE       ( row.cbegin() <  row.cend() );  

  REQUIRE_FALSE ( row.begin()  >  row.end()  );  
  REQUIRE_FALSE ( row.begin()  >  row.cend() );  
  REQUIRE_FALSE ( row.cbegin() >  row.end()  );  
  REQUIRE_FALSE ( row.cbegin() >  row.cend() );  

  REQUIRE_FALSE ( row.begin()  == row.end()  );  
  REQUIRE_FALSE ( row.begin()  == row.cend() );  
  REQUIRE_FALSE ( row.cbegin() == row.end()  );  
  REQUIRE_FALSE ( row.cbegin() == row.cend() );  

  REQUIRE       ( row.begin()  != row.end()  );  
  REQUIRE       ( row.begin()  != row.cend() );  
  REQUIRE       ( row.cbegin() != row.end()  );  
  REQUIRE       ( row.cbegin() != row.cend() );  

  REQUIRE       ( row.begin()  <= row.end()  );  
  REQUIRE       ( row.begin()  <= row.cend() );  
  REQUIRE       ( row.cbegin() <= row.end()  );  
  REQUIRE       ( row.cbegin() <= row.cend() );  

  REQUIRE_FALSE ( row.rbegin()  >= row.rend()  );  
  REQUIRE_FALSE ( row.rbegin()  >= row.crend() );  
  REQUIRE_FALSE ( row.crbegin() >= row.rend()  );  
  REQUIRE_FALSE ( row.crbegin() >= row.crend() );  

  REQUIRE       ( row.rbegin()  <  row.rend()  );  
  REQUIRE       ( row.rbegin()  <  row.crend() );  
  REQUIRE       ( row.crbegin() <  row.rend()  );  
  REQUIRE       ( row.crbegin() <  row.crend() );  

  REQUIRE_FALSE ( row.rbegin()  >  row.rend()  );  
  REQUIRE_FALSE ( row.rbegin()  >  row.crend() );  
  REQUIRE_FALSE ( row.crbegin() >  row.rend()  );  
  REQUIRE_FALSE ( row.crbegin() >  row.crend() );  

  {
    auto itr = row.begin();
    REQUIRE ( ++itr == row.end() );
    REQUIRE (   itr == row.end() );
    REQUIRE ( --itr == row.begin() );
    REQUIRE (   itr == row.begin() );
  }
  {
    auto itr = row.begin();
    REQUIRE ( itr++ == row.begin() );
    REQUIRE ( itr   == row.end() );
    REQUIRE ( itr-- == row.end() );
    REQUIRE ( itr   == row.begin() );
  }
  {
    auto itr = row.rbegin();
    REQUIRE ( ++itr == row.rend() );
    REQUIRE (   itr == row.rend() );
    REQUIRE ( --itr == row.rbegin() );
    REQUIRE (   itr == row.rbegin() );
  }
  {
    auto itr = row.rbegin();
    REQUIRE ( itr++ == row.rbegin() );
    REQUIRE ( itr   == row.rend() );
    REQUIRE ( itr-- == row.rend() );
    REQUIRE ( itr   == row.rbegin() );
  }

  REQUIRE("abc" == row.begin()->as<std::string>());
  REQUIRE("abc" == row.rbegin()->as<std::string>());
  REQUIRE("abc" == row[0].as<std::string>());
  csv::Cell cell = *row.begin();
  REQUIRE("abc" == cell.as<std::string>());
  const csv::Cell * cell_ptr = &*row.begin();
  REQUIRE("abc" == cell_ptr->as<std::string>());
  
}

TEST_CASE("OneColumnRowFromCString","[csv_row]")
{
  const char * myrow[] = { "abc" };
  csv::Row row(myrow);  
  REQUIRE       ( row.size()   == 1u );
  REQUIRE_FALSE ( row.begin()  == row.end()  );  
  REQUIRE       ( "abc" == row.begin()->as<std::string>());
}

TEST_CASE("BufferIsShared", "[csv_row]")
{
  csv::Cell cell;
  {
    csv::Row row(std::vector<std::string>{"123"});
    cell = *row.begin();
  }
  REQUIRE( cell.as<int>() == 123);
}


TEST_CASE("ReadRowWithHeaderAndAssignNamesToCells", "[csv_row]")
{
  auto spec = csv::Specification()
    .withColumn(0,"first")
    .withColumn(1,"second")
    .withColumn(2,"third")
    .withColumn(5,"sixth");
  csv::Row row(std::vector<std::string>{
      "0", "1", "2", "3", "4", "5", "6" }, spec);  

  for(int i = 0; i < 7; i++) 
  {
    REQUIRE(row[i].as<int>() == i);
  }
  REQUIRE(row[0].name() == "first");
  REQUIRE(row[1].name() == "second");
  REQUIRE(row[2].name() == "third");
  REQUIRE(row[3].name() == "");
  REQUIRE(row[4].name() == "");
  REQUIRE(row[5].name() == "sixth");
  REQUIRE(row[6].name() == "");
  bool caught = false;
  try
  {
    row[7];
  }
  catch(csv::CellOutOfRangeError ex) 
  {
    caught = true;
    REQUIRE(ex.size()        == 7);
    REQUIRE(ex.index()       == 7);
    REQUIRE(ex.row()         == 0);
    REQUIRE(ex.column()      == 0);
    REQUIRE(ex.inputLine()   == 0);
    REQUIRE(ex.inputColumn() == 0);
  }
  REQUIRE(caught);

  REQUIRE_THROWS(row[""]);
  REQUIRE(row["first"].as<int>()  == 0);
  REQUIRE(row["second"].as<int>() == 1);
  REQUIRE(row["third"].as<int>()  == 2);
  REQUIRE(row["sixth"].as<int>()  == 5);
}
