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

#include "eval/results/dubious/dubioustracksingle.h"
#include "eval/results/dubious/dubioustrackjoined.h"
#include "eval/requirement/base/base.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <QFileDialog>

#include <algorithm>
#include <cassert>
#include <fstream>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedDubiousTrack::JoinedDubiousTrack(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer, EvaluationManager& eval_man)
    : Joined("JoinedDubiousTrack", result_id, requirement, sector_layer, eval_man)
{
}


void JoinedDubiousTrack::join(std::shared_ptr<Base> other)
{
    Joined::join(other);

    std::shared_ptr<SingleDubiousTrack> other_sub =
            std::static_pointer_cast<SingleDubiousTrack>(other);
    assert (other_sub);

    addToValues(other_sub);
}

void JoinedDubiousTrack::addToValues (std::shared_ptr<SingleDubiousTrack> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    num_updates_ += single_result->numUpdates();
    num_pos_outside_ += single_result->numPosOutside();
    num_pos_inside_ += single_result->numPosInside();
    num_pos_inside_dubious_ += single_result->numPosInsideDubious();
    num_tracks_ += single_result->numTracks();
    num_tracks_dubious_ += single_result->numTracksDubious();

    track_duration_all_ += single_result->trackDurationAll();
    track_duration_nondub_ += single_result->trackDurationNondub();
    track_duration_dubious_ += single_result->trackDurationDubious();

    details_.insert(details_.end(), single_result->details().begin(), single_result->details().end());

    //const vector<double>& other_values = single_result->values();
    //values_.insert(values_.end(), other_values.begin(), other_values.end());

    update();
}

void JoinedDubiousTrack::update()
{
    assert (num_updates_ == num_pos_inside_ + num_pos_outside_);
    assert (num_tracks_ >= num_tracks_dubious_);

    //assert (values_.size() == num_comp_failed_+num_comp_passed_);

    //unsigned int num_speeds = values_.size();

    if (num_tracks_)
    {

        p_dubious_ = (float)num_tracks_dubious_/(float)num_tracks_;
        has_p_dubious_ = true;
    }
    else
    {
        has_p_dubious_ = false;
        p_dubious_ = 0;
    }

    if (num_pos_inside_)
    {
        p_dubious_update_ = (float)num_pos_inside_dubious_/(float)num_pos_inside_;
        has_p_dubious_update_ = true;
    }
    else
    {
        p_dubious_update_ = 0;
        has_p_dubious_update_ = false;
    }
}

void JoinedDubiousTrack::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedDubiousTrack " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedDubiousTrack " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedDubiousTrack " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedDubiousTrack::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::DubiousTrack> req =
            std::static_pointer_cast<EvaluationRequirement::DubiousTrack>(requirement_);
    assert (req);

    QVariant p_dubious_var;

    string result {"Unknown"};

    if (has_p_dubious_)
    {
        p_dubious_var = String::percentToString(p_dubious_ * 100.0, req->getNumProbDecimals()).c_str();

        result = req->getResultConditionStr(p_dubious_);
    }

    // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
    ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                     +(requirement_->shortname()).c_str(),
                     result_id_.c_str(), num_updates_,
                     p_dubious_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
}

void JoinedDubiousTrack::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::DubiousTrack> req =
            std::static_pointer_cast<EvaluationRequirement::DubiousTrack>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    // callbacks
    auto exportAsCSV_lambda = [this]() {
        this->exportAsCSV();
    };

    sec_det_table.registerCallBack("Save Data As CSV", exportAsCSV_lambda);

    // details
    addCommonDetails(sec_det_table);

    sec_det_table.addRow({"Use", "To be used in results", use_}, this);
    sec_det_table.addRow({"#Pos [1]", "Number of updates", num_updates_}, this);
    sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    sec_det_table.addRow({"#DU [1]", "Number of dubious updates inside sector", num_pos_inside_dubious_}, this);

    QVariant p_dubious_up_var;

    if (has_p_dubious_update_)
        p_dubious_up_var = roundf(p_dubious_update_ * 10000.0) / 100.0;

    sec_det_table.addRow({"PDU [%]", "Probability of dubious update", p_dubious_up_var}, this);

    sec_det_table.addRow({"#T [1]", "Number of tracks", num_tracks_}, this);
    sec_det_table.addRow({"#DT [1]", "Number of dubious tracks", num_tracks_dubious_},
                         this);

    sec_det_table.addRow({"Duration [s]", "Duration of all tracks",
                          String::doubleToStringPrecision(track_duration_all_,2).c_str()}, this);
    sec_det_table.addRow({"Duration Dubious [s]", "Duration of dubious tracks",
                          String::doubleToStringPrecision(track_duration_dubious_,2).c_str()}, this);

    QVariant dubious_t_avg_var;

    if (num_tracks_dubious_)
        dubious_t_avg_var = roundf(track_duration_dubious_/(float)num_tracks_dubious_ * 100.0) / 100.0;

    sec_det_table.addRow({"Duration Non-Dubious [s]", "Duration of non-dubious tracks",
                          String::doubleToStringPrecision(track_duration_nondub_,2).c_str()}, this);

    sec_det_table.addRow({"Average Duration Dubious [s]", "Average duration of dubious tracks",
                          dubious_t_avg_var}, this);

    QVariant p_dubious_t_var, p_nondub_t_var;

    if (track_duration_all_)
    {
        p_dubious_t_var = roundf(track_duration_dubious_/track_duration_all_ * 10000.0) / 100.0;
        p_nondub_t_var = roundf(track_duration_nondub_/track_duration_all_ * 10000.0) / 100.0;
    }

    sec_det_table.addRow({"Duration Ratio Dubious [%]", "Duration ratio of dubious tracks", p_dubious_t_var}, this);
    sec_det_table.addRow({"Duration Ratio Non-Dubious [%]", "Duration ratio of non-dubious tracks", p_nondub_t_var}, this);

    // condition
    {
        QVariant p_dubious_var;

        if (has_p_dubious_)
            p_dubious_var = roundf(p_dubious_ * 10000.0) / 100.0;

        sec_det_table.addRow({"PDT [%]", "Probability of dubious track", p_dubious_var}, this);

        sec_det_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (has_p_dubious_)
            result = req->getResultConditionStr(p_dubious_);

        sec_det_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);
    }

//    // figure
//    if (has_p_min_ && p_passed_ != 1.0) // TODO
//    {
//        sector_section.addFigure("sector_errors_overview", "Sector Errors Overview",
//                                 getErrorsViewable());
//    }
//    else
//    {
//        sector_section.addText("sector_errors_overview_no_figure");
//        sector_section.getText("sector_errors_overview_no_figure").addText(
//                    "No target errors found, therefore no figure was generated.");
//    }
}

bool JoinedDubiousTrack::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedDubiousTrack::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedDubiousTrack::getErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
            eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

//    double lat_min, lat_max, lon_min, lon_max;

//    tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
//    tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

//    (*viewable_ptr)[VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
//    (*viewable_ptr)[VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

//    double lat_w = 1.1*(lat_max-lat_min)/2.0;
//    double lon_w = 1.1*(lon_max-lon_min)/2.0;

//    if (lat_w < eval_man_.resultDetailZoom())
//        lat_w = eval_man_.resultDetailZoom();

//    if (lon_w < eval_man_.resultDetailZoom())
//        lon_w = eval_man_.resultDetailZoom();

//    (*viewable_ptr)["speed_window_latitude"] = lat_w;
//    (*viewable_ptr)["speed_window_longitude"] = lon_w;

    return viewable_ptr;
}

bool JoinedDubiousTrack::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedDubiousTrack::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();
}

void JoinedDubiousTrack::updatesToUseChanges()
{
    loginf << "JoinedDubiousTrack: updatesToUseChanges";

    num_updates_ = 0;
    num_pos_outside_ = 0;
    num_pos_inside_ = 0;
    num_pos_inside_dubious_ = 0;
    num_tracks_ = 0;
    num_tracks_dubious_ = 0;

    track_duration_all_ = 0;
    track_duration_nondub_ = 0;
    track_duration_dubious_ = 0;

    details_.clear();

    for (auto result_it : results_)
    {
        std::shared_ptr<SingleDubiousTrack> result =
                std::static_pointer_cast<SingleDubiousTrack>(result_it);
        assert (result);

        addToValues(result);
    }
}

void JoinedDubiousTrack::exportAsCSV()
{
    loginf << "JoinedDubiousTrack: exportAsCSV";

//    QFileDialog dialog(nullptr);
//    dialog.setFileMode(QFileDialog::AnyFile);
//    dialog.setNameFilter("CSV Files (*.csv)");
//    dialog.setDefaultSuffix("csv");
//    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

//    if (dialog.exec())
//    {
//        QStringList file_names = dialog.selectedFiles();
//        assert (file_names.size() == 1);

//        string filename = file_names.at(0).toStdString();

//        std::ofstream output_file;

//        output_file.open(filename, std::ios_base::out);

//        if (output_file)
//        {
//            output_file << "speed_offset\n";
//            unsigned int size = values_.size();

//            for (unsigned int cnt=0; cnt < size; ++cnt)
//                output_file << values_.at(cnt) << "\n";
//        }
//    }
}

std::vector<EvaluationRequirement::DubiousTrackDetail> JoinedDubiousTrack::details() const
{
    return details_;
}

}
