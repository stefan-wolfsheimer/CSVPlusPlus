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
#pragma once

#include <type_traits>
#include <exception>
#include <string>
#include <typeindex>

namespace csv
{
  typedef ::std::char_traits<char> char_traits;
  typedef ::std::char_traits<wchar_t> wchar_traits;

  template<typename CHAR=char, typename TRAITS=::std::char_traits<CHAR> >
  class BasicSpecification;

  template<typename CHAR=char, typename TRAITS=::std::char_traits<CHAR> >
  class BasicCell;

  template<typename CHAR=char, typename TRAITS=::std::char_traits<CHAR> >
  class BasicRow;

  template<typename CHAR=char, typename TRAITS=::std::char_traits<CHAR> >
  class BasicReader;

  template<typename CLASS, typename CHAR=char,
           typename TRAITS=::std::char_traits<CHAR> >
  class BasicObjectReader;

  typedef BasicSpecification<char, char_traits> Specification;
  typedef BasicCell<char, char_traits> Cell;
  typedef BasicRow<char, char_traits> Row;
  typedef BasicReader<char, char_traits> Reader;
  typedef BasicSpecification<wchar_t, wchar_traits> WSpecification;
  typedef BasicCell<wchar_t, wchar_traits> WCell;
  typedef BasicRow<wchar_t, wchar_traits> WRow;
  typedef BasicReader<wchar_t, wchar_traits> WReader;

  class CsvException : public ::std::exception
  {
  public:
    CsvException( const ::std::string & message,
                  ::std::size_t         input_line,
                  ::std::size_t         input_column,
                  ::std::size_t         csv_row,
                  ::std::size_t         csv_column);
    const char * what() const noexcept (true) override
    {
      return _full_message.c_str();
    }

    inline ::std::size_t inputLine() const;
    inline ::std::size_t inputColumn() const;
    inline ::std::size_t row() const;
    inline ::std::size_t column() const;

  private:
    ::std::string _message;
    ::std::string _full_message;
    ::std::size_t _input_line;
    ::std::size_t _input_column;
    ::std::size_t _csv_row;
    ::std::size_t _csv_column;
  };

  class ParseError : public CsvException
  { 
  public:
    ParseError( const ::std::string & message,
                ::std::size_t         input_line,
                ::std::size_t         input_column,
                ::std::size_t         csv_row,
                ::std::size_t         csv_column);
  };


  class DuplicateColumnError : public CsvException
  {
  public:
    DuplicateColumnError( const ::std::string & message,
                          ::std::size_t         index,
                          ::std::size_t         input_line,
                          ::std::size_t         input_column,
                          ::std::size_t         csv_row,
                          ::std::size_t         csv_column);
    inline ::std::size_t index() const;
  private:
    ::std::size_t _index;
  };


  class CellAccessViolationError : public CsvException
  {
  public:
    CellAccessViolationError( const ::std::string & message,
                              ::std::size_t         input_line,
                              ::std::size_t         input_column,
                              ::std::size_t         csv_row,
                              ::std::size_t         csv_column);
  };


  class CellOutOfRangeError : public CellAccessViolationError
  {
  public:
    CellOutOfRangeError( const ::std::string & message,
                         ::std::size_t         index,
                         ::std::size_t         size,
                         ::std::size_t         input_line,
                         ::std::size_t         input_column,
                         ::std::size_t         csv_row,
                         ::std::size_t         csv_column);
    inline ::std::size_t index() const;
    inline ::std::size_t size()  const;
  private:
    ::std::size_t _index;
    ::std::size_t _size;
  };

  class UndefinedColumnError : public CellAccessViolationError
  {
  public:
    UndefinedColumnError( const ::std::string & message,
                          ::std::size_t         size,
                          ::std::size_t         input_line,
                          ::std::size_t         input_column,
                          ::std::size_t         csv_row,
                          ::std::size_t         csv_column);
    inline ::std::size_t       size()  const;
  private:
    ::std::size_t _size;
  };

  class DefinedCellOutOfRangeError : public CellAccessViolationError
  {
  public:
    DefinedCellOutOfRangeError( const ::std::string & message,
                                ::std::size_t         index,
                                ::std::size_t         size,
                                ::std::size_t         input_line,
                                ::std::size_t         input_column,
                                ::std::size_t         csv_row,
                                ::std::size_t         csv_column);
    inline ::std::size_t index() const;
    inline ::std::size_t size()  const;
  private:
    ::std::size_t _index;
    ::std::size_t _size;
  };

  class ConversionError : public CsvException
  {
  public:
    
    ConversionError( const ::std::string   & message,
                     const std::type_index & type,
                     ::std::size_t           input_line,
                     ::std::size_t           input_column,
                     ::std::size_t           csv_row,
                     ::std::size_t           csv_column);
    inline const ::std::type_index & typeIndex() const;

  private:
    ::std::type_index _type_index;
  };

  ////////////////////////////////////////////////////////////////////////////
  //
  // Implementation
  //
  ////////////////////////////////////////////////////////////////////////////
  inline CsvException::CsvException ( const ::std::string & message,
                               ::std::size_t         input_line,
                               ::std::size_t         input_column,
                               ::std::size_t         csv_row,
                               ::std::size_t         csv_column ) 
    : _message(message),
      _input_line(input_line),
      _input_column(input_column),
      _csv_row(csv_row),
      _csv_column(csv_column)
    {
      _full_message = 
        _message + 
        ::std::string(" at position: ") + 
        ::std::to_string(input_line) + 
        ::std::string(":")  + 
        ::std::to_string(input_column) +
        ::std::string(" (input stream ) ") + 
        ::std::to_string(csv_row) + 
        ::std::string(":") + 
        ::std::to_string(csv_column) +
        ::std::string(" (CSV cell) ");
    }
  

  ::std::size_t CsvException::inputLine() const 
  {
    return _input_line;
  }

  ::std::size_t CsvException::inputColumn() const 
  {
    return _input_column;
  }

  ::std::size_t CsvException::row() const 
  {
    return _csv_row;
  }
  
  ::std::size_t CsvException::column() const 
  {
    return _csv_column;
  }




  inline ParseError::ParseError( const ::std::string & message,
                                 ::std::size_t         input_line,
                                 ::std::size_t         input_column,
                                 ::std::size_t         csv_row,
                                 ::std::size_t         csv_column) 
    : CsvException(message, input_line, 
                   input_column, csv_row, csv_column) {}


  inline 
  DuplicateColumnError::DuplicateColumnError( const ::std::string & message,
                                              ::std::size_t         index,
                                              ::std::size_t         input_line,
                                              ::std::size_t         input_col,
                                              ::std::size_t         csv_row,
                                              ::std::size_t         csv_column) 
  : CsvException(message, input_line, 
                 input_col, csv_row, csv_column),
    _index(index)
  {}

  ::std::size_t DuplicateColumnError::index() const 
  {
    return _index;
  }
    

  inline CellAccessViolationError::
  CellAccessViolationError( const ::std::string & message,
                            ::std::size_t         input_line,
                            ::std::size_t         input_column,
                            ::std::size_t         csv_row,
                            ::std::size_t         csv_column) 
    : CsvException(message, input_line, 
                   input_column, csv_row, csv_column) {}
  


  inline CellOutOfRangeError::
  CellOutOfRangeError( const ::std::string & message,
                       ::std::size_t         index,
                       ::std::size_t         size,
                       ::std::size_t         input_line,
                       ::std::size_t         input_column,
                       ::std::size_t         csv_row,
                       ::std::size_t         csv_column) 
      : CellAccessViolationError(message, 
                                 input_line, 
                                 input_column, 
                                 csv_row, 
                                 csv_column),
    _index(index), _size(size) {}

  inline ::std::size_t CellOutOfRangeError::index() const 
  {
    return _index;
  }

  inline ::std::size_t CellOutOfRangeError::size() const 
  {
    return _size;
  }


  inline UndefinedColumnError::
  UndefinedColumnError( const ::std::string & message,
                        ::std::size_t         size,
                        ::std::size_t         input_line,
                        ::std::size_t         input_column,
                        ::std::size_t         csv_row,
                        ::std::size_t         csv_column) 
    : CellAccessViolationError(message, 
                               input_line, 
                               input_column, 
                               csv_row, 
                               csv_column),
    _size(size) {}
  

  inline ::std::size_t UndefinedColumnError::size() const
  {
    return _size;
  }



  inline DefinedCellOutOfRangeError::
  DefinedCellOutOfRangeError( const ::std::string & message,
                              ::std::size_t         index,
                              ::std::size_t         size,
                              ::std::size_t         input_line,
                              ::std::size_t         input_column,
                              ::std::size_t         csv_row,
                              ::std::size_t         csv_column) 
  : CellAccessViolationError(message, 
                             input_line, 
                             input_column, 
                             csv_row, 
                             csv_column),
    _index(index), _size(size) {}
  
    

  inline ::std::size_t DefinedCellOutOfRangeError::index() const
  {
    return _index;
  }

  inline ::std::size_t DefinedCellOutOfRangeError::size() const
  {
    return _size;
  }


  inline ConversionError::ConversionError( const ::std::string   & message,
                                           const std::type_index & type,
                                           ::std::size_t           input_line,
                                           ::std::size_t           input_column,
                                           ::std::size_t           csv_row,
                                           ::std::size_t           csv_column) 
    : CsvException(message, 
                   input_line,
                   input_column,
                   csv_row,
                   csv_column),
    _type_index(type)
  {}

  const ::std::type_index& ConversionError::typeIndex() const
  {
    return _type_index;
  }

} // namespace
