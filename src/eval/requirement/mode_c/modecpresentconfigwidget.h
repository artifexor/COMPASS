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

#ifndef EVALUATIONREQUIREMENTMODECPRESENTCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTMODECPRESENTCONFIGWIDGET_H

#include "eval/requirement/base/baseconfigwidget.h"

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class ModeCPresentConfig;

    class ModeCPresentConfigWidget : public BaseConfigWidget
    {
        Q_OBJECT

    public:
        ModeCPresentConfigWidget(ModeCPresentConfig& cfg);

    protected:

        ModeCPresentConfig& config();
    };

}

#endif // EVALUATIONREQUIREMENTMODECRESENTCONFIGWIDGET_H
