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

#include "manageschemataskwidget.h"

#include "compass.h"
#include "manageschematask.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

ManageSchemaTaskWidget::ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    expertModeChangedSlot();

    setLayout(main_layout);
}

void ManageSchemaTaskWidget::schemaLockedSlot()
{
    loginf << "ManageSchemaTaskWidget: schemaLockedSlot";
    emit task_.statusChangedSignal(task_.name());
}

void ManageSchemaTaskWidget::expertModeChangedSlot() {}
