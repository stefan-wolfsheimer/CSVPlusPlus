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
#include <csv/cell.h>
#include <iostream>
#include <locale>

typedef csv::BasicCell<char>         cell_t;
typedef csv::BasicCell<char16_t>     cell16_t;
typedef csv::BasicCell<char32_t>     cell32_t;
typedef csv::BasicCell<wchar_t>      wcell_t;
typedef std::basic_string<char>      string_t;

typedef std::basic_string<char32_t>  string32_t;

#define CONVERT_FROM_U32 0
#define CONVERT_FROM_U16 0

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

TEST_CASE("ConvertFromString", "[csv_cell]")
{
  REQUIRE(" abcde " == cell_t(" abcde ")       .as<std::string>());
  REQUIRE(123       == cell_t("123")           .as<int>());
  REQUIRE(123       == cell_t("123   ")        .as<int>());
  REQUIRE(12.3      == cell_t("  12.3  ")      .as<double>());
  REQUIRE_THROWS_AS( cell_t("xxx")             .as<int>(), 
                     csv::ConversionError);
  REQUIRE_THROWS_AS( cell_t("12ff")            .as<int>(), 
                     csv::ConversionError);

  REQUIRE(u8" abcde " == cell_t(u8" abcde ")   .as<std::string>());
  REQUIRE(123         == cell_t(u8"123")       .as<int>());
  REQUIRE(123         == cell_t(u8"123   ")    .as<int>());
  REQUIRE(12.3        == cell_t(u8"  12.3  ")  .as<double>());
  REQUIRE_THROWS_AS( cell_t(u8"xxx")           .as<int>(),
                     csv::ConversionError);
  REQUIRE_THROWS_AS( cell_t(u8"12ff")          .as<int>(),
                     csv::ConversionError);
}

TEST_CASE("ConvertFromStringWithLocale", "[csv_cell]")
{
  auto spec = csv::Specification()
    .withLocale(std::locale(std::locale(),
                            new DecimalSeparator<char>('@')));

  REQUIRE(" abcde " == cell_t(spec, " abcde ")       .as<std::string>());
  REQUIRE(123       == cell_t(spec, "123")           .as<int>());
  REQUIRE(123       == cell_t(spec, "123   ")        .as<int>());
  REQUIRE(12.3      == cell_t(spec, "  12@3 ")       .as<double>());
  REQUIRE_THROWS_AS(cell_t(spec, "  12.3  ")         .as<double>(),
                    csv::ConversionError);
  REQUIRE_THROWS_AS( cell_t(spec, "xxx")             .as<int>(), 
                     csv::ConversionError);
  REQUIRE_THROWS_AS( cell_t(spec, "12ff")            .as<int>(), 
                     csv::ConversionError);

  REQUIRE(u8" abcde " == cell_t(spec, u8" abcde ")   .as<std::string>());
  REQUIRE(123         == cell_t(spec, u8"123")       .as<int>());
  REQUIRE(123         == cell_t(spec, u8"123   ")    .as<int>());
  REQUIRE(12.3        == cell_t(spec, u8"  12@3  ")  .as<double>());
  REQUIRE_THROWS_AS( cell_t(spec, u8"  12.3  ")      .as<double>(),
                     csv::ConversionError);
  REQUIRE_THROWS_AS( cell_t(spec, u8"xxx")           .as<int>(),
                     csv::ConversionError);
  REQUIRE_THROWS_AS( cell_t(spec, u8"12ff")          .as<int>(),
                     csv::ConversionError);
}


TEST_CASE("ConvertFromWString", "[csv_cell]")
{
  REQUIRE(L"abcde" == wcell_t(L"abcde")        .as<std::wstring>());
  REQUIRE(123      == wcell_t(L"123")          .as<int>());
  REQUIRE_THROWS_AS( wcell_t(L"xxx")           .as<int>(),
                     csv::ConversionError);
  REQUIRE_THROWS_AS( wcell_t(L"12ff")          .as<int>(),
                     csv::ConversionError);
  REQUIRE(123     == wcell_t(L"123   ")        .as<int>());
  REQUIRE(12.3    == wcell_t(L"  12.3  ")      .as<double>());

}

TEST_CASE("ConvertFromU16String", "[csv_cell]")
{

  REQUIRE(u"abcde" == cell16_t(u"abcde")       .as<std::u16string>());
#if CONVERT_FROM_U16
  REQUIRE(123      == cell16_t(u"123")         .as<int>());
  REQUIRE(123      == cell16_t(u"123   ")      .as<int>());
  REQUIRE(12.3     == cell16_t(u"  12.3  ")    .as<double>());
  REQUIRE_THROWS_AS( cell16_t(u"xxx")          .as<int>(),
                     csv::ConversionError);
  REQUIRE_THROWS_AS( cell16_t(u"12ff")         .as<int>(),
                     csv::ConversionError);
#endif
}

TEST_CASE("ConvertFromU32String", "[csv_cell]")
{
  REQUIRE(U"abcde" == cell32_t(U"abcde")       .as<std::u32string>());
#if CONVERT_FROM_U32
  REQUIRE(123     == cell32_t(U"123")          .as<int>());
  REQUIRE_THROWS_AS( cell32_t(U"xxx")          .as<int>(),
                     csv::ConversionError);
  REQUIRE_THROWS_AS( cell32_t(U"12ff")         .as<int>(),
                     csv::ConversionError);
  REQUIRE(123     == cell32_t(U"123   ")       .as<int>());
  REQUIRE(12.3    == cell32_t(U"  12.3  ")     .as<double>());
#endif
}

TEST_CASE("CellWithName", "[csv_cell]")
{
  REQUIRE("myname" == cell_t("value","myname").name());
}


TEST_CASE("CellAssignmentName", "[csv_cell]")
{
  cell_t my_cell;
  {
    cell_t tmp("value", "myname");
    my_cell = tmp;
  }
  REQUIRE("myname" == my_cell.name());
  REQUIRE("value" == my_cell.as<::std::string>());
}

TEST_CASE("CellCopyName", "[csv_cell]")
{
  cell_t tmp("value", "myname");
  cell_t my_cell(tmp);
  REQUIRE("myname" == my_cell.name());
  REQUIRE("value" == my_cell.as<::std::string>());
}

TEST_CASE("CellMoveName", "[csv_cell]")
{
  cell_t tmp("value", "myname");
  cell_t my_cell(std::move(tmp));
  REQUIRE("myname" == my_cell.name());
  REQUIRE("value" == my_cell.as<::std::string>());
}






