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

#include "histogramview.h"

#include <QApplication>

#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/metavariable.h"
#include "histogramviewconfigwidget.h"
#include "histogramviewdatasource.h"
#include "histogramviewdatawidget.h"
#include "histogramviewwidget.h"
#include "logger.h"
#include "latexvisitor.h"
#include "evaluationmanager.h"

using namespace dbContent;

/**
 */
HistogramView::HistogramView(const std::string& class_id, const std::string& instance_id,
                             ViewContainer* w, ViewManager& view_manager)
    : View(class_id, instance_id, w, view_manager)
{
    registerParameter("data_var_dbo", &data_var_dbo_, META_OBJECT_NAME);
    registerParameter("data_var_name", &data_var_name_, DBContent::meta_var_timestamp_.name());

    registerParameter("use_log_scale", &use_log_scale_, true);

    // create sub done in init
}

/**
 */
HistogramView::~HistogramView()
{
    if (data_source_)
    {
        delete data_source_;
        data_source_ = nullptr;
    }

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

/**
 */
bool HistogramView::init()
{
    View::init();

    createSubConfigurables();

    assert(data_source_);

    DBContentManager& object_man = COMPASS::instance().dbContentManager();
    connect(&object_man, &DBContentManager::loadingDoneSignal, this, &HistogramView::allLoadingDoneSlot);

//    connect(data_source_, &HistogramViewDataSource::loadingStartedSignal, widget_->getDataWidget(),
//            &HistogramViewDataWidget::loadingStartedSlot);
//    connect(data_source_, &HistogramViewDataSource::updateDataSignal, widget_->getDataWidget(),
//            &HistogramViewDataWidget::updateDataSlot);

//    connect(widget_->configWidget(), &HistogramViewConfigWidget::exportSignal,
//            widget_->getDataWidget(), &HistogramViewDataWidget::exportDataSlot);
//    connect(widget_->getDataWidget(), &HistogramViewDataWidget::exportDoneSignal,
//            widget_->configWidget(), &HistogramViewConfigWidget::exportDoneSlot);

//    connect(data_source_, &HistogramViewDataSource::loadingStartedSignal, widget_->configWidget(),
//            &HistogramViewConfigWidget::loadingStartedSlot);

    //    connect(this, &HistogramView::showOnlySelectedSignal, widget_->getDataWidget(),
    //            &HistogramViewDataWidget::showOnlySelectedSlot);
    //    connect(this, &HistogramView::usePresentationSignal, widget_->getDataWidget(),
    //            &HistogramViewDataWidget::usePresentationSlot);
    //    connect(this, &HistogramView::showAssociationsSignal, widget_->getDataWidget(),
    //            &HistogramViewDataWidget::showAssociationsSlot);

    //    widget_->getDataWidget()->showOnlySelectedSlot(show_only_selected_);
    //    widget_->getDataWidget()->usePresentationSlot(use_presentation_);
    //    widget_->getDataWidget()->showAssociationsSlot(show_associations_);

    // eval
    connect(&COMPASS::instance().evaluationManager(), &EvaluationManager::resultsChangedSignal,
            this, &HistogramView::resultsChangedSlot);

    return true;
}

/**
 */
void HistogramView::loadingStarted()
{
    loginf << "HistogramView: loadingStarted";

    getDataWidget()->loadingStartedSlot();
}

/**
 */
void HistogramView::loadedData(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    loginf << "HistogramView: loadedData";

    getDataWidget()->updateDataSlot(data, requires_reset);
}

/**
 */
void HistogramView::loadingDone()
{
    loginf << "HistogramView: loadingDone";

    getDataWidget()->loadingDoneSlot();
}

/**
 */
void HistogramView::clearData()
{
    loginf << "HistogramView: clearData";

    getDataWidget()->clear();
}

/**
 */
void HistogramView::appModeSwitch (AppMode app_mode_previous, AppMode app_mode_current)
{
    loginf << "HistogramView: appModeSwitch: app_mode " << toString(app_mode_current)
           << " prev " << toString(app_mode_previous);

    widget_->getViewConfigWidget()->appModeSwitch(app_mode_current);
}

/**
 */
void HistogramView::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
    logdbg << "HistogramView: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id == "HistogramViewDataSource")
    {
        assert(!data_source_);
        data_source_ = new HistogramViewDataSource(class_id, instance_id, this);
    }
    else if (class_id == "HistogramViewWidget")
    {
        widget_ = new HistogramViewWidget(class_id, instance_id, this, this, central_widget_);
        setWidget(widget_);
    }
    else
        throw std::runtime_error("HistogramView: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

/**
 */
void HistogramView::checkSubConfigurables()
{
    if (!data_source_)
    {
        generateSubConfigurable("HistogramViewDataSource", "HistogramViewDataSource0");
    }

    if (!widget_)
    {
        generateSubConfigurable("HistogramViewWidget", "HistogramViewWidget0");
    }
}

/**
 */
bool HistogramView::hasDataWidget() const
{
    return (widget_ && widget_->getViewDataWidget());
}

/**
 */
HistogramViewDataWidget* HistogramView::getDataWidget()
{
    assert (widget_);
    return widget_->getViewDataWidget();
}

/**
 */
VariableSet HistogramView::getSet(const std::string& dbcontent_name)
{
    assert(data_source_);

    VariableSet set = data_source_->getSet()->getExistingInDBFor(dbcontent_name);

    if (hasDataVar())
    {
        if (isDataVarMeta())
        {
            MetaVariable& meta_var = metaDataVar();

            if (meta_var.existsIn(dbcontent_name) && !set.hasVariable(meta_var.getFor(dbcontent_name)))
                set.add(meta_var.getFor(dbcontent_name));
        }
        else
        {
            if (dataVar().dbContentName() == dbcontent_name && !set.hasVariable(dataVar()))
                set.add(dataVar());
        }
    }

    return set;
}

/**
 */
void HistogramView::accept(LatexVisitor& v)
{
    v.visit(this);
}

/**
 */
bool HistogramView::useLogScale() const
{
    return use_log_scale_;
}

/**
 */
void HistogramView::useLogScale(bool use_log_scale)
{
    use_log_scale_ = use_log_scale;

    HistogramViewDataWidget* data_widget = dynamic_cast<HistogramViewDataWidget*>(getDataWidget());
    assert (data_widget);

    data_widget->updateChart();
}

/**
 */
bool HistogramView::hasDataVar ()
{
    if (!data_var_dbo_.size() || !data_var_name_.size())
        return false;

    if (data_var_dbo_ == META_OBJECT_NAME)
        return COMPASS::instance().dbContentManager().existsMetaVariable(data_var_name_);
    else
        return COMPASS::instance().dbContentManager().dbContent(data_var_dbo_).hasVariable(data_var_name_);
}

/**
 */
bool HistogramView::isDataVarMeta ()
{
    return data_var_dbo_ == META_OBJECT_NAME;
}

/**
 */
Variable& HistogramView::dataVar()
{
    assert (hasDataVar());
    assert (!isDataVarMeta());
    assert (COMPASS::instance().dbContentManager().dbContent(data_var_dbo_).hasVariable(data_var_name_));

    return COMPASS::instance().dbContentManager().dbContent(data_var_dbo_).variable(data_var_name_);
}

/**
 */
void HistogramView::dataVar (Variable& var)
{
    loginf << "HistogramView: dataVar: dbo " << var.dbContentName() << " name " << var.name();

    data_var_dbo_ = var.dbContentName();
    data_var_name_ = var.name();
    assert (hasDataVar());
    assert (!isDataVarMeta());

    assert (widget_);
    widget_->getViewDataWidget()->updateView();

    if (COMPASS::instance().appMode() != AppMode::LiveRunning && widget_->getViewDataWidget()->dataNotInBuffer())
        widget_->getViewConfigWidget()->setStatus("Reload Required", true, Qt::red);
}

/**
 */
MetaVariable& HistogramView::metaDataVar()
{
    assert (hasDataVar());
    assert (isDataVarMeta());

    return COMPASS::instance().dbContentManager().metaVariable(data_var_name_);
}

/**
 */
void HistogramView::metaDataVar (MetaVariable& var)
{
    loginf << "HistogramView: metaDataVar: name " << var.name();

    data_var_dbo_ = META_OBJECT_NAME;
    data_var_name_ = var.name();
    assert (hasDataVar());
    assert (isDataVarMeta());

    assert (widget_);
    widget_->getViewDataWidget()->updateView();

    if (COMPASS::instance().appMode() != AppMode::LiveRunning && widget_->getViewDataWidget()->dataNotInBuffer())
        widget_->getViewConfigWidget()->setStatus("Reload Required", true, Qt::red);
}

/**
 */
std::string HistogramView::dataVarDBO() const
{
    return data_var_dbo_;
}

/**
 */
std::string HistogramView::dataVarName() const
{
    return data_var_name_;
}

/**
 */
void HistogramView::updateSelection()
{
    loginf << "HistogramView: updateSelection";
    assert(widget_);
    
    widget_->getViewDataWidget()->updateView();

    //    if (show_only_selected_)
    //        widget_->getDataWidget()->updateToSelection();
    //    else
    //        widget_->getDataWidget()->resetModels();  // just updates the checkboxes
}

/**
 */
void HistogramView::unshowViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "HistogramView: unshowViewPoint";

//    assert (vp);
//    assert (data_source_);
//    data_source_->unshowViewPoint(vp);

    current_view_point_ = nullptr;
}

/**
 */
bool HistogramView::showResults() const
{
    return show_results_;
}

/**
 */
void HistogramView::showResults(bool value)
{
    show_results_ = value;

    onShowResultsChanged();
}

/**
 */
std::string HistogramView::evalResultGrpReq() const
{
    return eval_results_grpreq_;
}

/**
 */
void HistogramView::evalResultGrpReq(const std::string& value)
{
    if (eval_results_grpreq_ == value)
        return;

    loginf << "HistogramView: evalResultGrpReq: value " << value;

    eval_results_grpreq_ = value;

    widget_->getViewDataWidget()->updateView();
}

/**
 */
std::string HistogramView::evalResultsID() const
{
    return eval_results_id_;
}

/**
 */
void HistogramView::evalResultsID(const std::string& value)
{
    if (eval_results_id_ == value)
        return;

    loginf << "HistogramView: evalResultsID: value " << value;

    eval_results_id_ = value;

    widget_->getViewDataWidget()->updateView();
}

/**
 */
bool HistogramView::hasResultID() const
{
    return (!evalResultGrpReq().empty() && !evalResultsID().empty());
}

/**
 */
void HistogramView::showViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "HistogramView: showViewPoint";

    assert (vp);
//    assert (data_source_);
//    data_source_->showViewPoint(vp);
//    assert (widget_);

    current_view_point_ = vp;

    //widget_->getDataWidget()->resetViewPointZoomed(); // TODO
}

/**
 */
void HistogramView::onShowResultsChanged()
{
    widget_->getViewConfigWidget()->updateConfig();
    widget_->getViewDataWidget()->updateView();
}

/**
 */
void HistogramView::resultsChangedSlot()
{
    loginf << "HistogramView: resultsChangedSlot";

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results =
            eval_man.results();

    // check if result ids are still valid
    if (!results.size())
    {
        eval_results_grpreq_ = "";
        eval_results_id_ = "";
    }
    else
    {
        if (!results.count(eval_results_grpreq_))
        {
            eval_results_grpreq_ = "";
            eval_results_id_ = "";
        }
        else if (!results.at(eval_results_grpreq_).count(eval_results_id_))
        {
            eval_results_id_ = "";
        }
    }

    //reset result visualization if result id is bad
    if (show_results_ && !hasResultID())
    {
        show_results_ = false;
    }

    //update on result change
    onShowResultsChanged();
}

/**
 */
void HistogramView::allLoadingDoneSlot()
{
    loginf << "HistogramView: allLoadingDoneSlot";
    assert(widget_);

    widget_->getViewConfigWidget()->setDisabled(false);
    widget_->getViewConfigWidget()->setStatus("", false);

    if (current_view_point_ && current_view_point_->data().contains("evaluation_results"))
    {
        //infer result visualization from view point
        const nlohmann::json& data = current_view_point_->data();
        assert (data.at("evaluation_results").contains("show_results"));
        assert (data.at("evaluation_results").contains("req_grp_id"));
        assert (data.at("evaluation_results").contains("result_id"));

        show_results_        = data.at("evaluation_results").at("show_results");
        eval_results_grpreq_ = data.at("evaluation_results").at("req_grp_id");
        eval_results_id_     = data.at("evaluation_results").at("result_id");

        //show_results_ = true;

//        eval_highlight_details_.clear();

//        if (data.at("evaluation_results").contains("highlight_details"))
//        {
//            loginf << "OSGView: loadingDoneSlot: highlight_details "
//                   << data.at("evaluation_results").at("highlight_details").dump();

//            vector<unsigned int> highlight_details = data.at("evaluation_results").at("highlight_details");
//            eval_highlight_details_ = move(highlight_details);
//        }

        resultsChangedSlot();
    }
    else
    {
        //do not show results
        showResults(false);
    }
}
