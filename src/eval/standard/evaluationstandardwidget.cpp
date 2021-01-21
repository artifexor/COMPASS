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

#include "evaluationstandardwidget.h"
#include "evaluationstandard.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/base/baseconfigwidget.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QSplitter>
#include <QSettings>

using namespace std;

EvaluationStandardWidget::EvaluationStandardWidget(EvaluationStandard& standard)
    : QWidget(), standard_(standard), standard_model_(standard)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    splitter_ = new QSplitter();
    splitter_->setOrientation(Qt::Horizontal);

    //QHBoxLayout* req_layout = new QHBoxLayout();

    tree_view_.reset(new QTreeView());
    tree_view_->setModel(&standard_model_);
    tree_view_->setRootIsDecorated(false);
    tree_view_->expandAll();

    connect (tree_view_.get(), &QTreeView::clicked, this, &EvaluationStandardWidget::itemClickedSlot);

    splitter_->addWidget(tree_view_.get());
    //req_layout->addWidget(tree_view_.get());

    // requirements stack
    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    requirements_widget_ = new QStackedWidget();

    scroll_area->setWidget(requirements_widget_);

    splitter_->addWidget(scroll_area);

    splitter_->setStretchFactor(1, 1);
    //req_layout->addWidget(scroll_area, 1);

    QSettings settings("COMPASS", ("EvalStandardWidget"+standard_.name()).c_str());
    splitter_->restoreState(settings.value("splitterSizes").toByteArray());

    //main_layout->addLayout(req_layout);
    main_layout->addWidget(splitter_);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

EvaluationStandardWidget::~EvaluationStandardWidget()
{
    assert (splitter_);

    QSettings settings("COMPASS", ("EvalStandardWidget"+standard_.name()).c_str());
    settings.setValue("splitterSizes", splitter_->saveState());
}

EvaluationStandardTreeModel& EvaluationStandardWidget::model()
{
    return standard_model_;
}

void EvaluationStandardWidget::itemClickedSlot(const QModelIndex& index)
{
    EvaluationStandardTreeItem* item = static_cast<EvaluationStandardTreeItem*>(index.internalPointer());
    assert (item);

    if (dynamic_cast<EvaluationStandard*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got standard";

        EvaluationStandard* std = dynamic_cast<EvaluationStandard*>(item);

        showRequirementWidget(nullptr);

        std->showMenu();

    }
    else if (dynamic_cast<Group*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got group";

        Group* group = dynamic_cast<Group*>(item);

        showRequirementWidget(nullptr);

        group->showMenu();
    }
    else if (dynamic_cast<EvaluationRequirement::BaseConfig*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got config";

        EvaluationRequirement::BaseConfig* config =
                dynamic_cast<EvaluationRequirement::BaseConfig*>(item);

        showRequirementWidget(config->widget());
    }
    else
        assert (false);
}

void EvaluationStandardWidget::expandAll()
{
    tree_view_->expandAll();
}

void EvaluationStandardWidget::showRequirementWidget(QWidget* widget)
{
    assert(requirements_widget_);

    if (!widget)
    {
        while (requirements_widget_->count() > 0)  // remove all widgets
            requirements_widget_->removeWidget(requirements_widget_->widget(0));
        return;
    }

    if (requirements_widget_->indexOf(widget) < 0)
        requirements_widget_->addWidget(widget);

    requirements_widget_->setCurrentWidget(widget);
}
