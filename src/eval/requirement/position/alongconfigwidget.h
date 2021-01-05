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

#ifndef EVALUATIONREQUIREMENTPOSITIONALONGCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTPOSITIONALONGCONFIGWIDGET_H

#include "eval/requirement/base/baseconfigwidget.h"

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class PositionAlongConfig;

    class PositionAlongConfigWidget : public BaseConfigWidget
    {
        Q_OBJECT

    public slots:
        void maxAbsValueEditSlot(QString value);
        void minimumProbEditSlot(QString value);

    public:
        PositionAlongConfigWidget(PositionAlongConfig& cfg);

    protected:
        //PositionAlongConfig& config_;

        QLineEdit* max_abs_value_edit_{nullptr};
        QLineEdit* minimum_prob_edit_{nullptr};

        //QFormLayout* form_layout_ {nullptr};

        PositionAlongConfig& config();
    };

}

#endif // EVALUATIONREQUIREMENTPOSITIONALONGCONFIGWIDGET_H
