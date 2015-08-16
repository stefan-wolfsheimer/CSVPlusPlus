#include "csv/specification.h"
#include "csv/reader.h"

#include <iostream>
#include <string>
#include <fstream>

namespace 
{
  ::csv::Specification parseSpecificationFromArgv(int argc, 
                                                  const char ** argv, 
                                                  std::string & input_file);
}


int main(int argc, const char ** argv)
{
  std::string input_file;
  csv::Specification spec = parseSpecificationFromArgv(argc, argv, input_file);
  {
    std::ifstream ist(input_file.c_str());
    if(!ist.is_open()) 
    {
      std::cerr << "Open '" << input_file << "' failed" << std::endl;
      exit(8);
    }
    csv::Reader reader(ist, spec);
    for(auto row : reader) 
    {
      for(auto cell : row) 
      {
        std::cout << cell.row() << ":" << cell.column();
        if(spec.hasHeader()) 
        {
          std::cout << "[" << cell.name() << "]";
        }
        
        std::cout << "=" << cell.as<std::string>() << std::endl;
      }
    }
  }
  return 0;
}

namespace 
{
  // helper
  ::csv::Specification parseSpecificationFromArgv(int argc, 
                                                  const char ** argv, 
                                                  std::string & input_file) 
  {
    auto ret = ::csv::Specification()
      .withHeader()
      .withoutSeparator();
    std::string fname;
    bool show_help = false;
    bool error     = false;
    ret.withHeader(); 
    for(int i = 1; i < argc; i++) 
    {
      if( argv[i] == ::std::string("--use_empty_lines"))
      {
        ret.withUsingEmptyLines();
      }
      else if( argv[i] == ::std::string("-c") || 
               argv[i] == ::std::string("--comma") )
      
      {
        ret.withSeparator(',');
      }
      else if( argv[i] == ::std::string("-t") || 
               argv[i] == ::std::string("--tab") )
      {
        ret.withSeparator('\t');
      }
      else if( argv[i] == ::std::string("-s") || 
               argv[i] == ::std::string("--space") )
      {
        ret.withSeparator(' ');
      }
      else if( argv[i] == ::std::string("--semicolon") )
      {
        ret.withSeparator(';');
      }
      else if( argv[i] == ::std::string("--comment") )
      {
        ret.withComment(';');
      }
      else if( argv[i] == ::std::string("-n") ||
               argv[i] == ::std::string("--no-header") )
      {
        ret.withoutHeader(); 
      }
      else if( argv[i] == ::std::string("-h") || 
               argv[i] == ::std::string("--help") )
      {
        show_help = true;
      }
      else if(fname.empty()) 
      {
        fname = argv[i];
      }
      else 
      {
        error = true;
      }
    }
    if(fname.empty()) 
    {
      error = true;
    }
    if(show_help || error) 
    {
      std::cout << "usage:" << std::endl;
      std::cout << argv[0] << " OPTIONS input_file" << std::endl;
      std::cout << "OPTIONS:" << std::endl;
      std::cout << "-c | --comma:      use comma as separator" << std::endl;
      std::cout << "-t | --tab:        use tab as separator" << std::endl;
      std::cout << "-s | --space:      use space as separator" << std::endl;
      std::cout << "-w | --whitespace: use space or tab as separator" 
                << std::endl;      
      std::cout << "                   equivalent to: -s -t" << std::endl;
      std::cout << "--semicolon:       use semicolon as separator" << std::endl;
      std::cout << "--comment:         enable comment lines beginning with '#'" 
                << std::endl;
      std::cout << "-n | --no-header:  first line is not the header" 
                << std::endl;      
      exit(error ? -1 : 0);
    }
    if(!ret.hasSeparator())
    {
      ret.withSeparator(',');
    }
    input_file = fname;
    return ret;
  }
} // namespace


