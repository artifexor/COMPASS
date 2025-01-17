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

#include "eval/requirement/identification/falseconfig.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "stringconv.h"

using namespace Utils;
using namespace EvaluationResultsReport;
using namespace std;

namespace EvaluationRequirement
{

IdentificationFalseConfig::IdentificationFalseConfig(const std::string& class_id, const std::string& instance_id,
                                                     Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("require_all_false", &require_all_false_, true);

    registerParameter("use_mode_a", &use_mode_a_, true);
    registerParameter("use_ms_ta", &use_ms_ta_, true);
    registerParameter("use_ms_ti", &use_ms_ti_, true);
}

std::shared_ptr<Base> IdentificationFalseConfig::createRequirement()
{
    shared_ptr<IdentificationFalse> req = make_shared<IdentificationFalse>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_,
                require_all_false_, use_mode_a_, use_ms_ta_, use_ms_ti_);

    return req;
}

void IdentificationFalseConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new IdentificationFalseConfigWidget(*this));
    assert (widget_);
}

bool IdentificationFalseConfig::requireAllFalse() const
{
    return require_all_false_;
}

void IdentificationFalseConfig::requireAllFalse(bool value)
{
    require_all_false_ = value;
}

bool IdentificationFalseConfig::useModeA() const
{
    return use_mode_a_;
}

void IdentificationFalseConfig::useModeA(bool value)
{
    use_mode_a_ = value;
}

bool IdentificationFalseConfig::useMsTa() const
{
    return use_ms_ta_;
}

void IdentificationFalseConfig::useMsTa(bool value)
{
    use_ms_ta_ = value;
}

bool IdentificationFalseConfig::useMsTi() const
{
    return use_ms_ti_;
}

void IdentificationFalseConfig::useMsTi(bool value)
{
    use_ms_ti_ = value;
}

void IdentificationFalseConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Probability [1]", "Probability of false identification",
                  roundf(prob_ * 10000.0) / 100.0}, nullptr);
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_).c_str()}, nullptr);

    table.addRow({"Require All False",
                  "If checked, all available secondary attributes be different than in "
                  " the reference to count. If not checked, a single wrong secondary attribute is enough.",
                  String::boolToString(require_all_false_).c_str()}, nullptr);

    table.addRow({"Use Mode 3/A Code",
                  "If the Mode 3/A code should be checked",
                  String::boolToString(use_mode_a_).c_str()}, nullptr);

    table.addRow({"Use Mode S Target Address",
                  "If the Mode S target address should be checked",
                  String::boolToString(use_ms_ta_).c_str()}, nullptr);

    table.addRow({"Use Mode S Target Identification",
                  "If the Mode S target identification should be checked",
                  String::boolToString(use_ms_ti_).c_str()}, nullptr);
}

}
