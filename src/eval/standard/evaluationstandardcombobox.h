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

#ifndef EVALUATIONSTANDARDCOMBOBOX_H
#define EVALUATIONSTANDARDCOMBOBOX_H

#include <QComboBox>

class EvaluationManager;

class EvaluationStandardComboBox : public QComboBox
{
    Q_OBJECT

public slots:
    /// @brief Emitted if type was changed
    void changedStandardSlot(const QString& standard_name); // slot for box

public:
    EvaluationStandardComboBox(EvaluationManager& eval_man, QWidget* parent=0);
    virtual ~EvaluationStandardComboBox();

    void setStandardName(const std::string& value);
    void updateStandards();

protected:
    EvaluationManager& eval_man_;
};

#endif // EVALUATIONSTANDARDCOMBOBOX_H
