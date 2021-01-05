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

#ifndef EVALUATIONREQUIREMENTMODEAPRESENTCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTMODEAPRESENTCONFIGWIDGET_H

#include "eval/requirement/base/baseconfigwidget.h"

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class ModeAPresentConfig;

    class ModeAPresentConfigWidget : public BaseConfigWidget
    {
        Q_OBJECT

    public slots:
        void minProbPresentEditSlot(QString value);

    public:
        ModeAPresentConfigWidget(ModeAPresentConfig& cfg);

    protected:
        //ModeAPresentConfig& config_;

        //QFormLayout* form_layout_ {nullptr};

        QLineEdit* min_prob_pres_edit_{nullptr};

        ModeAPresentConfig& config();
    };

}

#endif // EVALUATIONREQUIREMENTMODEARESENTCONFIGWIDGET_H
