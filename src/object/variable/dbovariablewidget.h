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

#ifndef DBOVARIABLEWIDGET_H_
#define DBOVARIABLEWIDGET_H_

#include <QWidget>
#include <map>

class QLineEdit;
class QComboBox;
class QPushButton;
class QTextEdit;

class DBContentVariable;
class DBContentVariableDataTypeComboBox;
class StringRepresentationComboBox;
class UnitSelectionWidget;
class QGridLayout;

/**
 * @brief Edit widget for a DBObject
 */
class DBContentVariableWidget : public QWidget
{
    Q_OBJECT

  signals:
    void dboVariableChangedSignal();

  public slots:
    /// @brief Changes DBO name
    void editNameSlot();
    /// @brief Changes DBO info
    void editDescriptionSlot();
    //void editDataTypeSlot();

  public:
    /// @brief Constructor
    DBContentVariableWidget(DBContentVariable& variable, QWidget* parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBContentVariableWidget();

    void lock();
    void unlock();

    void setVariable(DBContentVariable& variable);
    void update();

  private:
    /// @brief DBObject to be managed
    DBContentVariable* variable_{nullptr};
    QGridLayout* properties_layout_{nullptr};

    bool locked_{false};

    /// @brief DBOVariable name
    QLineEdit* name_edit_{nullptr};
    /// @brief DBOVariable info
    QLineEdit* description_edit_{nullptr};
    DBContentVariableDataTypeComboBox* type_combo_{nullptr};
    StringRepresentationComboBox* representation_box_{nullptr};
    UnitSelectionWidget* unit_sel_{nullptr};
};

#endif /* DBOBJECTEDITWIDGET_H_ */
