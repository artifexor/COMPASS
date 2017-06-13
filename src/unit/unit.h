/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UNIT_H_
#define UNIT_H_

#include "configurable.h"
#include "quantity.h"

#include <string>
#include <map>

/**
 * @brief Definition of a base unit
 *
 * Automatically registers to UnitManager, has a name (length, time), allows registering sub units with scaling factors.
 */
class Unit : public Configurable
{
public:
  /// @brief Constructor with a name
  Unit(const std::string &class_id, const std::string &instance_id, Quantity &parent)
      : Configurable (class_id, instance_id, &parent) {}
  /// @brief Destructor
  virtual ~Unit() {}

  virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id) { assert (false); }

  double factor () const { return factor_; }

private:
  /// Comment definition
  std::string definition_;
  /// Scaling factor
  double factor_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif /* UNIT_H_ */