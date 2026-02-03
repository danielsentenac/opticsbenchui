/*******************************************************************
This file is part of OpticsBenchUI.

OpticsBenchUI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#ifndef DRIVERDEFINITION_H
#define DRIVERDEFINITION_H

#include <string>

// Set namespace
using namespace std;

class Driver;

/// Shared enums and types for actuator driver implementations.
namespace DriverDefinition
{
  /// Loop mode identifiers.
  enum { OPEN_LOOP = 0, CLOSED_LOOP = 1};
 
  /// Unit conversion direction.
  enum { DEFAULT_UNIT = 0, CUSTOM_UNIT =1 , FROM_DEFAULT_TO_CUSTOM = 2};

  /// Operation status codes.
  enum { OK = 0, NOT_CONFIGURED = 1, ACTUATOR_ALREADY_MOVING = 2,
         DISPLACEMENT_EXCEEDS_RANGE = 3, NOT_CLOSED_LOOP = 4, 
          NOT_OPEN_LOOP = 5 };

  /// Unit conversion function pointer.
  typedef int (*pConvertUnitType)(int unit, 
                                  float valueToConvert, 
                                  float& rconvertedValue);
  /// Driver feature metadata.
  typedef struct 
  {
    /// Motion unit.
    string unit;
    /// Custom motion unit.
    string customUnit;
    /// Range of motion.
    float  range;
    /// Custom range of motion.
    float  customRange;
    /// Closed-loop capability flag.
    int    closedLoop;
    /// Conversion operation.
    pConvertUnitType convertUnit;
  } DriverFeature; // Set of features of a kind of actuator steered 
                       // by a given type of driver.

  /// Available error codes.
  enum ADErrorCode { NO_ERROR, OTHER_ERROR, COMMUNICATION_ERROR, 
                             OUT_OF_RANGE_ERROR };

  /// Limit switch state.
  enum ADLimitSwitch { UPPER_LIMIT, LOWER_LIMIT, IN_BETWEEN, UNAVAILABLE };
}
#endif //_ADGLOBALDEFINITION_H_
