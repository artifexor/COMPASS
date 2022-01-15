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

#ifndef DBOEDITDATASOURCESWIDGET_H
#define DBOEDITDATASOURCESWIDGET_H

#include <QWidget>

#include "dbcontent/dbcontent.h"
#include "dboeditdatasourceactionoptions.h"

class ManageDataSourcesTask;
class DBObject;

class QTableWidget;
class QLabel;
class QPushButton;
class QTableWidgetItem;
class QGridLayout;

class DBOEditDataSourcesWidget : public QWidget
{
    Q_OBJECT
  signals:
    void dbItemChangedSignal();

  public slots:
    void syncOptionsFromDBSlot();
    void addStoredDSSlot();

    void syncOptionsFromCfgSlot();

    void selectAllActionsSlot();
    void deselectAllActionsSlot();
    void performActionsSlot();

    void configItemChangedSlot(QTableWidgetItem* item);
    void dbItemChangedSlot(QTableWidgetItem* item);

  public:
    DBOEditDataSourcesWidget(ManageDataSourcesTask& task, DBObject& object, QWidget* parent = 0,
                             Qt::WindowFlags f = 0);
    virtual ~DBOEditDataSourcesWidget();

    void update();

  private:
    ManageDataSourcesTask& task_;
    DBObject& object_;

    QStringList table_columns_{"ID",  "Name", "Short Name", "SAC",
                               "SIC", "Latitude", "Longitude",  "Altitude"};

    // radar ranges
    bool has_primary_range_min_{false};
    bool has_primary_range_max_{false};

    bool has_secondary_range_min_{false};
    bool has_secondary_range_max_{false};

    bool has_mode_s_range_min_{false};
    bool has_mode_s_range_max_{false};

    // radar std devs
    bool has_primary_azimuth_stddev_{false};
    bool has_primary_range_stddev_{false};

    bool has_secondary_azimuth_stddev_{false};
    bool has_secondary_range_stddev_{false};

    bool has_mode_s_azimuth_stddev_{false};
    bool has_mode_s_range_stddev_{false};

    QTableWidget* config_ds_table_{nullptr};
    QPushButton* sync_from_cfg_button_{nullptr};

    QLabel* action_heading_label_{nullptr};
    std::string action_heading_;
    QGridLayout* action_layout_{nullptr};
    DBOEditDataSourceActionOptionsCollection action_collection_;

    QPushButton* select_all_actions_{nullptr};
    QPushButton* deselect_all_actions_{nullptr};
    QPushButton* perform_actions_button_{nullptr};

    QTableWidget* db_ds_table_{nullptr};
    QPushButton* sync_from_db_button_{nullptr};

    void updateConfigDSTable();
    void updateDBDSTable();
    void updateColumnSizes();

    void clearSyncOptions();
    void displaySyncOptions();

    void updateActionButtons();
    bool haveActionsToPerform();
};

#endif  // DBOEDITDATASOURCESWIDGET_H
