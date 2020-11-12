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

#ifndef EVALUATIONREQUIREMENTIDENTIFICATION_H
#define EVALUATIONREQUIREMENTIDENTIFICATION_H

#include "eval/requirement/base.h"

namespace EvaluationRequirement
{
    class Identification : public Base
    {
    public:
        Identification(
                const std::string& name, const std::string& short_name, const std::string& group_name,
                EvaluationManager& eval_man, float max_ref_time_diff, float minimum_probability);

        float maxRefTimeDiff() const;
        float minimumProbability() const;

        virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
                const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
                const SectorLayer& sector_layer) override;

    protected:
        float max_ref_time_diff_ {0};
        float minimum_probability_{0};
    };

}
#endif // EVALUATIONREQUIREMENTDETECTION_H
