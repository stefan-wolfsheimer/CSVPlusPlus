#include <sstream>
#include <iostream>
#include <stdexcept>
#include "csv/reader.h"

class BenchmarkInputBuffer : public std::streambuf
{
public:
  BenchmarkInputBuffer(std::size_t n_rows, std::size_t n_cols)
  {
    std::stringstream ost;
    for(std::size_t i = 0; i < n_cols; i++) 
      {
        ost << "  " << i << " ";
      }
    _buffer  = ost.str();
    _current = _buffer.begin();
    _n_rows  = n_rows;
    _row     = 1;
  }

private:
  std::string                 _buffer;
  std::string::const_iterator _current;
  std::size_t                 _n_rows;
  std::size_t                 _row;

  int_type underflow() override 
  {
    if (_current == _buffer.end())
      {
        if(_row < _n_rows) 
          {
            return traits_type::to_int_type('\n');
          }
        else 
          {
            return traits_type::eof();
          }
      }
    else 
      {
        return traits_type::to_int_type(*_current);
      }
  }

  int_type uflow() override 
  {
    if (_current == _buffer.end())
      {
        if(_row < _n_rows) 
          {
            _row++;
            _current = _buffer.begin();
            return traits_type::to_int_type('\n');
          }
        else 
          {
            return traits_type::eof();
          }
      }
    else 
      {
        return traits_type::to_int_type(*_current++);
      }
  }

  int_type pbackfail(int_type ch)
  {
    throw std::logic_error("pbackfail not expected to be called");
  }
  
  std::streamsize showmanyc()
  {
    return _buffer.size() * _n_rows;
  }
};


std::size_t readFromIstream(std::size_t nrows, std::size_t ncols) 
{
  BenchmarkInputBuffer buffer(nrows,ncols);
  {
    std::istream ist(&buffer);
    int a;
    int hash = 0;
    std::size_t counter = 0;
    while(ist >> a)
      {
        hash+=a;
        counter++;
      }
    return counter;
  }
}

std::size_t readFromCsvReader(std::size_t nrows, std::size_t ncols) 
{
  BenchmarkInputBuffer buffer(nrows,ncols);
  {
    std::istream ist(&buffer);
    csv::Reader reader(ist,
                       csv::Specification()
                       .withSeparator(' ')
                       .withSeparator(' ')
                       .withoutHeader());
    
    int a;
    int hash = 0;
    std::size_t counter = 0;
    std::size_t row_counter = 0;
    for(auto row : reader) 
      {
        for(auto cell : row) 
          {
            a = cell.as<int>();
            hash+= a;
            counter++;
          }
        row_counter++;
      }
    return counter;
  }
}


int main(int argc, const char ** argv)
{
  bool ok = true;
  std::function<std::size_t(std::size_t, std::size_t)> exe;
  std::size_t nrows = 0;
  std::size_t ncols = 0;
  if(argc < 4) 
    {
      ok = false;
    }
  else 
    {
      if(argv[1] == std::string("std")) 
        {
          exe = readFromIstream;
        }
      else if(argv[1] == std::string("csv"))
        {
          exe = readFromCsvReader;
        }
      else
        {
          ok = false;
        }
      try 
        {
          nrows = std::stoi(argv[2]);
          ncols = std::stoi(argv[3]);
        }
      catch(std::exception ex)
        {
          std::cerr << ex.what() << std::endl;
          ok = false;
        }
    }
  if(!ok) 
    {
      std::cerr << "run test as " << argv[0] << " std|csv nrows ncols" << std::endl;
      return 8;
    }
  std::size_t n = exe(nrows, ncols);
  std::cout << n << " values read with method " << argv[1] << std::endl;
  return 0;
}
