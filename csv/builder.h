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
#include <memory>
#include <vector>
#include <string>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include "csv_common.h"
#include "serializer.h"


namespace csv
{

template<typename OBJECT, typename CHAR, typename TRAITS>
class MemberInterface
{
public:
  typedef OBJECT                                     object_type;
  typedef CHAR                                       char_type;
  typedef TRAITS                                     char_traits;
  typedef std::basic_string<char_type, char_traits>  string_type;
  virtual ~MemberInterface() {}
  virtual void initialize(object_type & cls) const = 0;
  virtual void parse(object_type & cls, const string_type & str) const = 0;
  virtual void parse(object_type & cls, const string_type & str, const::std::locale & locale) const = 0;
  virtual string_type getName() const = 0;
  virtual void streamOut(::std::ostream & ost, const object_type & obj) = 0;
  virtual const std::type_info& getTypeInfo() const = 0;

  template<typename T> T& bind(object_type & obj) const
  {
    T * ptr = (T*)bindMember(obj);
    return (T&) *ptr;
  }

  template<typename T> const T& bind(const object_type & obj) const
  {
    const T * ptr = (const T*)bindMember(obj);
    return (T&) *ptr;
  }

protected:
  virtual void* bindMember(object_type & obj) const = 0;
  virtual const void* bindMember(const object_type & obj) const = 0;
};

template<typename OBJECT, typename MEMBER, typename CHAR, typename TRAITS>
class BasicMember : public MemberInterface<OBJECT, CHAR, TRAITS>
{
public:
  typedef OBJECT                                     object_type;
  typedef MEMBER                                     member_type;
  typedef CHAR                                       char_type;
  typedef TRAITS                                     char_traits;
  typedef member_type object_type::*pointer_to_member_type;
  typedef MemberInterface<object_type,
                          char_type,
                          char_traits>               parent_type;
  typedef typename parent_type::string_type          string_type;

  BasicMember(pointer_to_member_type pointerToMember,
              const string_type & name,
              const member_type & defaultValue)
    : _pointerToMember(pointerToMember),
      _name(name),
      _defaultValue(defaultValue)
  {}

  string_type getName() const override
  {
    return _name;
  }

  virtual void initialize(object_type & obj) const override
  {
    obj.*_pointerToMember = _defaultValue;
  }

  virtual void parse(object_type & obj, const string_type & str) const override
  {
    typedef BasicSerializer<char_type, char_traits, member_type> serializer_type;
    obj.*_pointerToMember = serializer_type::as(str);
  }

  virtual void parse(object_type & obj, const string_type & str, const ::std::locale & locale) const override
  {
    typedef BasicSerializer<char_type, char_traits, member_type> serializer_type;
    obj.*_pointerToMember = serializer_type::as(str, locale);
  }

  virtual void streamOut(::std::ostream & ost, const object_type & obj) override
  {
    ost << obj.*_pointerToMember;
  }

  const std::type_info& getTypeInfo() const override
  {
    return typeid(member_type);
  }

protected:
  void* bindMember(object_type & obj) const override
  {
    return &(obj.*_pointerToMember);
  }

  const void* bindMember(const object_type & obj) const override
  {
    return &(obj.*_pointerToMember);
  }

private:
  pointer_to_member_type _pointerToMember;
  string_type _name;
  member_type _defaultValue;
};

template<typename OBJECT, typename CHAR, typename TRAITS>
class BasicBuilder
{
public:
  typedef OBJECT                                         object_type;
  typedef CHAR                                           char_type;
  typedef TRAITS                                         char_traits;
  typedef MemberInterface<object_type,char_type,
                          char_traits>                    member_type;
  typedef typename member_type::string_type               string_type;
  typedef std::shared_ptr<member_type>                    shared_member_type;
  typedef std::vector<shared_member_type>                 member_container_type;
  typedef typename member_container_type::const_iterator  const_iterator;
  typedef BasicBuilder<object_type,char_type,char_traits> self_type;
  
  template<typename FIELD>
  inline self_type&
  member(typename BasicMember<object_type, FIELD, char_type, char_traits>::pointer_to_member_type ptr,
         const string_type & name,
         const FIELD & defaultValue)
  {
    typedef BasicMember<object_type, FIELD, char_type, char_traits> member_type;
    members.push_back(std::make_shared<member_type>(ptr, name, defaultValue));
    return *this;
  }
  
  inline object_type build() const
  {
    object_type value;
    for(auto memb : members)
    {
      memb->initialize(value);
    }
    return value;
  }

  inline object_type * buildPointer() const
  {
    object_type * ret = new object_type();
    for(auto memb : members)
    {
      memb->initialize(*ret);
    }
    return ret;
  }

  inline std::shared_ptr<object_type> buildShared() const
  {
    object_type * cls = buildPointer();
    return std::shared_ptr<object_type>(cls);
  }

  inline std::vector<string_type> getFieldNames() const
  {
    std::vector<string_type> ret;
    for(auto memb : members)
    {
      ret.push_back(memb->getName());
    }
    return ret;
  }

  inline const_iterator begin() const
  {
    return members.begin();
  }

  inline const_iterator end() const
  {
    return members.end();
  }

private:
  std::vector<shared_member_type> members;
};

template<typename CLS>
class Builder : public BasicBuilder<CLS, char, char_traits>
{
};

template<typename CLS>
class WBuilder : public BasicBuilder<CLS, wchar_t, wchar_traits>
{
};

}
