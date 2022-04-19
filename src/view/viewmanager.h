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

#ifndef VIEWMANAGER_H_
#define VIEWMANAGER_H_

#include <QObject>

#include "configurable.h"
#include "dbcontent/variable/variableset.h"
#include "appmode.h"

class COMPASS;
class Buffer;
class ViewContainer;
class ViewContainerWidget;
//class ViewManagerWidget;
class View;
class ViewableDataConfig;
class ViewPointsWidget;
class ViewPointsReportGenerator;

class QWidget;
class QTabWidget;

class ViewManager : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void selectionChangedSignal();
    void unshowViewPointSignal (const ViewableDataConfig* vp);
    void showViewPointSignal (const ViewableDataConfig* vp);

  public slots:
    void selectionChangedSlot();

    void databaseOpenedSlot();
    void databaseClosedSlot();

    void loadingStartedSlot();
    // all data contained, also new one. requires_reset true indicates that all shown info should be re-created,
    // e.g. when data in the beginning was removed, or order of previously emitted data was changed, etc.
    void loadedDataSlot (const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot(); // emitted when all dbos have finished loading

    void appModeSwitchSlot (AppMode app_mode);

  public:
    ViewManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~ViewManager();

    void init(QTabWidget* tab_widget);
    void loadViewPoints();
    void close();

    void clearDataInViews();

    void registerView(View* view);
    void unregisterView(View* view);
    bool isRegistered(View* view);

    ViewContainerWidget* addNewContainerWidget();

    // void deleteContainer (std::string instance_id);
    void removeContainer(std::string instance_id);
    void deleteContainerWidget(std::string instance_id);
    void removeContainerWidget(std::string instance_id);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    void viewShutdown(View* view, const std::string& err = "");

    std::map<std::string, ViewContainer*> getContainers() { return containers_; }
    std::map<std::string, View*> getViews() { return views_; }
    dbContent::VariableSet getReadSet(const std::string& dbo_name);

    //ViewManagerWidget* widget();

    ViewPointsWidget* viewPointsWidget() const;

    ViewPointsReportGenerator& viewPointsGenerator();

    void setCurrentViewPoint (const ViewableDataConfig* viewable);
    void unsetCurrentViewPoint ();

    void doViewPointAfterLoad ();
    void selectTimeWindow(float time_min, float time_max);

    void showMainViewContainerAddView();

    QStringList viewClassList() const;

    unsigned int newViewNumber();

    void disableDataDistribution(bool value);
    // disables propagation of data to the views. used when loading is performed for processing purposes


    bool isProcessingData() const;

protected:
    COMPASS& compass_;

    //ViewManagerWidget* widget_{nullptr};
    ViewPointsWidget* view_points_widget_{nullptr};

    bool initialized_{false};
    bool processing_data_ {false};

    QTabWidget* main_tab_widget_{nullptr};

    std::map<std::string, ViewContainer*> containers_;
    std::map<std::string, ViewContainerWidget*> container_widgets_;
    std::map<std::string, View*> views_;

    std::unique_ptr<ViewPointsReportGenerator> view_points_report_gen_;

    const ViewableDataConfig* current_viewable_ {nullptr};
    bool view_point_data_selected_ {false};

    unsigned int container_count_{0};

    QStringList view_class_list_;

    bool disable_data_distribution_ {false};

    virtual void checkSubConfigurables();
};

#endif /* VIEWMANAGER_H_ */
