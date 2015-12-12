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
#include "cell.h"
#include <iterator>
#include <limits>
#include <iostream>

namespace csv
{
  template<typename CHAR, typename TRAITS>
  class BasicRow
  {
  public:
    typedef CHAR                                             char_type;
    typedef TRAITS                                           char_traits;
    typedef BasicCell<char_type, char_traits>                cell_type;
    typedef typename cell_type::spec_type                    spec_type;
    typedef std::shared_ptr<spec_type>                       shared_spec_type;
    typedef typename cell_type::string_type                  string_type;
    typedef typename std::vector<cell_type>::value_type      value_type;
    typedef typename std::vector<cell_type>::size_type       size_type;
    typedef typename std::vector<cell_type>::difference_type difference_type;
    typedef typename std::vector<cell_type>::const_reference const_reference;
    typedef typename std::vector<cell_type>::const_pointer   const_pointer;
    typedef typename std::vector<cell_type>::const_iterator  const_iterator;
    typedef typename 
    std::vector<cell_type>::const_reverse_iterator    const_reverse_iterator;

    BasicRow();
    BasicRow(const BasicRow<char_type, char_traits> & rhs);
    BasicRow(const BasicRow<char_type, char_traits> && rhs);
    BasicRow(const spec_type & spec);
    BasicRow(const shared_spec_type & spec);

    template<typename C>
    BasicRow(const C & container);

    template<typename C>
    BasicRow(const C & container, const spec_type & spec) ;

    template<typename C>
    BasicRow(const C & container, const shared_spec_type & spec) ;

    BasicRow & operator=(const BasicRow & rhs);
    BasicRow & operator=(const BasicRow && rhs);

    inline std::size_t size() const               { return _shared_buffer->_ranges.size();    }
    inline const_iterator begin() const           { return _cells.begin();   }
    inline const_iterator end() const             { return _cells.end();     }
    inline const_iterator cbegin() const          { return _cells.cbegin();  }
    inline const_iterator cend() const            { return _cells.cend();    }
    inline const_reverse_iterator rbegin() const  { return _cells.rbegin();  }
    inline const_reverse_iterator rend() const    { return _cells.rend();    }
    inline const_reverse_iterator crbegin() const { return _cells.crbegin(); }
    inline const_reverse_iterator crend() const   { return _cells.crend();   }

    inline const cell_type & operator[](std::size_t i) const;
    inline const cell_type & operator[](const string_type & name) const ;
    
    inline ::std::size_t inputLine() const        { return _shared_buffer->_begin_input_line; }
    inline ::std::size_t row() const              { return _shared_buffer->_csv_row; }
    inline void clear(::std::size_t begin_input_line = 0,
                      ::std::size_t csv_row = 0);
  private:
    friend class BasicReader<char_type, char_traits>;
    typedef typename cell_type::buffer_type            buffer_type;
    typedef typename cell_type::shared_buffer_type     shared_buffer_type;
    typedef typename ::std::vector<cell_type>          cell_vector_type;
    typedef typename spec_type::Column                 column_type;
    typedef typename ::std::shared_ptr<column_type>    shared_column_type;
    shared_spec_type                                   _shared_spec;
    shared_buffer_type                                 _shared_buffer;
    cell_vector_type                                   _cells;

    ::std::size_t initColumn(const char_type * str, 
                             ::std::size_t       j, 
                             buffer_type *     _tmp_buffer);
    std::size_t initColumn(const string_type & str, 
                           std::size_t         j, 
                           buffer_type       * _tmp_buffer);

    template<typename ITER>
    void init(const ITER & begin, const ITER & end, std::forward_iterator_tag);
    typedef int int_type; //Todo: char traits
    inline void closeCell();
    inline void addCell(::std::size_t input_row, ::std::size_t input_column);
    inline void addChar(int_type ch);
    inline void flushCell();
    inline void revertCell();
  };

  //////////////////////////////////////////////////////////////////////
  //
  // Implementation 
  //
  //////////////////////////////////////////////////////////////////////
  template<typename CHAR, typename TRAITS>
  BasicRow<CHAR,TRAITS>::BasicRow(const BasicRow<char_type, char_traits> & rhs)
      : _shared_spec(rhs._shared_spec),
        _shared_buffer(rhs._shared_buffer),
        _cells(rhs._cells)
  {}

  template<typename CHAR, typename TRAITS>
  BasicRow<CHAR,TRAITS>::BasicRow(const BasicRow<char_type, char_traits> && 
                                  rhs)
    : _shared_spec(rhs._shared_spec),
      _shared_buffer(rhs._shared_buffer),
      _cells(::std::move(rhs._cells))
  {}

  template<typename CHAR, typename TRAITS>
  BasicRow<CHAR,TRAITS>::BasicRow()
    : _shared_spec(::std::make_shared<spec_type>()),
      _shared_buffer(::std::make_shared<buffer_type>(0,0))
  {}

  template<typename CHAR, typename TRAITS>
  BasicRow<CHAR,TRAITS>::BasicRow(const spec_type & spec)
    : _shared_spec(::std::make_shared<spec_type>(spec)),
      _shared_buffer(::std::make_shared<buffer_type>())
  {}

  template<typename CHAR, typename TRAITS>
  BasicRow<CHAR,TRAITS>::BasicRow(const shared_spec_type & spec)
    : _shared_spec(spec),
      _shared_buffer(::std::make_shared<buffer_type>())
  {}
  template<typename CHAR, typename TRAITS>
    template<typename C>
  BasicRow<CHAR,TRAITS>::BasicRow(const C & container) 
    : _shared_spec(::std::make_shared<spec_type>())
  {
    auto begin = std::begin(container);
    auto end   = std::end(container);
    typedef std::iterator_traits<decltype(begin)> iterator_traits;
    typedef typename iterator_traits::iterator_category iterator_category;
    init(begin, end, iterator_category());
  }
  template<typename CHAR, typename TRAITS>
    template<typename C>
  BasicRow<CHAR,TRAITS>::BasicRow(const C & container, const spec_type & spec) 
    : _shared_spec(::std::make_shared<spec_type>(spec))
  {
    auto begin = std::begin(container);
    auto end   = std::end(container);
    typedef std::iterator_traits<decltype(begin)> iterator_traits;
    typedef typename iterator_traits::iterator_category iterator_category;
    init(begin, end, iterator_category());
  }

  template<typename CHAR, typename TRAITS>
  template<typename C>
  BasicRow<CHAR,TRAITS>::BasicRow(const C & container, 
                                  const shared_spec_type & spec) 
    : _shared_spec(spec)
  {
    auto begin = std::begin(container);
    auto end   = std::end(container);
    typedef std::iterator_traits<decltype(begin)> iterator_traits;
    typedef typename iterator_traits::iterator_category iterator_category;
    init(begin, end, iterator_category());
  }

  template<typename CHAR, typename TRAITS>
  BasicRow<CHAR,TRAITS> & 
  BasicRow<CHAR,TRAITS>::operator=(const BasicRow<CHAR,TRAITS> & rhs) 
  {
    _shared_spec   = rhs._shared_spec;
    _shared_buffer = rhs._shared_buffer;
    _cells         = rhs._cells;
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  BasicRow<CHAR,TRAITS> & 
  BasicRow<CHAR,TRAITS>::operator=(const BasicRow<CHAR,TRAITS> && rhs) 
  {
    _shared_buffer = ::std::move(rhs._shared_buffer);
    _cells         = ::std::move(rhs._cells);
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline void BasicRow<CHAR,TRAITS>::clear(::std::size_t begin_input_line,
                                           ::std::size_t csv_row) 
  {
    _cells.clear();
    if(_shared_buffer.use_count() > 1) 
    {
      // Copy on write
      _shared_buffer.reset(new buffer_type(begin_input_line, csv_row));
    }
    else 
    {
      _shared_buffer->clear(begin_input_line, csv_row);
    }
  }
  
  template<typename CHAR, typename TRAITS>
  const typename BasicRow<CHAR,TRAITS>::cell_type & 
  BasicRow<CHAR,TRAITS>::operator[](std::size_t i) const
  {
    if(i >= _cells.size())
    {
      throw CellOutOfRangeError("Cell index " + ::std::to_string(i) + 
                                " out of range [0," + 
                                ::std::to_string(_cells.size()) + ")",
                                i,
                                _cells.size(),
                                _shared_buffer->_begin_input_line,
                                0, /*input_column, */
                                _shared_buffer->_csv_row,
                                0  /*csv_column */);
    }
    else 
    {
      return _cells[i];
    }
  } 

  template<typename CHAR, typename TRAITS>
  const typename BasicRow<CHAR,TRAITS>::cell_type & 
  BasicRow<CHAR,TRAITS>::operator[](const string_type & name) const 
  {
    auto itr = _shared_spec->_lookup.find(name);
    if(itr == _shared_spec->_lookup.end())
    {
      throw UndefinedColumnError("Accessing undefined column by name.",
                                 _cells.size(),
                                 _shared_buffer->_begin_input_line,
                                 0, /* input_column, */
                                 _shared_buffer->_csv_row,
                                 0  /* csv_column*/ );
    }
    else 
    {
      std::size_t i = itr->second->index();
      if(i >= _cells.size()) 
      {
        throw DefinedCellOutOfRangeError("Named column out of range.",
                                         i,
                                         _cells.size(),
                                         _shared_buffer->_begin_input_line,
                                         0, /* input_column,*/
                                         _shared_buffer->_csv_row,
                                         0 /*csv_column*/ );
      }
      else 
      {
        return _cells[i];
      }
    }
  }

  template<typename CHAR, typename TRAITS>
  ::std::size_t 
  BasicRow<CHAR,TRAITS>::initColumn(const char_type * str, 
                                    ::std::size_t       j, 
                                    buffer_type *     _tmp_buffer) 
  {
    while(*str) 
    {
      _tmp_buffer->addChar(*str);
      ++str;
      ++j;
    }
    return j;
  }

  template<typename CHAR, typename TRAITS>
  ::std::size_t 
  BasicRow<CHAR,TRAITS>::initColumn(const string_type & str, 
                                    std::size_t         j, 
                                    buffer_type       * _tmp_buffer) 
  {
    for(auto itr = str.begin(); itr != str.end(); ++itr) 
    {
      _tmp_buffer->addChar(*itr);
      ++j;
    }
    return j;
  }

  template<typename CHAR, typename TRAITS>
  inline void BasicRow<CHAR,TRAITS>::addCell(::std::size_t input_row, ::std::size_t input_column )
  {
    _shared_buffer->addCell(input_row,input_column);
  }

  template<typename CHAR, typename TRAITS>
  inline void BasicRow<CHAR,TRAITS>::addChar(int_type ch )
  {
    _shared_buffer->addChar(ch);
  }


  template<typename CHAR, typename TRAITS>
  inline void BasicRow<CHAR,TRAITS>::closeCell()
  {
    _shared_buffer->closeCell();
    _cells.push_back(cell_type(_shared_spec,
                               _shared_spec->addColumnIfNotExists(_cells.size()),
                               _shared_buffer,
                               _cells.size()));
  }

  template<typename CHAR, typename TRAITS>
  inline void BasicRow<CHAR,TRAITS>::flushCell()
  {
    _shared_buffer->flushCell();
  }

  template<typename CHAR, typename TRAITS>
  inline void BasicRow<CHAR,TRAITS>::revertCell()
  {
    _shared_buffer->revertCell();
  }


  template<typename CHAR, typename TRAITS>
  template<typename ITER>
  void BasicRow<CHAR,TRAITS>::init( const ITER & begin, 
                                    const ITER & end, 
                                    std::forward_iterator_tag )
  {
    buffer_type * _tmp = new buffer_type(0,0);
    _shared_buffer = shared_buffer_type(_tmp);
    std::size_t j = 0;
    for(ITER itr=begin; itr != end; ++itr) 
    {
      std::size_t i = j;
      addCell(0,i);
      j = initColumn(*itr, j, _tmp);
      closeCell();
    }
  }
} // namaspace 

