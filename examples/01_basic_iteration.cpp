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

#include "csv/specification.h"
#include "csv/reader.h"
#include <sstream>
/* planet data from wikipedia:
http://en.wikipedia.org/wiki/Planet
*/
static const char * planets_csv =
R"--(Number,Name,Equatorial diameter ,Mass ,Orbital period ,Rotation period
1,      Mercury,	0.382,	             0.06,	0.24             58.64
2,      Venus,    0.949,	             0.82,	0.62             −243.02
3,      Earth ,    1.00,	             1.00,	1.00             1.00
4,      Mars,     0.532,               0.11,	1.88             1.03
5,      Jupiter,  11.209,	             317.8,	11.86            0.41
6,      Saturn,   9.449,               95.2,  29.46            0.43
7,      Uranus,   4.007,               14.6,  84.01            −0.72
8,      Neptune,  3.883,               17.2,  164.8            0.67	
)--";

int main(int argc, const char ** argv)
{
  std::stringstream ist(planets_csv);
  csv::Reader reader(ist,
                     csv::Specification().withHeader());
  for(auto row : reader) 
  {
    for(auto cell : row) 
    {
      std::cout << cell.row() << ":" << cell.column();
      std::cout << "[" << cell.name() << "]";
      std::cout << "=" << cell.as<std::string>() << std::endl;
    }
  }
  return 0;
}
