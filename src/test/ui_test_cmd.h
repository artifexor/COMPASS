/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "rtcommand.h"

namespace ui_test
{

/**
 */
struct RTCommandUISet : public rtcommand::RTCommandObjectValue 
{
    DECLARE_RTCOMMAND(uiset, "sets an ui element to the given value")
protected:
    virtual bool run_impl() const override;
};

/**
 */
struct RTCommandUIGet : public rtcommand::RTCommandObjectValue 
{
    DECLARE_RTCOMMAND(uiget, "retrieves the value of the given ui element")
protected:
    virtual bool run_impl() const override;
};

inline void initUITestCommands()
{
    RTCommandUISet::init();
    RTCommandUIGet::init();
}

} // namespace ui_test
