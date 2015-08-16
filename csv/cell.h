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
#include <sstream>
#include <exception>
#include <iostream>
#include <memory>
#include "csv_common.h"
#include "specification.h"

namespace csv
{
  template<typename CHAR, typename TRAITS, typename TARGET>
  class BasicSerializer 
  {
  public:
    typedef CHAR                                            char_type;
    typedef TRAITS                                          char_traits;
    typedef TARGET                                          return_type;
    typedef std::basic_string<char_type, char_traits>       string_type;
    typedef std::basic_stringstream<char_type, char_traits> stream_type;
    typedef BasicSpecification<char_type, char_traits>      spec_type;    
    typedef std::shared_ptr<spec_type>                      shared_spec_type;
    typedef BasicCell<char_type, char_traits>               cell_type;


    template<typename ITER>
    static return_type as(const cell_type        & cell,
                          ITER                     begin, 
                          ITER                     end);
  };

  template<typename CHAR, typename TRAITS>
  class BasicSerializer<CHAR, TRAITS, std::basic_string<CHAR, TRAITS> >
  {
  public:
    typedef CHAR                                            char_type;
    typedef TRAITS                                          char_traits;
    typedef ::std::basic_string<char_type, char_traits>     string_type;
    typedef BasicSpecification<char_type, char_traits>      spec_type;    
    typedef std::shared_ptr<spec_type>                      shared_spec_type;
    typedef BasicCell<char_type, char_traits>               cell_type;
    
    template<typename ITER>
    static string_type as(const cell_type  & cell,
                          ITER               begin, 
                          ITER               end);
  };

  template<typename CHAR, typename TRAITS>
  class BasicCell
  {
  public:
    typedef CHAR                                       char_type;
    typedef TRAITS                                     char_traits;
    typedef std::basic_string<char_type, char_traits>  string_type;
    typedef BasicSpecification<char_type, char_traits> spec_type;    
    typedef std::shared_ptr<spec_type>                 shared_spec_type;

    BasicCell(spec_type           specs = spec_type());

    BasicCell(spec_type           specs,
              const string_type & str, 
              const string_type & name = string_type());

    BasicCell(const string_type & str, 
              const string_type & name = string_type());

    BasicCell(const BasicCell & rhs);

    template<typename RET> 
    RET as() const;

    inline const string_type & name() const;
    inline ::std::size_t inputColumn() const;
    inline ::std::size_t inputLine() const;
    inline ::std::size_t row() const;
    inline ::std::size_t column() const;
    shared_spec_type specification() const ;

  private:
    friend class BasicRow<char_type, char_traits>;
    friend class BasicReader<char_type, char_traits>;
    struct range_type
    {
      ::std::size_t _begin;
      ::std::size_t _end;
      ::std::size_t _csv_row;
      ::std::size_t _csv_column;
      ::std::size_t _input_line;
      ::std::size_t _input_column;

      range_type(::std::size_t begin,
                 ::std::size_t end,
                 ::std::size_t csv_row      = 0,
                 ::std::size_t csv_column   = 0,
                 ::std::size_t input_line   = 0,
                 ::std::size_t input_column = 0);
    };
    typedef std::vector<char_type>                     buffer_type;
    typedef std::shared_ptr<buffer_type>               shared_buffer_type;
    typedef typename buffer_type::const_iterator       buffer_iterator;
    typedef typename spec_type::Column                 column_type;
    typedef std::shared_ptr<column_type>               shared_column_type;

    shared_spec_type                                   _specs;
    shared_column_type                                 _shared_column;
    shared_buffer_type                                 _shared_buffer;
    range_type                                         _range;

    BasicCell(const shared_spec_type       & specs,
              const shared_column_type     & column_specs,
              const shared_buffer_type     & buffer,
              const range_type             & range );
    static buffer_type * string2buffer(const string_type & str);
  };

  ////////////////////////////////////////////////////////////////////
  // 
  // Implementation
  //
  ////////////////////////////////////////////////////////////////////
  template<typename CHAR, typename TRAITS>
  inline 
  BasicCell<CHAR,TRAITS>::BasicCell(spec_type specs) 
    : _specs(std::make_shared<spec_type>(specs)),
      _shared_column(std::make_shared<column_type>(0,string_type())),
      _shared_buffer(std::make_shared<buffer_type>()),
      _range(range_type(0,0))
  {
  }

  template<typename CHAR, typename TRAITS>
  inline
  BasicCell<CHAR,TRAITS>::BasicCell(spec_type           specs,
                                    const string_type & str, 
                                    const string_type & name) 
    : _specs(std::make_shared<spec_type>(specs)),
      _shared_column(std::make_shared<column_type>(0,name)),
      _shared_buffer(string2buffer(str)),
      _range(range_type(0,_shared_buffer->size()))
  {
  }


  template<typename CHAR, typename TRAITS>
  inline
  BasicCell<CHAR,TRAITS>::BasicCell(const string_type & str, 
                                    const string_type & name) 
    : _specs(std::make_shared<spec_type>()),
      _shared_column(std::make_shared<column_type>(0,name)),
      _shared_buffer(string2buffer(str)),
      _range(range_type(0,_shared_buffer->size()))
    {
    }

  template<typename CHAR, typename TRAITS> 
  inline
  BasicCell<CHAR,TRAITS>::BasicCell(const BasicCell & rhs) 
    : _specs(rhs._specs),
      _shared_column(rhs._shared_column),
      _shared_buffer(rhs._shared_buffer),
      _range(rhs._range) 
    {}

  template<typename CHAR, typename TRAITS> 
  inline
  BasicCell<CHAR,TRAITS>::BasicCell(const shared_spec_type     & specs,
                                    const shared_column_type   & column_specs,
                                    const shared_buffer_type   & buffer,
                                    const range_type           & range )
    : _specs(specs),
      _shared_column(column_specs),
      _shared_buffer(buffer),
      _range(range) 
  {
  }

  template<typename CHAR, typename TRAITS>
  template<typename RET> 
  inline RET 
  BasicCell<CHAR,TRAITS>::as() const 
  {
    return 
      BasicSerializer<char_type, 
                      char_traits,
                      RET>::as(*this, 
                               _shared_buffer->begin() + _range._begin,
                               _shared_buffer->begin() + _range._end );
  }

  template<typename CHAR, typename TRAITS>
  inline const typename BasicCell<CHAR,TRAITS>::string_type & 
  BasicCell<CHAR,TRAITS>::name() const 
  {
    return _shared_column->name();
  }

  template<typename CHAR, typename TRAITS>
  inline ::std::size_t BasicCell<CHAR,TRAITS>::inputColumn() const 
  {
    return _range._input_column;
  }

  template<typename CHAR, typename TRAITS>
  inline ::std::size_t BasicCell<CHAR,TRAITS>::inputLine() const 
  {
    return _range._input_line;
  }

  template<typename CHAR, typename TRAITS>
  inline ::std::size_t BasicCell<CHAR,TRAITS>::row() const
  {
    return _range._csv_row;
  }

  template<typename CHAR, typename TRAITS>
  inline ::std::size_t BasicCell<CHAR,TRAITS>::column() const
  {
    return _range._csv_column;
  }

  template<typename CHAR, typename TRAITS>
  typename BasicCell<CHAR,TRAITS>::shared_spec_type 
  BasicCell<CHAR,TRAITS>::specification() const 
  {
    return _specs;
  }

  template<typename CHAR, typename TRAITS>
  BasicCell<CHAR,TRAITS>::range_type::range_type(::std::size_t begin,
                                                 ::std::size_t end,
                                                 ::std::size_t csv_row,
                                                 ::std::size_t csv_column,
                                                 ::std::size_t input_line,
                                                 ::std::size_t input_column )
        : _begin(begin), 
          _end(end),
          _csv_row(csv_row),
          _csv_column(csv_column),
          _input_line(input_line),
          _input_column(input_column)
      {}

  template<typename CHAR, typename TRAITS>
  typename BasicCell<CHAR,TRAITS>::buffer_type * 
  BasicCell<CHAR,TRAITS>::string2buffer(const string_type & str) 
  {
    buffer_type * ret = new buffer_type;
    ret->reserve(str.size());
    for(auto itr : str) 
    {
      ret->push_back(itr);
    }
    return ret;
  }



  template<typename CHAR, typename TRAITS, typename TARGET>
  template<typename ITER>
  typename BasicSerializer<CHAR, TRAITS, TARGET>::return_type
  BasicSerializer<CHAR, TRAITS, TARGET>::as(const cell_type        & cell,
                                            ITER                     begin, 
                                            ITER                     end)
  {
    stream_type ss(string_type(begin,end));
    return_type value;
    ss.imbue(cell.specification()->locale());
    ss >> value;
    if(ss.fail()) 
    {
      ::std::type_index ti(typeid(TARGET));
      throw 
        ConversionError(::std::string("Cannot convert cell to type ") + 
                        ti.name(),
                        ti,
                        cell.inputLine(),
                        cell.inputColumn(),
                        cell.row(),
                        cell.column());
    }
    while(!ss.eof())
    {
      int ch = ss.get();
      if(ss.eof()) break;
      if(ch != ' ' && ch != '\t') 
      {
        ::std::type_index ti(typeid(TARGET));
        throw 
          ConversionError(::std::string("Cannot convert cell to type ") + 
                          ti.name(),
                          ti,
                          cell.inputLine(),
                          cell.inputColumn(),
                          cell.row(),
                          cell.column());
      }
    }
    return value;
  }

  template<typename CHAR, typename TRAITS>
  template<typename ITER>
  typename 
  BasicSerializer<CHAR, TRAITS, std::basic_string<CHAR, TRAITS> >::string_type
  BasicSerializer<CHAR, TRAITS, std::basic_string<CHAR, TRAITS> >::
  as(const cell_type        & cell,
     ITER                     begin, 
     ITER                     end)
  {
    return string_type(begin,end);
  }

} // namespace 
