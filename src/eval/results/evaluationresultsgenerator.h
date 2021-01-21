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

#ifndef EVALUATIONRESULTSGENERATOR_H
#define EVALUATIONRESULTSGENERATOR_H

#include "eval/results/report/treemodel.h"
#include "eval/requirement/base/base.h"
#include "eval/results/base.h"
#include "evaluationdata.h"
#include "evaluationresultsgeneratorwidget.h"
#include "sectorlayer.h"
#include "logger.h"
#include "configurable.h"

#include <tbb/tbb.h>

class EvaluationManager;
class EvaluationStandard;

namespace EvaluationRequirementResult
{
    class Base;
    class Single;
}

class EvaluateTask : public tbb::task {

public:
    EvaluateTask(std::vector<std::shared_ptr<EvaluationRequirementResult::Single>>& results,
                 std::vector<unsigned int>& utns,
                 EvaluationData& data,
                 std::shared_ptr<EvaluationRequirement::Base> req,
                 const SectorLayer& sector_layer,
                 std::vector<bool>& done_flags, bool& task_done, bool single_thread)
        : results_(results), utns_(utns), data_(data), req_(req), sector_layer_(sector_layer),
          done_flags_(done_flags), task_done_(task_done),
          single_thread_(single_thread)
    {
    }

    /*override*/ tbb::task* execute() {
        // Do the job

        loginf << "EvaluateTask: execute: starting";

        unsigned int num_utns = utns_.size();
        assert (done_flags_.size() == num_utns);

        if (single_thread_)
        {
            for(unsigned int utn_cnt=0; utn_cnt < num_utns; ++utn_cnt)
            {
                results_[utn_cnt] = req_->evaluate(data_.targetData(utns_.at(utn_cnt)), req_, sector_layer_);
                done_flags_[utn_cnt] = true;
            }
        }
        else
        {
            tbb::parallel_for(uint(0), num_utns, [&](unsigned int utn_cnt)
            {
                results_[utn_cnt] = req_->evaluate(data_.targetData(utns_.at(utn_cnt)), req_, sector_layer_);
                done_flags_[utn_cnt] = true;
            });
        }

        for(unsigned int utn_cnt=0; utn_cnt < num_utns; ++utn_cnt)
            assert (results_[utn_cnt]);

        loginf << "EvaluateTask: execute: done";
        task_done_ = true;

        return NULL; // or a pointer to a new task to be executed immediately
    }

protected:
    std::vector<std::shared_ptr<EvaluationRequirementResult::Single>>& results_;
    std::vector<unsigned int>& utns_;
    EvaluationData& data_;
    std::shared_ptr<EvaluationRequirement::Base> req_;
    const SectorLayer& sector_layer_;
    std::vector<bool>& done_flags_;
    bool& task_done_;
    bool single_thread_;
};

class EvaluationResultsGenerator : public Configurable
{
public:
    EvaluationResultsGenerator(const std::string& class_id, const std::string& instance_id,
                               EvaluationManager& eval_man);
    virtual ~EvaluationResultsGenerator();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    void evaluate (EvaluationData& data, EvaluationStandard& standard);

    EvaluationResultsReport::TreeModel& resultsModel();

    typedef std::map<std::string,
    std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>::const_iterator ResultIterator;

    ResultIterator begin() { return results_.begin(); }
    ResultIterator end() { return results_.end(); }

    const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results ()
    const { return results_; } ;

    void updateToChanges();

    void generateResultsReportGUI();

    void clear();

    bool splitResultsByMOPS() const;
    void splitResultsByMOPS(bool value);

    bool showAdsbInfo() const;
    void showAdsbInfo(bool value);

    bool skipNoDataDetails() const;
    void skipNoDataDetails(bool value);

    EvaluationResultsGeneratorWidget& widget();

protected:
    EvaluationManager& eval_man_;

    std::unique_ptr<EvaluationResultsGeneratorWidget> widget_;

    bool skip_no_data_details_ {true};
    bool split_results_by_mops_ {false};
    bool show_adsb_info_ {false};

    EvaluationResultsReport::TreeModel results_model_;

    // rq group+name -> id -> result, e.g. "All:PD"->"UTN:22"-> or "SectorX:PD"->"All"
    std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>> results_;
    std::vector<std::shared_ptr<EvaluationRequirementResult::Base>> results_vec_; // ordered as generated

    virtual void checkSubConfigurables() override;
};

#endif // EVALUATIONRESULTSGENERATOR_H
