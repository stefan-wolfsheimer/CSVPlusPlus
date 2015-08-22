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

#include <iterator>
#include <istream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include "csv_common.h"
#include "specification.h"
#include "row.h"
#include "cell.h"

namespace csv
{
  
  template<typename CHAR, typename TRAITS>
  class BasicReader
  {
  public:
    typedef CHAR                                         char_type;
    typedef TRAITS                                       char_traits;
    typedef ::std::basic_string<char_type,  char_traits> string_type;
    typedef ::std::basic_istream<char_type, char_traits> istream_type;
    typedef BasicRow<char_type, char_traits>             row_type;
    typedef typename row_type::spec_type                 spec_type;

    /** 
     * Input iterator to read CSV from std::istream
     */
    class iterator : public ::std::iterator<::std::input_iterator_tag, 
                                             row_type>
    {
      friend class BasicReader<char_type, char_traits>;
      BasicReader * reader;
      row_type      row;
      iterator(BasicReader * _reader);
    public:
      inline const row_type& operator*()  const;
      inline const row_type* operator->() const;
      iterator();
      inline iterator& operator++();
      inline iterator& operator++(int);
      inline bool operator==(const iterator & rhs);
      inline bool operator!=(const iterator & rhs);
    };

    BasicReader(istream_type & _ist, spec_type _specs = spec_type());

    inline iterator begin() { return iterator(this); }
    inline iterator end()   { return iterator();     }

  protected:
    enum class State
    {
      START,
      NEXT_COL,
      WS_BEFORE_NEXT_COL,
      QUOTED_COL,
      ESCAPED_COL,
      QUOTED_COL_RIGHT_WS,
      UNQUOTED_COL,
      UNQUOTED_COL_RIGHT_WS,
      COMMENT,
      END
    };
    typedef typename row_type::buffer_type        buffer_type;
    typedef typename row_type::shared_buffer_type shared_buffer_type;
    typedef typename row_type::shared_spec_type   shared_spec_type;
    typedef typename row_type::cell_type          cell_type;
    typedef typename row_type::range_type         range_type;

    istream_type                                & _ist;
    shared_spec_type                              _specs;
    char_type                                     _quote;

    // buffer
    row_type                                      _current_row;
    shared_buffer_type                            _last_buffer;
    shared_buffer_type                            _buffer;
    ::std::vector<cell_type>                      _last_cells;
    ::std::vector<cell_type>                      _cells;

    // state
    State                                         _state;
    bool                                          _is_end_of_row;
    bool                                          _has_been_flushed;
    ::std::size_t                                 _last_unquoted_non_ws_pos;
    ::std::size_t                                 _current_input_line;
    ::std::size_t                                 _current_input_column;
    ::std::size_t                                 _last_input_line;
    ::std::size_t                                 _flushed_input_line;
    ::std::size_t                                 _last_cell_input_line;
    ::std::size_t                                 _last_cell_input_column;
    ::std::size_t                                 _csv_row;
    ::std::size_t                                 _buffer_csv_row;
    ::std::size_t                                 _last_buffer_csv_row;
    ::std::size_t                                 _csv_column;

    inline bool isWhiteSpace(int ch);
    inline bool isNewline(int ch);
    inline bool isQuote(int ch);
    inline bool isEof(int ch);
    inline void flush();
    inline void addCell();
    inline void addEmptyCell();

    void scanStateStart(int ch);
    void scanStateWhiteSpaceBeforeNextCol(int ch);
    void scanStateNextCol(int ch);
    void scanStateQuotedCol(int ch);
    void scanStateEscapedCol(int ch) ;
    void scanStateQuotedColRightWS(int ch);
    void scanUnquotedCol(int ch);
    void scanUnquotedColRightWS(int ch);
    void scanStateComment(int ch);
    bool consume();
  };

  ///////////////////////////////////////////////////////////////
  //
  // Implementation
  //
  ///////////////////////////////////////////////////////////////


  // iterator
  template<typename CHAR, typename TRAITS>
  BasicReader<CHAR,TRAITS>::iterator::iterator(BasicReader * _reader) 
    : reader(_reader) 
  {
    row._shared_spec   = reader->_specs;
    row._shared_buffer = reader->_last_buffer;
    row._cells         = reader->_last_cells;
    row._input_line    = reader->_last_input_line;
    ++*this;
  }

  template<typename CHAR, typename TRAITS>
  inline const typename BasicReader<CHAR,TRAITS>::row_type& 
  BasicReader<CHAR,TRAITS>::iterator::operator*()  const 
  { 
    return row;  
  }

  template<typename CHAR, typename TRAITS>
  inline const typename BasicReader<CHAR,TRAITS>::row_type* 
  BasicReader<CHAR,TRAITS>::iterator::operator->() const 
  { 
    return &row; 
  }

  template<typename CHAR, typename TRAITS>
  BasicReader<CHAR,TRAITS>::iterator::iterator() 
  {
    reader = 0;
  }

  template<typename CHAR, typename TRAITS>
  inline typename BasicReader<CHAR,TRAITS>::iterator& 
  BasicReader<CHAR,TRAITS>::iterator::operator++() 
  {
    if(reader) 
    {
      while(reader->consume());
      if(reader->_has_been_flushed) 
      {
        row._shared_spec          = reader->_specs;
        row._shared_buffer        = reader->_last_buffer; 
        row._cells                = reader->_last_cells;
        row._input_line           = reader->_flushed_input_line;
        row._row                  = reader->_last_buffer_csv_row;
        reader->_has_been_flushed = false;
      }
      else if(reader->_state == BasicReader::State::END) 
      {
        reader = 0;
      }
    }
    return *this;
  }

  template<typename CHAR, typename TRAITS>
  inline typename BasicReader<CHAR,TRAITS>::iterator& 
  BasicReader<CHAR,TRAITS>::iterator::operator++(int) 
  {
    iterator tmp = *this;
    ++*this;
    return tmp;
  }

  template<typename CHAR, typename TRAITS>
  inline bool 
  BasicReader<CHAR,TRAITS>::iterator::operator==(const iterator & rhs) 
  { 
    return reader == rhs.reader; 
  }
      
  template<typename CHAR, typename TRAITS>
  inline bool 
  BasicReader<CHAR,TRAITS>::iterator::operator!=(const iterator & rhs) 
  { 
    return reader != rhs.reader; 
  }

  // Basic reader
  template<typename CHAR, typename TRAITS>
  BasicReader<CHAR,TRAITS>::BasicReader( istream_type       & ist,
                                         spec_type            specs )
    : _ist(ist)
  {
    _buffer                   = ::std::make_shared<buffer_type>();
    _last_buffer              = ::std::make_shared<buffer_type>();
    _specs                    = ::std::make_shared<spec_type>(specs);
    _quote                    = '"';
    _last_input_line          = 0;
    _flushed_input_line       = 0;
    _current_input_line       = 0;
    _current_input_column     = 0;
    _last_cell_input_line     = 0;
    _last_cell_input_column   = 0;
    _csv_row                  = 0;
    _buffer_csv_row           = 0;
    _last_buffer_csv_row      = 0;
    _csv_column               = 0;

    _state                    = State::START;
    _is_end_of_row            = false;
    _has_been_flushed         = false;
    _last_unquoted_non_ws_pos = 0;

    if(_specs->hasHeader()) 
    {
      // read header from file
      row_type row = *this->begin();
      std::size_t column = 0;
      for(auto itr = row.begin(); itr != row.end(); ++itr) 
      {
        if(!_specs->addColumnIfNotEmpty(column, 
                                        itr->template as<string_type>()))
        {
          auto res = _specs->_lookup.find(itr->template as<string_type>());
          ::std::size_t def_column = 0;
          if(res != _specs->_lookup.end()) 
          {
            def_column = res->second->index();
          }
          throw DuplicateColumnError("Column already defined.",
                                     def_column,
                                     row.inputLine(),
                                     itr->inputColumn(),
                                     itr->row(),
                                     itr->column());
        }
        column++;
      }
    }
  }
  
  // state automaton
  template<typename CHAR, typename TRAITS>
  inline void BasicReader<CHAR,TRAITS>::flush()
  {
    if( _is_end_of_row ) 
    {
      _last_buffer         = _buffer;
      _last_buffer_csv_row = _buffer_csv_row;
      _last_cells          = _cells;
      _flushed_input_line  = _last_input_line;
      _buffer              = std::make_shared<buffer_type>();
      _buffer_csv_row      = _csv_row;
      _has_been_flushed    = true;
      _is_end_of_row       = false;
      _cells.clear();
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::addCell()
  {
    std::size_t n = _cells.empty() ? 0 : _cells.back()._range._end;
    auto x = _specs->addColumnIfNotExists(_cells.size());
    _cells.push_back(cell_type(_specs,
                               _specs->addColumnIfNotExists(_cells.size()),
                               _buffer, 
                               range_type(n,
                                          _buffer->size(),
                                          _csv_row,
                                          _csv_column,
                                          _last_cell_input_line,
                                          _last_cell_input_column)));
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::addEmptyCell()
  {
    std::size_t n = _cells.empty() ? 0 : _cells.back()._range._end;
    _cells.push_back( cell_type( _specs,
                                 _specs->addColumnIfNotExists(_cells.size()),
                                 _buffer, 
                                 range_type( n,
                                             n,
                                             _csv_row,
                                             _csv_column,
                                             _last_cell_input_line,
                                             _last_cell_input_column)));
  }

  template<typename CHAR, typename TRAITS>
  inline bool BasicReader<CHAR,TRAITS>::isWhiteSpace(int ch) 
  {
    return ch == ' ' || ch == '\t';
  }

  template<typename CHAR, typename TRAITS>
  inline bool BasicReader<CHAR,TRAITS>::isNewline(int ch) 
  {
    return ch == '\n';
  }

  template<typename CHAR, typename TRAITS>
  inline bool BasicReader<CHAR,TRAITS>::isQuote(int ch) 
  {
      return ch == _quote;
  }
  
  template<typename CHAR, typename TRAITS>
  inline bool BasicReader<CHAR,TRAITS>::isEof(int ch) 
  {
    return ch == EOF;
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanStateStart(int ch) 
  {
    if(isWhiteSpace(ch)) 
    {
    }
    else if( _specs->isSeparator(ch) ) 
    {
      flush();
      _last_input_line        = _current_input_line;
      _last_cell_input_line   = _current_input_line;
      _last_cell_input_column = _current_input_column;
      addEmptyCell();
      _csv_column++;
      _state = State::NEXT_COL;
    }
    else if( isNewline(ch) )
    {
      if( _specs->hasUsingEmptyLines() ) 
      {
        flush();
        _last_input_line = _current_input_line;
        _is_end_of_row = true;
        _csv_column = 0;
        _csv_row++;
      }
    }
    else if( isEof(ch) ) 
    {
      flush();
      _last_input_line = _current_input_line;
      _state = State::END;
    }
    else if( isQuote(ch) ) 
    {
      flush();
      _last_input_line        = _current_input_line;
      _last_cell_input_line   = _current_input_line;
      _last_cell_input_column = _current_input_column;
      _state = State::QUOTED_COL;
    }
    else if( _specs->isComment(ch))
    {
      _state = State::COMMENT;
      if( _specs->hasUsingEmptyLines() ) 
      {
        _csv_column = 0;
        _csv_row++;
      }
    }
    else 
    {
      flush();
      _last_input_line        = _current_input_line;
      _last_cell_input_line   = _current_input_line;
      _last_cell_input_column = _current_input_column;
      _buffer->push_back(ch);
      _state = State::UNQUOTED_COL;
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanStateWhiteSpaceBeforeNextCol(int ch) 
  {
    if(isWhiteSpace(ch)) 
    {
      // stay in state
    }
    else if( _specs->isSeparator(ch) ) 
    {
      _last_cell_input_line   = _current_input_line;
      _last_cell_input_column = _current_input_column;
      addEmptyCell();
      _csv_column++;
      _state = State::NEXT_COL;
    }
    else if( isNewline(ch) ) 
    {
      _is_end_of_row = true;
      _state = State::START;
      _csv_column    = 0;
      _csv_row++;
    }
    else if( _specs->isComment(ch))
    {
      _is_end_of_row = true;
      _state         = State::COMMENT;
      _csv_column    = 0;
      _csv_row++;
    }
    else if( isEof(ch) ) 
    {
      _is_end_of_row = true;
      flush();
      _state = State::END;
    }
    else if( isQuote(ch) ) 
    {
      flush();
      _state = State::QUOTED_COL;
    }
    else 
    {
      flush();
      _buffer->push_back(ch);
      _state = State::UNQUOTED_COL;
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanStateNextCol(int ch) 
  {
    if(isWhiteSpace(ch)) 
    {
      // stay in state
    }
    else if( _specs->isSeparator(ch) ) 
    {
      _last_cell_input_line = _current_input_line;
      _last_cell_input_column = _current_input_column;
      addEmptyCell();
      _csv_column++;
      _state = State::NEXT_COL;
    }
    else if( isNewline(ch) ) 
    {
      _last_cell_input_line = _current_input_line;
      _last_cell_input_column = _current_input_column;
      addEmptyCell();
      _is_end_of_row = true;
      _csv_column = 0;
      _csv_row++;
      _state = State::START;
    }
    else if( _specs->isComment(ch))
    {
      _last_cell_input_line = _current_input_line;
      _last_cell_input_column = _current_input_column;
      addEmptyCell();
      _is_end_of_row = true;
      _state         = State::COMMENT;
      _csv_column    = 0;
      _csv_row++;
    }
    else if( isEof(ch) ) 
    {
      _last_cell_input_line = _current_input_line;
      _last_cell_input_column = _current_input_column;
      addEmptyCell();
      _is_end_of_row = true;
      _csv_column++;
      flush();
      _state = State::END;
    }
    else if( isQuote(ch) ) 
    {
      flush();
      _last_cell_input_line = _current_input_line;
      _last_cell_input_column = _current_input_column;
      _state = State::QUOTED_COL;
    }
    else 
    {
      flush();
      _last_cell_input_line = _current_input_line;
      _last_cell_input_column = _current_input_column;
      _buffer->push_back(ch);
      _state = State::UNQUOTED_COL;
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanStateQuotedCol(int ch) 
  {
    if(isQuote(ch)) 
    {
      _state = State::ESCAPED_COL;
    }
    else if(!isEof(ch)) 
    {
      _buffer->push_back(ch);
    }
    else 
    {
      throw std::exception();
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanStateEscapedCol(int ch) 
  {
    if(isQuote(ch)) 
    {
      _buffer->push_back(_quote);
      _state = State::QUOTED_COL;
    }
    else if(_specs->isSeparator(ch)) 
    {
      // separator before white space 
      addCell();
      _csv_column++;
      if( isWhiteSpace(ch) ) 
      {
        _state = State::WS_BEFORE_NEXT_COL;
      }
      else 
      {
        _state = State::NEXT_COL;
      }
    }
    else if(isWhiteSpace(ch))
    {
      addCell();
      _csv_column++;
      _state = State::QUOTED_COL_RIGHT_WS;
    }
    else if(isNewline(ch)) 
    {
      addCell();
      _csv_column = 0;
      _csv_row++;
      _is_end_of_row = true;
      _state = State::START;
    }
    else if( _specs->isComment(ch))
    {
      addCell();
      _csv_column = 0;
      _csv_row++;
      _is_end_of_row = true;
      _state = State::COMMENT;
    }
    else if(isEof(ch)) 
    {
      addCell();
      _csv_column++;
      _is_end_of_row = true;
      flush();
      _state = State::END;
    }
    else 
    {
      throw ParseError("Unexpected character at the end of quoted cell.",
                       _current_input_line,
                       _current_input_column,
                       _csv_row,
                       _csv_column);
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanStateQuotedColRightWS(int ch)
  {
    if( isWhiteSpace(ch) )
    {
    }
    else if( _specs->isSeparator(ch) ) 
    {
      _state = State::NEXT_COL;
    }
    else if( isNewline(ch) ) 
    {
      _is_end_of_row = true;
      _state         = State::START;
      _csv_row++;
      _csv_column    = 0;

    }
    else if( _specs->isComment(ch))
    {
      _csv_row++;
      _csv_column    = 0;
      _is_end_of_row = true;
      _state         = State::COMMENT;
    }
    else if( isEof(ch) )
    {
      _is_end_of_row = true;
      flush();
      _state = State::END;
    }
    else 
    {
      // error
      throw std::exception();
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanUnquotedCol(int ch)
  {
    if(_specs->isSeparator(ch)) 
    {
      // check separator before white space 
      addCell();
      _csv_column++;
      if( isWhiteSpace(ch) ) 
      {
        _state = State::WS_BEFORE_NEXT_COL;
      }
      else 
      {
        _state = State::NEXT_COL;
      }
    }
    else if( isWhiteSpace(ch) ) 
    {
      // remember current position in buffer
      _last_unquoted_non_ws_pos = _buffer->size();
      _buffer->push_back(ch);
      _state = State::UNQUOTED_COL_RIGHT_WS;
    }
    else if(isNewline(ch)) 
    {
      addCell();
      _csv_column=0;
      _csv_row++;
      _is_end_of_row = true;
      _state = State::START;
    }
    else if( _specs->isComment(ch))
    {
      addCell();
      _is_end_of_row = true;
      _state = State::COMMENT;
      // rest of the row is comment.
      _csv_column    = 0;
      _csv_row++;
    }
    else if(isEof(ch)) 
    {
      // end of col
      addCell();
      _csv_column++;
      _is_end_of_row = true;
      flush();
      _state = State::END;
    }
    else 
    {
      _buffer->push_back(ch);
    }
  }
  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanUnquotedColRightWS(int ch)
  {
    if(isWhiteSpace(ch)) 
    {
      _buffer->push_back(ch);
    }
    else if(_specs->isSeparator(ch))
    {
      _buffer->resize(_last_unquoted_non_ws_pos);
      addCell();
      _csv_column++;
      _state = State::NEXT_COL;
    }
    else if(isNewline(ch)) 
    {
      _buffer->resize(_last_unquoted_non_ws_pos);
      addCell();
      _csv_column    = 0;
      _csv_row++;
      _is_end_of_row = true;
      _state         = State::START;
    }
    else if( _specs->isComment(ch))
    {
      _buffer->resize(_last_unquoted_non_ws_pos);
      addCell();
      _csv_column=0;
      _csv_row++;
      _is_end_of_row = true;
      _state = State::COMMENT;
    }
    else if(isEof(ch))
    {
      _buffer->resize(_last_unquoted_non_ws_pos);
      addCell();
      _csv_column++;
      _is_end_of_row = true;
      flush();
      _state = State::END;
    }
    else 
    {
      _buffer->push_back(ch);
      _last_unquoted_non_ws_pos = _buffer->size();
      _state = State::UNQUOTED_COL;
    }
  }

  template<typename CHAR, typename TRAITS>
  void BasicReader<CHAR,TRAITS>::scanStateComment(int ch)
  {
    if(isNewline(ch)) 
    {
      _state = State::START;
    }
    else if(isEof(ch))
    {
      flush();
      _state = State::END;
    }
  }
  template<typename CHAR, typename TRAITS>
  bool BasicReader<CHAR,TRAITS>::consume()
  {
    _has_been_flushed = false;
    if(_state == State::END) 
    {
      return false;
    }
    if(_ist.good()) 
    {
      int ch = _ist.get();
      if(ch == '\r') 
      {
        if(_ist.good() && _ist.peek() == '\n') 
        {
          _ist.get();
        }
        ch = '\n';
      }
      switch(_state) 
      {
      case State::START:
        scanStateStart(ch);
        break;

      case State::WS_BEFORE_NEXT_COL:
        scanStateWhiteSpaceBeforeNextCol(ch);
        break;

      case State::NEXT_COL:
        scanStateNextCol(ch);
        break;
 
      case State::QUOTED_COL:
        scanStateQuotedCol(ch);
        break;
          
      case State::ESCAPED_COL:
        scanStateEscapedCol(ch); 
        break;

      case State::QUOTED_COL_RIGHT_WS:
        scanStateQuotedColRightWS(ch);
        break;

      case  State::UNQUOTED_COL:
        scanUnquotedCol(ch);
        break;

      case State::UNQUOTED_COL_RIGHT_WS:
        scanUnquotedColRightWS(ch);
        break;

      case State::COMMENT:
        scanStateComment(ch);          
        break;

      default:
        // error invalid _state
        // never should end here
        throw ParseError("Internal error: invalid state: " + 
                         std::to_string((int)_state) + 
                         " in csv parser.",
                         _current_input_line,
                         _current_input_column,
                         _csv_row,
                         _csv_column);
        break;
      }
      if(isNewline(ch)) 
      {
        _current_input_line++;
        _current_input_column = 0;
      }
      else 
      {
        _current_input_column++;
      }
    }
    else 
    {
      // error
      std::cout << "error 1" << std::endl;
      throw std::exception();
      return false;
    }
    return !_has_been_flushed;
  }
} // namespace csv
