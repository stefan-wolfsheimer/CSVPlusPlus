/******************************************************************************
Copyright (c) 2015-2018, Stefan Wolfsheimer

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
#include <locale>
#include <exception>
#include <sstream>
#include <typeindex>

namespace csv
{
  class BasicSerializerFailure : public ::std::exception
  {
  public:
    BasicSerializerFailure(const ::std::string & msg, const ::std::type_index & type)
      : _msg(msg), _type(type)
    {}

    virtual const char* what() const noexcept override
    {
      return _msg.c_str();
    }

    ::std::type_index getType() const
    {
      return _type;
    }

  private:
    ::std::string _msg;
    ::std::type_index _type;
  };


  template<typename CHAR, typename TRAITS, typename TARGET>
  class BasicSerializer
  {
  public:
    typedef CHAR                                            char_type;
    typedef TRAITS                                          char_traits;
    typedef TARGET                                          return_type;
    typedef std::basic_string<char_type, char_traits>       string_type;
    typedef std::basic_stringstream<char_type, char_traits> stream_type;

    static return_type as(stream_type & ss)
    {
      return_type value;
      ss >> value;
      if(ss.fail()) 
      {
        ::std::type_index ti(typeid(TARGET));
        throw BasicSerializerFailure(::std::string("Cannot convert cell content ") + ti.name(),
                                     ti);
      }
      while(!ss.eof())
      {
        int ch = ss.get();
        if(ss.eof()) break;
        if(ch != ' ' && ch != '\t') 
        {
          ::std::type_index ti(typeid(TARGET));
          throw BasicSerializerFailure(::std::string("Cannot convert cell content ") + ti.name(),
                                       ti);
        }
      }
      return value;
    }

    static return_type as(const string_type & str)
    {
      stream_type ss(str);
      return as(ss);
    }

    static return_type as(const string_type & str, const ::std::locale & locale)
    {
      stream_type ss(str);
      ss.imbue(locale);
      return as(ss);
    }
  };

  template<typename CHAR, typename TRAITS>
  class BasicSerializer<CHAR, TRAITS, std::basic_string<CHAR, TRAITS> >
  {
  public:
    typedef CHAR                                      char_type;
    typedef TRAITS                                    char_traits;
    typedef std::basic_string<char_type, char_traits> string_type;
    typedef std::basic_string<CHAR, TRAITS>           return_type;

    static return_type as(const string_type & str)
    {
      return str;
    }

    static return_type as(const string_type & str, const ::std::locale & locale)
    {
      return str;
    }
  };

}
