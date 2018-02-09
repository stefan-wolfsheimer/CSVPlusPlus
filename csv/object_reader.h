#pragma once
#include "builder.h"
#include "reader.h"

namespace csv
{
  template<typename CLASS, typename CHAR, typename TRAITS>
  class BasicObjectReader
  {
  public:
    typedef CLASS                                        object_type;
    typedef CHAR                                         char_type;
    typedef TRAITS                                       char_traits;
    typedef std::basic_string<char_type, char_traits>    string_type;
    typedef BasicReader<char_type, char_traits>          reader_type;
    typedef typename reader_type::row_type               row_type;
    typedef typename row_type::cell_type                 cell_type;
    typedef typename row_type::spec_type                 spec_type;
    typedef ::std::basic_istream<char_type,
                                 char_traits>            istream_type;
    typedef BasicBuilder<object_type,
                         char_type,
                         char_traits>                    builder_type;

    BasicObjectReader(const builder_type & builder,
                      istream_type & ist,
                      spec_type specs = spec_type()) :
      _reader(ist, specs),
      _builder(std::make_shared<builder_type>(builder))
    {
    }

    static void map(object_type & obj,
                    const builder_type & builder,
                    const row_type & row)
    {
      for(auto field : builder)
      {
        auto itr = row.find(field->getName());
        if(itr != row.end())
        {
          const cell_type & col(*itr);
          try
          {
            field->parse(obj,
                         string_type(col.begin(), col.end()),
                         col.specification()->locale());
          }
          catch(BasicSerializerFailure failure)
          {
            
            throw ConversionError(::std::string(failure.what()),
                                  failure.getType(),
                                  col.inputLine(),
                                  col.inputColumn(),
                                  col.row(),
                                  col.column());
          }
        }
      }
    }

    class iterator : public ::std::iterator<::std::input_iterator_tag, object_type>
    {
      friend class BasicObjectReader<object_type, char_type, char_traits>;
      typedef typename reader_type::iterator reader_iterator;
      reader_iterator _itr;
      ::std::shared_ptr<object_type> _obj;
      ::std::shared_ptr<builder_type> _builder;

      iterator(reader_iterator itr,
               std::shared_ptr<builder_type> builder) :
        _itr(itr),
        _builder(builder)
      {
        if(_itr != reader_iterator())
        {
          _obj.reset(new object_type());
          BasicObjectReader::map(*_obj, *_builder, *_itr);
        }
      }

    public:
      iterator() : _itr(reader_iterator())
      {
        _obj = nullptr;
      }

      inline iterator& operator++()
      {
        ++_itr;
        _obj.reset(new object_type());
        BasicObjectReader::map(*_obj, *_builder, *_itr);
        return *this;
      }

      inline iterator operator++(int)
      {
        iterator tmp = *this;
        ++*this;
        return tmp;
      }

      inline const object_type&  operator*()  const
      {
        return *_obj;
      }

      inline const object_type* operator->() const
      {
        return _obj.get();
      }

      inline bool operator==(const iterator & rhs) const
      {
        return _itr == rhs._itr;
      }

      inline bool operator!=(const iterator & rhs) const
      {
        return _itr != rhs._itr;
      }
    };

    inline iterator begin() { return iterator(_reader.begin(), _builder); }
    inline iterator end()   { return iterator(_reader.end(), _builder); }

  private:
    ::std::shared_ptr<builder_type> _builder;
    reader_type _reader;
  };

  template<typename CLASS>
  class ObjectReader : public BasicObjectReader<CLASS, char, char_traits>
  {
  public:
    typedef BasicObjectReader<CLASS, char, char_traits> parent_type;
    typedef typename parent_type::builder_type builder_type;
    typedef typename parent_type::istream_type istream_type;
    typedef typename parent_type::spec_type spec_type;

    ObjectReader(const builder_type & _builder,
                 istream_type & _ist,
                 spec_type _specs = spec_type()) :
      parent_type(_builder, _ist, _specs) {}
  };

}

