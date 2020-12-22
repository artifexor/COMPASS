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

#ifndef HISTOGRAMVIEWCONFIGWIDGET_H_
#define HISTOGRAMVIEWCONFIGWIDGET_H_

#include <QWidget>

#include "dbovariable.h"

class DBOVariableOrderedSetWidget;
class QCheckBox;
class HistogramView;
class QLineEdit;
class QPushButton;
class DBOVariableSelectionWidget;

/**
 * @brief Widget with configuration elements for a HistogramView
 *
 */
class HistogramViewConfigWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void selectedVariableChangedSlot();

    void toggleLogScale();

//    void exportSlot();
//    void exportDoneSlot(bool cancelled);

    void reloadRequestedSlot();
    void loadingStartedSlot();

  signals:
    //void exportSignal(bool overwrite);
    void reloadRequestedSignal();  // reload from database

  public:
    HistogramViewConfigWidget(HistogramView* view, QWidget* parent = nullptr);
    virtual ~HistogramViewConfigWidget();

  protected:
    HistogramView* view_;

    DBOVariableSelectionWidget* select_var_ {nullptr};

    QCheckBox* log_check_ {nullptr};
    //QPushButton* export_button_{nullptr};

    QPushButton* reload_button_{nullptr};
};

#endif /* HISTOGRAMVIEWCONFIGWIDGET_H_ */
