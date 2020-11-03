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

#include "eval/requirement/base.h"

namespace EvaluationRequirement
{

Base::Base(const std::string& name, const std::string& short_name,
                                             const std::string& group_name, EvaluationManager& eval_man)
    : name_(name), short_name_(short_name), group_name_(group_name), eval_man_(eval_man)
{

}

std::string Base::name() const
{
    return name_;
}

std::string Base::shortname() const
{
    return short_name_;
}

std::string Base::groupName() const
{
    return group_name_;
}

}
