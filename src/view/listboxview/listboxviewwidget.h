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

#ifndef LISTBOXVIEWWIDGET_H_
#define LISTBOXVIEWWIDGET_H_

#include "viewwidget.h"

class ListBoxView;
class ListBoxViewDataWidget;
class ListBoxViewConfigWidget;

class QSplitter;
class QTabWidget;

/**
 * @brief Used for textual data display in a ListBoxView.
 *
 * Consists of a ListBoxViewDataWidget for data view and a ListBoxViewConfigWidget for configuration
 * and starting the loading process.
 */
class ListBoxViewWidget : public ViewWidget
{
  public:
    /// @brief Constructor
    ListBoxViewWidget(const std::string& class_id, const std::string& instance_id,
                      Configurable* config_parent, ListBoxView* view, QWidget* parent = NULL);
    /// @brief Destructor
    virtual ~ListBoxViewWidget();

    /// @brief Toggles visibility of the config widget
    void toggleConfigWidget();

    /// @brief Returns the config widget
    ListBoxViewConfigWidget* configWidget();

    /// @brief Returns the basis view
    ListBoxView* getView() { return (ListBoxView*)view_; }
    /// @brief Returns the data widget
    ListBoxViewDataWidget* getDataWidget() { return data_widget_; }

  protected:
    QSplitter* main_splitter_{nullptr};
    /// Data widget with data display
    ListBoxViewDataWidget* data_widget_{nullptr};
    /// Config widget with configuration elements
    ListBoxViewConfigWidget* config_widget_{nullptr};
};

#endif /* LISTBOXVIEWWIDGET_H_ */
