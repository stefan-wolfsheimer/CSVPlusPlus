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

#include <string>
#include <sstream>
#include <vector>

#include "csv/specification.h"
#include "csv/reader.h"

/* planet data from wikipedia:
http://en.wikipedia.org/wiki/Planet
*/
static const char * planets_csv =
R"--(
     Number, Name,     Diameter,  Mass, Orbital period, Rotation period
     1,      Mercury,  0.382,     0.06,   0.24,           58.64
     2,      Venus,    0.949,     0.82,   0.62,         -243.02
     3,      Earth,    1.00,      1.00,   1.00,            1.00
     4,      Mars,     0.532,     0.11,   1.88,            1.03
     5,      Jupiter, 11.209,   317.8,	 11.86,            0.41
     6,      Saturn,   9.449,    95.2,   29.46,            0.43
     7,      Uranus,   4.007,    14.6,   84.01,           -0.72
     8,      Neptune,  3.883,    17.2,  164.8,             0.67	
)--";

struct Planet
{
  int         index;
  std::string name;
  float       diameter;
  float       mass;
  float       orbital_period;
  float       rotation_period;
};

Planet parsePlanet(csv::Row row) 
{
  return Planet 
  {
    index           : row["Number"].as<int>(),
    name            : row["Name"].as<std::string>(),
    diameter        : row["Diameter"].as<float>(),
    mass            : row["Mass"].as<float>(),
    orbital_period  : row["Orbital period"].as<float>(),
    rotation_period : row["Rotation period"].as<float>()
  };
}

std::vector<Planet> readPlanetList(csv::Reader reader)
{
  std::vector<Planet> ret;
  for(auto row : reader) 
  {
    ret.push_back(parsePlanet(row));
  }
  return ret;
}

int main(int argc, const char ** argv)
{

  std::stringstream ist(planets_csv);
  csv::Reader reader(ist,
                     csv::Specification().withHeader());
  auto planets = readPlanetList(reader);
  Planet avg = Planet
  {
    index           : 0,
    name            : "avg",
    diameter        : 0,
    mass            : 0,
    orbital_period  : 0,
    rotation_period : 0
  };
  for(auto planet : planets) 
  {
    avg.diameter+=        planet.diameter / planets.size();
    avg.mass+=            planet.mass / planets.size();
    avg.orbital_period+=  fabs(planet.orbital_period) / planets.size();
    avg.rotation_period+= fabs(planet.rotation_period) / planets.size();
  }
  std::cout << "avg diameter        " << avg.diameter << std::endl;
  std::cout << "avg mass            " << avg.mass << std::endl;
  std::cout << "avg orbital_period  " << avg.orbital_period << std::endl;
  std::cout << "avg rotation_period " << avg.rotation_period << std::endl;
  return 0;
}
