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

#include "eval/requirement/speed/speed.h"
#include "eval/results/speed/speedsingle.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

#include <ogr_spatialref.h>

#include <algorithm>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

Speed::Speed(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
        float threshold_value, bool use_percent_if_higher, float threshold_percent,
        COMPARISON_TYPE threshold_value_check_type, bool failed_values_of_interest)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      threshold_value_(threshold_value),
      use_percent_if_higher_(use_percent_if_higher), threshold_percent_(threshold_percent),
      threshold_value_check_type_(threshold_value_check_type),
      failed_values_of_interest_(failed_values_of_interest)
{

}

float Speed::thresholdValue() const
{
    return threshold_value_;
}

bool Speed::usePercentIfHigher() const
{
    return use_percent_if_higher_;
}

float Speed::thresholdPercent() const
{
    return threshold_percent_;
}

COMPARISON_TYPE Speed::thresholdValueCheckType() const
{
    return threshold_value_check_type_;
}

bool Speed::failedValuesOfInterest() const
{
    return failed_values_of_interest_;
}

std::shared_ptr<EvaluationRequirementResult::Single> Speed::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementSpeed '" << name_ << "': evaluate: utn " << target_data.utn_
           << " threshold_value " << threshold_value_ << " threshold_value_check_type " << threshold_value_check_type_;

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

    const std::multimap<ptime, unsigned int>& tst_data = target_data.tstData();

    unsigned int num_pos {0};
    unsigned int num_no_ref {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_calc_errors {0};
    unsigned int num_no_tst_value {0};
    unsigned int num_comp_failed {0};
    unsigned int num_comp_passed {0};

    float tmp_threshold_value;

    std::vector<EvaluationRequirement::SpeedDetail> details;

    ptime timestamp;

    OGRSpatialReference wgs84;
    wgs84.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference local;

    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;

    EvaluationTargetPosition tst_pos;

    bool is_inside;
    EvaluationTargetPosition ref_pos;
    bool ok;

    EvaluationTargetVelocity ref_spd;
    float tst_spd_ms;
    float spd_diff;

    bool comp_passed;

    unsigned int num_speeds {0};
    string comment;

    vector<double> values;

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();

    bool has_ground_bit;
    bool ground_bit_set;

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        timestamp = tst_id.first;
        tst_pos = target_data.tstPosForTime(timestamp);

        comp_passed = false;

        if (!target_data.hasRefDataForTime (timestamp, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No reference data"});

            ++num_no_ref;
            continue;
        }

        tie(ref_pos, ok) = target_data.interpolatedRefPosForTime(timestamp, max_ref_time_diff);

        if (!ok)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No reference position"});

            ++num_no_ref;
            continue;
        }

        has_ground_bit = target_data.hasTstGroundBitForTime(timestamp);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(timestamp);
        else
            ground_bit_set = false;

        if (!ground_bit_set)
            tie(has_ground_bit, ground_bit_set) = target_data.interpolatedRefGroundBitForTime(timestamp, seconds(15));

        is_inside = sector_layer.isInside(ref_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   true, ref_pos, // has_ref_pos, ref_pos
                                   is_inside, {}, comp_passed, // pos_inside, value, check_passed
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "Outside sector"});
            ++num_pos_outside;
            continue;
        }

        tie (ref_spd, ok) = target_data.interpolatedRefPosBasedSpdForTime(timestamp, max_ref_time_diff);

        if (!ok)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No reference speed"});

            ++num_no_ref;
            continue;
        }

        ++num_pos_inside;

        // ref_spd ok

        if (!target_data.hasTstMeasuredSpeedForTime(timestamp))
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No tst speed"});

            ++num_no_tst_value;
            continue;
        }

        tst_spd_ms = target_data.tstMeasuredSpeedForTime (timestamp);
        spd_diff = fabs(ref_spd.speed_ - tst_spd_ms);

        if (use_percent_if_higher_ && tst_spd_ms * threshold_percent_ > threshold_value_) // use percent based threshold
            tmp_threshold_value = tst_spd_ms * threshold_percent_;
        else
            tmp_threshold_value = threshold_value_;

        ++num_speeds;

        if (compareValue(spd_diff, tmp_threshold_value, threshold_value_check_type_))
        {
            comp_passed = true;
            ++num_comp_passed;
            comment = "Passed";
        }
        else
        {
            ++num_comp_failed;
            comment = "Failed";
        }

        details.push_back({timestamp, tst_pos,
                           true, ref_pos,
                           is_inside, spd_diff, comp_passed, // pos_inside, value, check_passed
                           num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                           num_comp_failed, num_comp_passed,
                           comment});

        values.push_back(spd_diff);
    }

    //        logdbg << "EvaluationRequirementSpeed '" << name_ << "': evaluate: utn " << target_data.utn_
    //               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
    //               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
    //               << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok
    //               << " num_speeds " << num_speeds;

    assert (num_no_ref <= num_pos);

    if (num_pos - num_no_ref != num_pos_inside + num_pos_outside)
        logwrn << "EvaluationRequirementSpeed '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside;
    assert (num_pos - num_no_ref == num_pos_inside + num_pos_outside);


    if (num_speeds != num_comp_failed + num_comp_passed)
        logwrn << "EvaluationRequirementSpeed '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_speeds " << num_speeds << " num_comp_failed " <<  num_comp_failed
               << " num_comp_passed " << num_comp_passed;

    assert (num_speeds == num_comp_failed + num_comp_passed);
    assert (num_speeds == values.size());

    //assert (details.size() == num_pos);

    return make_shared<EvaluationRequirementResult::SingleSpeed>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_no_tst_value,
                num_comp_failed, num_comp_passed,
                values, details);
}

}
