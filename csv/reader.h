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
#include <functional>
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
      BasicReader * _reader;
      iterator(BasicReader * reader);
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
    bool readRow(row_type & row);


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
      START_COMMENT,
      END
    };
  protected:
    typedef typename row_type::buffer_type        buffer_type;
    typedef typename row_type::shared_buffer_type shared_buffer_type;
    typedef typename row_type::shared_spec_type   shared_spec_type;
    typedef typename row_type::cell_type          cell_type;

    istream_type                                & _ist;
    shared_spec_type                              _specs;
    char_type                                     _quote;

    // buffer
    row_type                                      _row_buffer[3];
    ::std::size_t                                 _row_buffer_state;
    State                                         _state;
    ::std::size_t                                 _current_input_line;
    ::std::size_t                                 _current_input_column;
    ::std::size_t                                 _next_input_line;
    ::std::size_t                                 _next_input_column;
    ::std::size_t                                 _csv_row;

    inline bool isWhiteSpace(int ch);
    inline bool isNewline(int ch);
    inline bool isQuote(int ch);
    inline bool isEof(int ch);

    inline void addCell();
    inline void addEmptyCell();
    bool scanStateStart(                   int ch, row_type & row, 
                                           bool & valid);
    bool scanStateNextCol(                 int ch, row_type & row);
    bool scanStateUnquotedCol(             int ch, row_type & row);
    bool scanStateUnquotedColRightWS(      int ch, row_type & row);

    bool scanStateQuotedCol(               int ch, row_type & row);
    bool scanStateEscapedCol(              int ch, row_type & row);
    bool scanStateQuotedColRightWS(        int ch, row_type & row);
    bool scanStateWhiteSpaceBeforeNextCol( int ch, row_type & row);
    bool scanStateStartComment(            int ch, row_type & row);
    bool scanStateComment(                 int ch, row_type & row);

  public:
    inline std::size_t inputLine() { return _current_input_line; }
    inline std::size_t inputColumn() { return _current_input_column; }
  };


  ///////////////////////////////////////////////////////////////
  //
  // Implementation
  //
  ///////////////////////////////////////////////////////////////


  // iterator
  template<typename CHAR, typename TRAITS>
  BasicReader<CHAR,TRAITS>::iterator::iterator(BasicReader * reader) 
    : _reader(reader)
  {
    if(reader->_row_buffer_state == 3) 
    {
      _reader = nullptr;
    }
    else 
    {
      _reader = reader;
    }
  }

  template<typename CHAR, typename TRAITS>
  inline const typename BasicReader<CHAR,TRAITS>::row_type& 
  BasicReader<CHAR,TRAITS>::iterator::operator*()  const 
  { 
    return _reader->_row_buffer[_reader->_row_buffer_state];  
  }

  template<typename CHAR, typename TRAITS>
  inline const typename BasicReader<CHAR,TRAITS>::row_type* 
  BasicReader<CHAR,TRAITS>::iterator::operator->() const 
  { 
    return &_reader->_row_buffer[_reader->_row_buffer_state];  
  }

  template<typename CHAR, typename TRAITS>
  BasicReader<CHAR,TRAITS>::iterator::iterator() : _reader(nullptr) 
  {
  }

  template<typename CHAR, typename TRAITS>
  inline typename BasicReader<CHAR,TRAITS>::iterator& 
  BasicReader<CHAR,TRAITS>::iterator::operator++() 
  {
    if(_reader && _reader->_row_buffer_state != 3) 
    {
      if(_reader->readRow(_reader->_row_buffer[1-_reader->_row_buffer_state])) 
      {
        _reader->_row_buffer_state = 1-_reader->_row_buffer_state;
      }
      else 
      {
        _reader->_row_buffer_state = 3;
        _reader = nullptr;
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
    return _reader == rhs._reader; 
  }
      
  template<typename CHAR, typename TRAITS>
  inline bool 
  BasicReader<CHAR,TRAITS>::iterator::operator!=(const iterator & rhs) 
  { 
    return _reader != rhs._reader; 
  }

  // Basic reader
  template<typename CHAR, typename TRAITS>
  BasicReader<CHAR,TRAITS>::BasicReader( istream_type       & ist,
                                         spec_type            specs )
    : _ist(ist),_specs(::std::make_shared<spec_type>(specs))
  {
    _state                          = State::START;
    _row_buffer[0]._shared_spec     = _specs;
    _row_buffer[1]._shared_spec     = _specs;
    _row_buffer_state               = 0;
    _quote                          = '"';
    _current_input_line             = 0;
    _current_input_column           = 0;
    _next_input_line                = 0;
    _next_input_column              = 0;
    _csv_row                        = 0;

    if(!readRow(_row_buffer[_row_buffer_state])) 
    {
      _row_buffer_state = 3;
    }
    else if(_specs->hasHeader()) 
    {
      std::size_t column = 0;
      for(auto & col : _row_buffer[_row_buffer_state]) 
      {
        if(!_specs->addColumnIfNotEmpty(column, 
                                        col.template as<string_type>()))
        {
          auto res = _specs->_lookup.find(col.template as<string_type>());
          ::std::size_t def_column = 0;
          if(res != _specs->_lookup.end()) 
          {
            def_column = res->second->index();
          }
          throw DuplicateColumnError("Column already defined.",
                                     def_column,
                                     _row_buffer[_row_buffer_state].inputLine(),
                                     col.inputColumn(),
                                     col.row(),
                                     col.column());
        }
        column++;
      }
      if(!readRow(_row_buffer[_row_buffer_state])) 
      {
        _row_buffer_state = 3;
      }
    }
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
  bool 
  BasicReader<CHAR,TRAITS>::readRow(row_type & row)
  {
    bool ok = true;
    bool valid = false;
    row.clear(_current_input_line, _csv_row);
    while(ok) 
    {
      if(_state == State::END) 
      {
        return false;
      }
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
        ok = scanStateStart(ch, row, valid);
        break;
      case State::QUOTED_COL:
        ok = scanStateQuotedCol(ch, row);
        break;
      case State::ESCAPED_COL:
        ok = scanStateEscapedCol(ch, row);
        break;
      case State::NEXT_COL:
        ok = scanStateNextCol(ch, row);
        break;
      case State::WS_BEFORE_NEXT_COL:
        ok = scanStateWhiteSpaceBeforeNextCol(ch, row);
        break;
      case State::QUOTED_COL_RIGHT_WS:
        ok = scanStateQuotedColRightWS(ch, row);
        break;
      case State::UNQUOTED_COL:
        ok = scanStateUnquotedCol(ch, row);
        break;
      case State::UNQUOTED_COL_RIGHT_WS:
        ok = scanStateUnquotedColRightWS(ch,row);
        break;
      case State::START_COMMENT:
        ok = scanStateStartComment(ch, row);          
        break;
      case State::COMMENT:
        ok = scanStateComment(ch, row);          
        break;
      case State::END:
        ok = false;
        break;
      }
      if(ch == '\n') 
      {
        _current_input_column = 0;
        _current_input_line++;
      }
      else if(!isEof(ch))
      {
        _current_input_column++;
      }
    }
    if(valid) 
    {
      row._shared_buffer->_csv_row = _csv_row;
      _csv_row++;
    }
    return valid;
  }


  template<typename CHAR, typename TRAITS>
  bool 
  BasicReader<CHAR,TRAITS>::scanStateStart(int ch, 
                                           row_type & row, 
                                           bool & valid) 
  {
    if(isWhiteSpace(ch)) 
    {
      /* stay in state start */
      return true;
    }
    else if( _specs->isSeparator(ch) ) 
    {
      row.addCell(_next_input_line, _next_input_column);
      row.closeCell();
      valid              = true;
      _next_input_line   = _current_input_line;
      _next_input_column = _current_input_column + 1;
      _state             = State::NEXT_COL;
      return true;
    }
    else if( isNewline(ch) )
    {
      _next_input_line   = _current_input_line+1;
      _next_input_column = 0;
      if( _specs->hasUsingEmptyLines() ) 
      {
        valid = true;
        return false;
      }
      else 
      {
        return true;
      }
    }
    else if( isEof(ch) ) 
    {
      _state = State::END;
      return false;
    }
    else if( isQuote(ch) ) 
    {
      row.addCell(_current_input_line, _current_input_column);
      valid  = true;
      _state = State::QUOTED_COL;
      return true;
    }
    else if( _specs->isComment(ch))
    {
      _state = State::START_COMMENT;
      if( _specs->hasUsingEmptyLines() ) 
      {
        valid = true;
      }
      return true;
    }
    else 
    {
      row.addCell(_current_input_line, _current_input_column);
      row.addChar(ch);
      valid = true;
      _state = State::UNQUOTED_COL;
      return true;
    }
  }

  template<typename CHAR, typename TRAITS>
  bool 
  BasicReader<CHAR,TRAITS>::scanStateWhiteSpaceBeforeNextCol( int ch, 
                                                              row_type & row)
  {
    if(isWhiteSpace(ch)) 
    {
      // stay in state
      return true;
    }
    else if( _specs->isSeparator(ch) ) 
    {
      row.addCell(_next_input_line, _next_input_column);
      row.closeCell();
      _next_input_line   = _current_input_line;
      _next_input_column = _current_input_column + 1;
      _state             = State::NEXT_COL;
      return true;
    }
    else if( isNewline(ch) ) 
    {
      _next_input_column   = 0;
      _next_input_line = _current_input_line + 1;
      _state = State::START;
      return false;
    }
    else if( _specs->isComment(ch))
    {
      _next_input_column   = 0;
      _next_input_line = _current_input_line + 1;
      _state         = State::COMMENT;
      return true;
    }
    else if( isEof(ch) ) 
    {
      _state = State::END;
      return false;
    }
    else if( isQuote(ch) ) 
    {
      row.addCell(_current_input_line, _current_input_column);
      _state = State::QUOTED_COL;
      return true;
    }
    else 
    {
      row.addCell(_current_input_line, _current_input_column);
      row.addChar(ch);
      _state = State::UNQUOTED_COL;
      return true;
    }
    return false;
  }

  template<typename CHAR, typename TRAITS>
  bool BasicReader<CHAR,TRAITS>::scanStateNextCol(int ch, row_type & row) 
  {
    if(isWhiteSpace(ch)) 
    {
      // stay in state
      return true;
    }
    else if( _specs->isSeparator(ch) ) 
    {
      row.addCell(_next_input_line, _next_input_column);
      row.closeCell();
      _next_input_line   = _current_input_line;
      _next_input_column = _current_input_column+1;
      _state = State::NEXT_COL;
      return true;
    }
    else if( isNewline(ch) ) 
    {
      row.addCell(_next_input_line, _next_input_column);
      row.closeCell();
      _next_input_line   = _current_input_line+1;
      _next_input_column = 0;
      _state = State::START;
      return false;
    }
    else if( _specs->isComment(ch))
    {
      // empty cell
      row.addCell(_next_input_line, _next_input_column);
      row.closeCell();
      _next_input_line   = _current_input_line+1;
      _next_input_column = 0;
      _state  = State::COMMENT;
      return true;
    }
    else if( isEof(ch) ) 
    {
      row.addCell(_next_input_line, _next_input_column);
      row.closeCell();
      _state = State::END;
      return false;
    }
    else if( isQuote(ch) ) 
    {
      _next_input_column = _current_input_column;
      _next_input_line = _current_input_line;
      row.addCell(_next_input_line, _next_input_column);
      _state = State::QUOTED_COL;
      return true;
    }
    else 
    {
      row.addCell(_current_input_line, _current_input_column);
      row.addChar(ch);
      _state = State::UNQUOTED_COL;
      return true;
    }
  }

  template<typename CHAR, typename TRAITS>
  bool BasicReader<CHAR,TRAITS>::scanStateQuotedCol(int ch, row_type & row) 
  {
    if(isQuote(ch)) 
    {
      _state = State::ESCAPED_COL;
      return true;
    }
    else if(!isEof(ch)) 
    {
      row.addChar(ch);
      return true;
    }
    else 
    {
      throw ParseError("Unexpected end of file, expected '\"'.",
                       _current_input_line,
                       _current_input_column,
                       _csv_row,
                       row.size());
    }
    return false;
  }

  template<typename CHAR, typename TRAITS>
  bool BasicReader<CHAR,TRAITS>::scanStateEscapedCol(int ch, row_type & row) 
  {
    if(isQuote(ch)) 
    {
      row.addChar(_quote);
      _state = State::QUOTED_COL;
      return true;
    }
    else if(_specs->isSeparator(ch)) 
    {
      row.closeCell();
      _next_input_column  = _current_input_column+1;
      _next_input_line    = _current_input_line;
      row.addCell(_next_input_line,_next_input_column);
      if( isWhiteSpace(ch) ) 
      {
        _state = State::WS_BEFORE_NEXT_COL;
      }
      else 
      {
        _state = State::NEXT_COL;
      }
      return true;
    }
    else if(isWhiteSpace(ch))
    {
      row.closeCell();
      _state = State::QUOTED_COL_RIGHT_WS;
      return true;
    }
    else if(isNewline(ch)) 
    {
      row.closeCell();
      _next_input_line   = _current_input_line + 1;;
      _next_input_column = 0;
      _state = State::START;
      return false;
    }
    else if( _specs->isComment(ch))
    {
      row.closeCell();
      _next_input_line   = _current_input_line + 1;;
      _next_input_column = 0;
      _state = State::COMMENT;
      return true;
    }
    else if(isEof(ch)) 
    {
      row.closeCell();
      _state = State::END;
      return false;
    }
    else 
    {
      throw ParseError("Unexpected character after end of quoted cell.",
                       _current_input_line,
                       _current_input_column,
                       _csv_row,
                       row.size());
    }
    return false;
  }

  template<typename CHAR, typename TRAITS>
  bool 
  BasicReader<CHAR,TRAITS>::scanStateQuotedColRightWS(int ch, row_type & row)
  {
    if( isWhiteSpace(ch) )
    {
      return true;
    }
    else if( _specs->isSeparator(ch) ) 
    {
      _next_input_column  = _current_input_column+1;
      _next_input_line    = _current_input_line;
      row.addCell(_next_input_line,_next_input_column);
      if( isWhiteSpace(ch) ) 
      {
        _state = State::WS_BEFORE_NEXT_COL;
      }
      else 
      {
        _state = State::NEXT_COL;
      }
      return true;
    }
    else if( isNewline(ch) ) 
    {
      _next_input_column   = 0;
      _next_input_line = _current_input_line + 1;
      _state = State::START;
      return false;
    }
    else if( _specs->isComment(ch))
    {
      _state = State::COMMENT;
      return true;
    }
    else if( isEof(ch) )
    {
      _state = State::END;
      return false;
    }
    else 
    {
      throw ParseError("Unexpected character at the end of quoted cell.",
                       _current_input_line,
                       _current_input_column,
                       _csv_row,
                       (row.size()?row.size()-1:0));
    }
    return true;
  }

  template<typename CHAR, typename TRAITS>
  bool BasicReader<CHAR,TRAITS>::scanStateUnquotedCol(int ch, row_type & row)
  {
    if(_specs->isSeparator(ch)) 
    {
      row.closeCell();
      _next_input_column  = _current_input_column+1;
      _next_input_line    = _current_input_line;
      if( isWhiteSpace(ch) ) 
      {
        _state = State::WS_BEFORE_NEXT_COL;
      }
      else 
      {
        _state = State::NEXT_COL;
      }
      return true;
    }
    else if( isWhiteSpace(ch) ) 
    {
      row.flushCell();
      row.addChar(ch);
      _state = State::UNQUOTED_COL_RIGHT_WS;
      return true;
    }
    else if(isNewline(ch)) 
    {
      row.closeCell();
      _state = State::START;
      return false;
    }
    else if( _specs->isComment(ch))
    {
      row.closeCell();
      _state = State::COMMENT;
      return true;
    }
    else if(isEof(ch)) 
    {
      row.closeCell();
      _state = State::END;
      return false;
    }
    else 
    {
      row.addChar(ch);
      return true;
    }
  }

  template<typename CHAR, typename TRAITS>
  bool 
  BasicReader<CHAR,TRAITS>::scanStateUnquotedColRightWS(int ch, 
                                                        row_type & row)
  {
    if(isWhiteSpace(ch)) 
    {
      row.addChar(ch);
      return true;
    }
    else if(_specs->isSeparator(ch))
    {
      row.revertCell();
      row.closeCell();
      _next_input_column  = _current_input_column+1;
      _next_input_line    = _current_input_line;
      row.addCell(_next_input_line,_next_input_column);
      _state = State::NEXT_COL;
      return true;
    }
    else if(isNewline(ch)) 
    {
      row.revertCell();
      row.closeCell();
      _state = State::START;
      return false;
    }
    else if( _specs->isComment(ch))
    {
      row.revertCell();
      row.closeCell();
      _state = State::COMMENT;
      return true;
    }
    else if(isEof(ch))
    {
      row.revertCell();
      row.closeCell();
      _state = State::END;
      return false;
    }
    else 
    {
      row.addChar(ch);
      _state = State::UNQUOTED_COL;
      return true;
    }
  }

  template<typename CHAR, typename TRAITS>
  bool BasicReader<CHAR,TRAITS>::scanStateStartComment(int ch, row_type & row)
  {
    if(isNewline(ch)) 
    {
      _state = State::START;
      return ! _specs->hasUsingEmptyLines();
    }
    else if(isEof(ch))
    {
      _state = State::END;
      return false;
    }
    else 
    {
      return true;
    }
  }

  template<typename CHAR, typename TRAITS>
  bool BasicReader<CHAR,TRAITS>::scanStateComment(int ch, row_type & row)
  {
    if(isNewline(ch)) 
    {
      _state = State::START;
      return false;
    }
    else if(isEof(ch))
    {
      _state = State::END;
      return false;
    }
    else 
    {
      return true;
    }
  }

} // namespace csv
