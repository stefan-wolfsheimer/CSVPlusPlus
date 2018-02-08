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
#include <csv/specification.h>
#include <locale>


template<typename CharT>
class DecimalSeparator : public std::numpunct<CharT>
{
public:
    DecimalSeparator(CharT Separator)
    : m_Separator(Separator)
    {}

protected:
    CharT do_decimal_point()const
    {
        return m_Separator;
    }

private:
    CharT m_Separator;
};

TEST_CASE("CreateDefaultSpecification", "[csv_specification]")
{
  csv::Specification spec;
  REQUIRE_FALSE( spec.hasHeader());
  REQUIRE_FALSE( spec.hasUsingEmptyLines());
  REQUIRE      ( spec.isSeparator(','));
  REQUIRE      ( spec.defaultSeparator() == ',');
  REQUIRE_FALSE( spec.isSeparator(' '));
  REQUIRE_FALSE( spec.isSeparator(';'));
  REQUIRE_FALSE( spec.isSeparator('\t'));
  REQUIRE_FALSE( spec.isSeparator('a'));
  REQUIRE      ( spec.isComment('\0'));
  REQUIRE_FALSE( spec.isComment('#'));  

}

TEST_CASE("CreateDefaultWcharSpecification", "[csv_specification]")
{
  csv::WSpecification spec;
  REQUIRE_FALSE( spec.hasHeader());
  REQUIRE_FALSE( spec.hasUsingEmptyLines());
  REQUIRE      ( spec.isSeparator(','));
  REQUIRE_FALSE( spec.isSeparator(' '));
  REQUIRE_FALSE( spec.isSeparator(';'));
  REQUIRE_FALSE( spec.isSeparator('\t'));
  REQUIRE_FALSE( spec.isSeparator('a'));
  REQUIRE      ( spec.isComment('\0'));
  REQUIRE_FALSE( spec.isComment('#'));  
}

TEST_CASE("ConstructionWithHeader", "[csv_specification]")
{
  REQUIRE(csv::Specification()
          .withHeader()
          .hasHeader());
  REQUIRE_FALSE(csv::Specification()
                .withHeader()
                .withoutHeader()
                .hasHeader());
  REQUIRE_FALSE(csv::Specification()
                .withoutHeader()
                .hasHeader());
}

TEST_CASE("ConstructionWithUsingEmptyLines", "[csv_specification]")
{
  REQUIRE(csv::Specification()
          .withUsingEmptyLines()
          .hasUsingEmptyLines());
  REQUIRE_FALSE(csv::Specification()
                .withUsingEmptyLines()
                .withoutUsingEmptyLines()
                .hasUsingEmptyLines());
  REQUIRE_FALSE(csv::Specification()
                .withoutUsingEmptyLines()
                .hasUsingEmptyLines());
}

TEST_CASE("ConstructionWithComment", "[csv_specification]")
{
  REQUIRE(csv::Specification()
          .withComment('#')
          .isComment('#'));

  REQUIRE_FALSE(csv::Specification()
                .withComment('#')
                .withoutComment()
                .isComment('#'));
  REQUIRE_FALSE(csv::Specification()
                .withoutComment()
                .isComment('#'));
}

TEST_CASE("ConstructionWithLocale", "[csv_specification]")
{

  auto spec = csv::Specification()
    .withLocale(std::locale(std::locale(),
                            new DecimalSeparator<char>('@')));
  std::ostringstream s;
  s.imbue(spec.locale());
  s << double(1.234);
  REQUIRE(s.str() == "1@234");
}

TEST_CASE("ConstructionWithDecimalSeparator", "[csv_specification]")
{

  auto spec = csv::Specification()
    .withDecimalSeparator(',');
  std::ostringstream s;
  s.imbue(spec.locale());
  s << double(1.234);
  REQUIRE(s.str() == "1,234");
}

TEST_CASE("ConstructionWithoutSeparator", "[csv_specification]")
{
  // no separator
  csv::Specification no_sep = csv::Specification().withoutSeparator(',');
  REQUIRE      ( no_sep.defaultSeparator() == '\0');
  REQUIRE_FALSE( no_sep.isSeparator(','));
  REQUIRE_FALSE( no_sep.isSeparator(';'));
}

TEST_CASE("ConstructionWithoutAllSeparators", "[csv_specification]")
{
  // no separator
  csv::Specification no_sep = csv::Specification().withoutSeparator();
  REQUIRE      ( no_sep.defaultSeparator() == '\0');
  REQUIRE_FALSE( no_sep.isSeparator(','));
  REQUIRE_FALSE( no_sep.isSeparator(';'));
}

TEST_CASE("ConstructionWithCommaSeparator", "[csv_specification]")
{
  csv::Specification comma_sep = csv::Specification().withSeparator(',');
  REQUIRE      ( comma_sep.defaultSeparator() == ',');
  REQUIRE      ( comma_sep.isSeparator(','));
  REQUIRE_FALSE( comma_sep.isSeparator(';'));
}

TEST_CASE("ConstructionWithSemicolonSeparator", "[csv_specification]")
{
  csv::Specification semicolon_sep = csv::Specification()
    .withoutSeparator(',')
    .withSeparator(';');
  REQUIRE      (  semicolon_sep.defaultSeparator() == ';');
  REQUIRE_FALSE(  semicolon_sep.isSeparator(','));
  REQUIRE      (  semicolon_sep.isSeparator(';'));
}

TEST_CASE("ConstructionWithMultipleSeparators", "[csv_specification]")
{
  csv::Specification multi_sep = csv::Specification()
    .withoutSeparator() //clear all separators
    .withSeparator(';')
    .withSeparator(' ')
    .withSeparator('\t');
  REQUIRE      ( multi_sep.defaultSeparator() == ';');
  REQUIRE_FALSE( multi_sep.isSeparator(','));
  REQUIRE(       multi_sep.isSeparator(';'));
  REQUIRE(       multi_sep.isSeparator(' '));
  REQUIRE(       multi_sep.isSeparator('\t'));
}

TEST_CASE("ConstructionSetAndUnsetSeparator", "[csv_specification]")
{
  REQUIRE_FALSE(csv::Specification()
                .withSeparator(';')
                .withSeparator(';')
                .withoutSeparator(';')
                .isSeparator(';'));

  REQUIRE( csv::Specification()
                .withSeparator(';')
                .withSeparator(';')
                .withoutSeparator(';')
                .defaultSeparator() == ',');
}

TEST_CASE("ConstructionSetAndUnsetAllSeparators", "[csv_specification]")
{
  REQUIRE_FALSE(csv::Specification()
                .withSeparator(';')
                .withSeparator(';')
                .withoutSeparator()
                .isSeparator(';'));
  REQUIRE( csv::Specification()
            .withSeparator(';')
            .withSeparator(';')
            .withoutSeparator()
            .defaultSeparator() == '\0');
}

TEST_CASE("ConstructionWithColumns", "[csv_specification]")
{
  auto spec = csv::Specification()
    .withColumn(0,"first")
    .withColumn(1,"second")
    .withColumn(2,"third");
}

TEST_CASE("ConstructionWithDuplicateColumnNames", "[csv_specification]")
{
  bool caught = false;
  try
  {
    csv::Specification()
      .withColumn(0,"first")
      .withColumn(1,"second")
      .withColumn(2,"first")
      .withColumn(3,"third");
  }
  catch( csv::DuplicateColumnError ex)
  {
    REQUIRE(ex.index() == 0u);
    caught = true;
  }
  REQUIRE(caught);
}

TEST_CASE("ConstructionWithDuplicateColumnIndex", "[csv_specification]")
{
  bool caught = false;
  try
  {
    auto spec = csv::Specification()
      .withColumn(0,"first")
      .withColumn(1,"second")
      .withColumn(1,"third");
  }
  catch(csv::DuplicateColumnError ex)
  {
    REQUIRE(ex.index() == 1u);
    caught = true;
  }
  REQUIRE(caught);
}

