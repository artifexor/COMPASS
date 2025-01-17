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

#ifndef UTNFILTERWIDGET_H
#define UTNFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "utnfilter.h"

class QLabel;
class QLineEdit;

class UTNFilterWidget : public DBFilterWidget
{
    Q_OBJECT

  protected slots:
    void valueEditedSlot(const QString& value);

public:
    UTNFilterWidget(UTNFilter& filter);
    virtual ~UTNFilterWidget();

    virtual void update();

protected:
    UTNFilter& filter_;

    QLabel* label_{nullptr};
    QLineEdit* value_edit_ {nullptr};
};

#endif // UTNFILTERWIDGET_H
