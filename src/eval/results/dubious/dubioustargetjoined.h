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

#ifndef EVALUATIONREQUIREMENTJOINEDDUBIOSTARGET_H
#define EVALUATIONREQUIREMENTJOINEDDUBIOSTARGET_H

#include "eval/results/joined.h"
#include "eval/requirement/dubious/dubioustarget.h"

namespace EvaluationRequirementResult
{
    using namespace std;

    class SingleDubiousTarget;

    class JoinedDubiousTarget : public Joined
    {
    public:
        JoinedDubiousTarget(
                const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
                const SectorLayer& sector_layer, EvaluationManager& eval_man);

        virtual void join(std::shared_ptr<Base> other) override;

        //virtual void print() override;
        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

        virtual void updatesToUseChanges() override;

        virtual bool hasViewableData (
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
        virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

        virtual bool hasReference (
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
        virtual std::string reference(
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

        void exportAsCSV();

        std::vector<EvaluationRequirement::DubiousTargetDetail> details() const;

    protected:
        unsigned int num_updates_ {0};
        unsigned int num_pos_outside_ {0};
        unsigned int num_pos_inside_ {0};
        unsigned int num_pos_inside_dubious_ {0};
        unsigned int num_utns_ {0};
        unsigned int num_utns_dubious_ {0};

        float duration_all_ {0};
        float duration_nondub_ {0};
        float duration_dubious_ {0};

        std::vector<EvaluationRequirement::DubiousTargetDetail> details_;

        bool has_p_dubious_ {false};
        float p_dubious_{0};

        bool has_p_dubious_update_ {false};
        float p_dubious_update_{0};

        void addToValues (std::shared_ptr<SingleDubiousTarget> single_result);
        void update();

        void addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
        void addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

        std::unique_ptr<nlohmann::json::object_t> getErrorsViewable ();
    };

}

#endif // EVALUATIONREQUIREMENTJOINEDDUBIOSTARGET_H
