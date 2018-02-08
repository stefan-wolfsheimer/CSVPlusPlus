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
#include "csv_common.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <locale>

namespace csv
{
  /************************************************************************
   *                                                                      *
   * CSV specification                                                    *
   *                                                                      *
   ************************************************************************/
  template<typename CHAR, typename TRAITS>
  class BasicSpecification
  {
  public:
    typedef CHAR                                      char_type;
    typedef TRAITS                                    char_traits;
    typedef std::basic_string<char_type, char_traits> string_type;

    BasicSpecification();

    inline BasicSpecification& withHeader();
    inline BasicSpecification& withoutHeader();
    inline bool hasHeader() const;
    
    inline BasicSpecification& withUsingEmptyLines();
    inline BasicSpecification& withoutUsingEmptyLines();
    inline bool isUsingEmptyLines() const;

    ///////////////////////////////////////////////
    inline BasicSpecification& withSeparator(const string_type & seps);
    inline bool isSeparator(char_type ch) const;
    inline bool hasSeparator() const;
    inline char_type defaultSeparator() const;

    ///////////////////////////////////////////////
    inline BasicSpecification& withLocale(const ::std::locale & loc);
    inline BasicSpecification& withDecimalSeparator(char_type ch);
    inline const ::std::locale& locale() const;

    ///////////////////////////////////////////////
    inline BasicSpecification& withComment(char_type ch);
    inline BasicSpecification& withoutComment();
    inline bool isComment(char_type ch) const;


    ///////////////////////////////////////////////
    inline BasicSpecification& withColumn(::std::size_t index, 
                                          const string_type & name);
    
  private:
    class Column
    {
    public:
      Column(::std::size_t index, const string_type & name) ;
      inline const string_type & name() const;
      inline ::std::size_t index() const;
    private:
      ::std::size_t _index;
      string_type   _name;
    };

    class DecimalSeparator : public std::numpunct<char_type>
    {
    public:
      DecimalSeparator(char_type ch)
        : separator(ch)
      {}
    protected:
      char_type do_decimal_point()const
      {
        return separator;
      }
    private:
      char_type separator;
    };

    friend class BasicCell<char_type,   char_traits>;
    friend class BasicRow<char_type,    char_traits>;
    friend class BasicReader<char_type, char_traits>;
    typedef unsigned int                                flags_type;    
    typedef ::std::shared_ptr<Column>                   shared_column_type;
    typedef ::std::map<string_type, shared_column_type> lookup_type;
    typedef ::std::vector<std::shared_ptr<Column> >     columns_type;


    static const flags_type _use_empty_lines = 1;
    static const flags_type _with_header     = 2;

    bool addColumnIfNotEmpty(std::size_t column, const string_type & name);

    inline shared_column_type addColumnIfNotExists(::std::size_t column);
    inline shared_column_type addColumnIfNotExists(::std::size_t column,
                                                   string_type   name);

    lookup_type              _lookup;
    columns_type             _columns;
    ::std::vector<char_type> _separators;
    char_type                _default_separator;
    flags_type               _flags;
    char_type                _comment_char;
    ::std::locale            _locale;
  };

  ///////////////////////////////////////////////////////////////////
  // 
  // Implementation 
  //
  ////////////////////////////////////////////////////////////////////
  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>::BasicSpecification() 
    : _default_separator(char_type(',')),
      _flags(0),
      _comment_char(char_type(0))
  {
    _separators.push_back(_default_separator);
  }

  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withHeader()
  {
    _flags|= _with_header;
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withoutHeader()
  {
    _flags = (_flags | _with_header) ^ _with_header;
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline bool BasicSpecification<CHAR, TRAITS>::hasHeader() const 
  {
    return _flags & _with_header;
  }

  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withUsingEmptyLines()
  {
    _flags|= _use_empty_lines;
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withoutUsingEmptyLines()
  {
    _flags = (_flags | _use_empty_lines) ^ _use_empty_lines;
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline bool BasicSpecification<CHAR, TRAITS>::isUsingEmptyLines() const
  {
    return _flags & _use_empty_lines;
  }

  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withSeparator(const string_type & seps)
  {
    _separators.clear();
    for(auto ch : seps)
    {
      for(auto item : _separators)
      {
        if(item == ch)
        {
          continue;
        }
      }
      _separators.push_back(ch);
      if(_separators.size() == 1u)
      {
        _default_separator = ch;
      }
    }
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline bool BasicSpecification<CHAR, TRAITS>::hasSeparator() const 
  {
    return !_separators.empty();
  }


  template<typename CHAR, typename TRAITS>
  inline bool BasicSpecification<CHAR, TRAITS>::isSeparator(char_type ch) const 
  {
    for(auto s : _separators) 
    {
      if(s == ch) 
      {
        return true;
      }
    }
    return false;
  }

  template<typename CHAR, typename TRAITS>
  inline typename BasicSpecification<CHAR, TRAITS>::char_type 
  BasicSpecification<CHAR, TRAITS>::defaultSeparator() const
  {
    return _default_separator;
  }

  ///////////////////////////////////////////////
  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withLocale(const ::std::locale & loc)
  {
    _locale=loc;
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>&
  BasicSpecification<CHAR, TRAITS>::withDecimalSeparator(CHAR ch)
  {
    typedef typename BasicSpecification<CHAR, TRAITS>::DecimalSeparator
      dec_sep_type;
    return withLocale(std::locale(std::locale(),new dec_sep_type(ch)));
  }

  template<typename CHAR, typename TRAITS>
  const ::std::locale& BasicSpecification<CHAR, TRAITS>::locale() const
  {
    return _locale;
  }

  
  ///////////////////////////////////////////////
  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withComment(char_type ch)
  {
    _comment_char = ch;
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withoutComment()
  {
    _comment_char = char_type(0);
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline bool BasicSpecification<CHAR, TRAITS>::isComment(char_type ch) const 
  {
    return ( ch == _comment_char );
  }

  ///////////////////////////////////////////////
  template<typename CHAR, typename TRAITS>
  inline BasicSpecification<CHAR, TRAITS>& 
  BasicSpecification<CHAR, TRAITS>::withColumn(::std::size_t index, 
                                               const string_type & name)
  {
    if(_columns.size() < index+1) 
    {
      _columns.resize(index+1, shared_column_type());
    }
    if( _columns[index] ) 
    {
      throw 
        DuplicateColumnError("Column " + ::std::to_string(index) + 
                             " already defined.",
                             _columns[index]->index(),
                             0,0,0,0);
    }
    auto res = 
      _lookup.insert(std::make_pair(name, 
                                    std::make_shared<Column>(index,
                                                             name)));
    if(res.second) 
    {
      _columns[index] = res.first->second;
    }
    else 
    {
      throw 
        DuplicateColumnError("Column name already defined.",
                             res.first->second->index(),
                             0,0,0,0);
    }
    return *this;
  }



  template<typename CHAR, typename TRAITS>
  BasicSpecification<CHAR, TRAITS>::Column::Column(::std::size_t index, 
                                                   const string_type & name)
    : _index(index),_name(name){}
  
  template<typename CHAR, typename TRAITS>
  const typename BasicSpecification<CHAR, TRAITS>::string_type &
  BasicSpecification<CHAR, TRAITS>::Column::name() const
  {
    return _name;
  }

  template<typename CHAR, typename TRAITS>
  ::std::size_t BasicSpecification<CHAR, TRAITS>::Column::index() const
  {
    return _index;
  }

  template<typename CHAR, typename TRAITS>
  bool BasicSpecification<CHAR, TRAITS>::
  addColumnIfNotEmpty(std::size_t column, const string_type & name)
  {
    if(_columns.size() < column+1) 
    {
      _columns.resize(column+1, shared_column_type());
    }
    if( name.empty()) 
    {
      return true;
    }
    if( _columns[column] && ! _columns[column]->name().empty()) 
    {
      // already defined 
      return false;
    }
    auto res = 
      _lookup.insert(std::make_pair(name, 
                                    std::make_shared<Column>(column,
                                                             name)));
    if(res.second) 
    {
      _columns[column] = res.first->second;
      return true;
    }
    else 
    {
      return false;
    }
  }

  template<typename CHAR, typename TRAITS>
  inline typename BasicSpecification<CHAR, TRAITS>::shared_column_type 
  BasicSpecification<CHAR, TRAITS>::addColumnIfNotExists(::std::size_t column)
  {
    if(_columns.size() < column+1) 
    {
      _columns.resize(column+1, shared_column_type());
    }
    if(! _columns[column]) 
    {
      _columns[column] = ::std::make_shared<Column>(column,string_type());
    }
    return _columns[column];
  }

  template<typename CHAR, typename TRAITS>
  inline typename BasicSpecification<CHAR, TRAITS>::shared_column_type 
  BasicSpecification<CHAR, TRAITS>::addColumnIfNotExists(::std::size_t column,
                                                         string_type name)
  {
    if(_columns.size() < column+1) 
    {
      _columns.resize(column+1, shared_column_type());
    }
    if(! _columns[column]) 
    {
      _columns[column] = ::std::make_shared<Column>(column,name);
    }
    return _columns[column];
  }

} // namespace
