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

#ifndef EVALUATIONREQUIREMENTPOSITIONDISTANCECONFIG_H
#define EVALUATIONREQUIREMENTPOSITIONDISTANCECONFIG_H

#include "configurable.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/position/distanceconfigwidget.h"
#include "eval/requirement/position/distance.h"

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class PositionDistanceConfig : public BaseConfig
{
public:
    PositionDistanceConfig(const std::string& class_id, const std::string& instance_id,
                        Group& group, EvaluationStandard& standard, EvaluationManager& eval_ma);
    virtual ~PositionDistanceConfig();

    std::shared_ptr<Base> createRequirement() override;

    float thresholdValue() const;
    void thresholdValue(float value);

    COMPARISON_TYPE thresholdValueCheckType() const;
    void thresholdValueCheckType(const COMPARISON_TYPE& type);

    bool failedValuesOfInterest() const;
    void failedValuesOfInterest(bool value);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

protected:
    float threshold_value_ {0};
    COMPARISON_TYPE threshold_value_check_type_ {COMPARISON_TYPE::LESS_THAN_OR_EQUAL};
    bool failed_values_of_interest_ {true};

    virtual void createWidget() override;
};

}

#endif // EVALUATIONREQUIREMENTPOSITIONDISTANCECONFIG_H
