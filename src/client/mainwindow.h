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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "global.h"
#include "appmode.h"
#include "autoresumedialog.h"

#include <QMainWindow>

#include <memory>

class QLabel;
class QPushButton;
class QTabWidget;
class QCheckBox;
class QMenu;
class QPushButton;
class QAction;
class QTimer;

class DBSelectionWidget;
class DBSchemaManagerWidget;
class DBContentManagerWidget;
class MainLoadWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:

    void newDBSlot();
    void openExistingDBSlot();
    void openRecentDBSlot();
    void exportDBSlot();
    void clearExistingDBsSlot();
    void closeDBSlot();

    void saveConfigSlot();

    void quitWOConfigSlot();
    void quitSlot();

    void loadButtonSlot();
    void loadingDoneSlot();

    void livePauseResumeSlot();
    void liveStopSlot();

    void configureDataSourcesSlot();
    void configureMetaVariablesSlot();
    void configureSectorsSlot();

    void importAsterixRecordingSlot();
    void importRecentAsterixRecordingSlot();
    void clearImportRecentAsterixRecordingsSlot();
    void importAsterixFromNetworkSlot();

    void importJSONRecordingSlot();

    void importGPSTrailSlot();

    void importViewPointsSlot();

    void calculateRadarPlotPositionsSlot();
    void calculateAssociationsARTASSlot();
    void calculateAssociationsSlot();

    void quitRequestedSlot();
    void showAddViewMenuSlot();

    void resetViewsMenuSlot();

    void appModeSwitchSlot (AppMode app_mode_previous, AppMode app_mode_current);

    void autoResumeTimerSlot();
    void autoResumeResumeSlot();
    void autoResumeStaySlot();

public:
    MainWindow();
    virtual ~MainWindow();

    void disableConfigurationSaving();
    void showEvaluationTab();
    void showViewPointsTab();

    void openExistingDB(const std::string& filename);
    void createDB(const std::string& filename);

    void updateMenus();
    void updateBottomWidget();

protected:
    QTabWidget* tab_widget_{nullptr};

    QPushButton* add_view_button_{nullptr};

    bool save_configuration_{true};

    // menu

    // file menu
    QAction* new_db_action_ {nullptr};
    QAction* open_existing_db_action_ {nullptr};
    QMenu* open_recent_db_menu_ {nullptr};
    QAction* export_db_action_ {nullptr};
    QAction* close_db_action_ {nullptr};
    QAction* quit_wo_cfg_sav_action_ {nullptr};

    // configure sectors
    QAction* sectors_action_ {nullptr};

    // import menu
    QMenu* import_menu_ {nullptr};

    QMenu* import_recent_asterix_menu_ {nullptr};

    // configuration menu
    QMenu* config_menu_ {nullptr};

    // process menu
    QMenu* process_menu_ {nullptr};

    // ui menu
    QMenu* ui_menu_ {nullptr};

    bool loading_ {false};

    QLabel* db_label_ {nullptr};
    QLabel* status_label_ {nullptr};
    QPushButton* load_button_ {nullptr};

    QPushButton* live_pause_resume_button_ {nullptr};
    QPushButton* live_stop_button_ {nullptr}; // optional button, may be nullptr

    std::unique_ptr<AutoResumeDialog> auto_resume_dialog_;
    QTimer* auto_resume_timer_ {nullptr};

    void createMenus ();
    void createDebugMenu();

    /// @brief Called when application closes
    void closeEvent(QCloseEvent* event);

    void shutdown();

private:
    void showCommandShell();
};

//}
#endif /* MAINWINDOW_H_ */
