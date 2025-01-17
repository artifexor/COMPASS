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

#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONFALSE_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONFALSE_H

#include "eval/requirement/base/base.h"

namespace EvaluationRequirement
{

class IdentificationFalse : public Base
{
public:
    IdentificationFalse(const std::string& name, const std::string& short_name, const std::string& group_name,
                        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
                        bool require_all_false, bool use_mode_a, bool use_ms_ta, bool use_ms_ti);

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

    bool requireAllFalse() const;

    bool useModeA() const;

    bool useMsTa() const;

    bool useMsTi() const;

protected:
    // true: all must be false, false: at least one must be false
    bool require_all_false_ {true};

    // mode a ssr code
    bool use_mode_a_ {true};
    // 24-bit mode s address
    bool use_ms_ta_ {true};
    // downlinked aircraft identification
    bool use_ms_ti_ {true};
};

}
#endif // EVALUATIONREQUIREMENTIDENTIFICATIONFALSE_H
