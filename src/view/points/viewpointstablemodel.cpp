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

#include "viewpointstablemodel.h"
#include "viewmanager.h"
#include "viewpoint.h"
#include "json.hpp"
#include "json.h"
#include "stringconv.h"
#include "files.h"
#include "compass.h"
#include "dbinterface.h"
#include "viewpointswidget.h"

#include <fstream>

#include <QMessageBox>

using namespace nlohmann;
using namespace Utils;
using namespace std;

ViewPointsTableModel::ViewPointsTableModel(ViewManager& view_manager)
    : view_manager_(view_manager)
{
    table_columns_ = default_table_columns_;

    open_icon_ = QIcon(Files::getIconFilepath("not_recommended.png").c_str());
    closed_icon_ = QIcon(Files::getIconFilepath("not_todo.png").c_str());
    todo_icon_ = QIcon(Files::getIconFilepath("todo.png").c_str());
    unknown_icon_ = QIcon(Files::getIconFilepath("todo_maybe.png").c_str());
}

//ViewPointsTableModel::~ViewPointsTableModel()
//{

//}


void ViewPointsTableModel::loadViewPoints()
{
    loginf << "ViewPointsTableModel: loadViewPoints";

    beginResetModel();

    // load view points
    if (COMPASS::instance().interface().existsViewPointsTable())
    {
        for (const auto& vp_it : COMPASS::instance().interface().viewPoints())
        {
            //assert (!view_points_.count(vp_it.first));
            assert (!hasViewPoint(vp_it.first));

            //view_points_.emplace_back(vp_it.first, vp_it.second, view_manager_, false);

            view_points_.push_back({vp_it.first, vp_it.second, view_manager_, false});  // args for mapped value
            // args for mapped value

            if (vp_it.first > max_id_)
                max_id_ = vp_it.first;
        }
    }

    updateTableColumns();
    updateTypes();
    updateStatuses();

    endResetModel();
}
void ViewPointsTableModel::clearViewPoints()
{
    loginf << "ViewPointsTableModel: clearViewPoints";

    beginResetModel();

    view_points_.clear();

    table_columns_ = default_table_columns_;
    types_.clear();
    statuses_.clear();

    endResetModel();
}

int ViewPointsTableModel::rowCount(const QModelIndex& parent) const
{
    return view_points_.size();
}

int ViewPointsTableModel::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QVariant ViewPointsTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        logdbg << "ViewPointsTableModel: data: display role: row " << index.row() << " col " << index.column();

        assert (index.row() >= 0);
        assert (index.row() < view_points_.size());

        const ViewPoint& vp = view_points_.at(index.row());

        logdbg << "ViewPointsTableModel: data: got key " << view_points_.at(index.row()).id();

        assert (index.column() < table_columns_.size());
        std::string col_name = table_columns_.at(index.column()).toStdString();

        if (!vp.data().contains(col_name))
            return QVariant();

        const json& data = vp.data().at(col_name);

        //            if (col_name == "status" && (data == "open" || data == "closed" || data == "todo"))
        //                return QVariant();

        // s1.find(s2) != std::string::npos
        if (data.is_number() && col_name.find("time") != std::string::npos)
            return String::timeStringFromDouble(data).c_str();

        if (data.is_boolean())
            return data.get<bool>();

        if (data.is_number())
            return data.get<float>();

        return JSON::toString(data).c_str();
    }
    case Qt::DecorationRole:
    {
        assert (index.column() < table_columns_.size());

        if (table_columns_.at(index.column()) == "status")
        {
            assert (index.row() >= 0);
            assert (index.row() < view_points_.size());

            const ViewPoint& vp = view_points_.at(index.row());

            const json& data = vp.data().at("status");
            assert (data.is_string());

            std::string status = data;

            if (status == "open")
                return open_icon_;
            else if (status == "closed")
                return closed_icon_;
            else if (status == "todo")
                return todo_icon_;
            else
                return unknown_icon_;
        }
        else
            return QVariant();
    }
    default:
    {
        return QVariant();
    }
    }
}

QVariant ViewPointsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex ViewPointsTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex ViewPointsTableModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags ViewPointsTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    if (table_columns_.at(index.column()) == "comment")
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    else
        return QAbstractItemModel::flags(index);
}

bool ViewPointsTableModel::setData(const QModelIndex& index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        unsigned int id = getIdOf(index);

        loginf << "ViewPointsTableModel: setData: row " << index.row() << " col " << index.column()
               << " id " << id << " '" << value.toString().toStdString() << "'";

        //assert (id < view_points_.size());

        auto it = view_points_.begin()+index.row();

        assert (index.column() == statusColumn() || index.column() == commentColumn());

        //        if (index.column() == statusColumn())
        //            view_points_.at(id).setStatus(value.toString().toStdString());
        //        else
        //            view_points_.at(id).setComment(value.toString().toStdString());

        if (index.column() == statusColumn())
        {
            view_points_.modify(it, [value](ViewPoint& p) { p.setStatus(value.toString().toStdString()); });
            updateStatuses();
        }
        else
            view_points_.modify(it, [value](ViewPoint& p) { p.setComment(value.toString().toStdString()); });

        emit dataChanged(index, index);

        return true;
    }
    return false;
}

//int ViewPointsTableModel::columnIndex (QString name)
//{
//    assert (table_columns_.contains(name));
//    return table_columns_.indexOf(name);
//}

bool ViewPointsTableModel::updateTableColumns()
{
    loginf << "ViewPointsTableModel: updateTableColumns";

    bool changed = false;

    for (auto& vp_it : view_points_)
    {
        const nlohmann::json& data = vp_it.data();

        assert (data.is_object());
        for (auto& j_it : data.get<json::object_t>())
        {
            logdbg << "ViewPointsTableModel: updateTableColumns: '" << j_it.first << "'";

            if (j_it.second.is_object() || j_it.second.is_array()) // skip complex items
                continue;

            if (!table_columns_.contains(j_it.first.c_str()))
            {
                beginInsertColumns(QModelIndex(), table_columns_.size(), table_columns_.size());
                table_columns_.append(j_it.first.c_str());
                endInsertColumns();

                changed = true;
            }
        }
    }

    return changed;
}

void ViewPointsTableModel::updateTypes()
{
    loginf << "ViewPointsTableModel: updateTypes";

    QStringList old_types = types_;
    types_.clear();

    for (auto& vp_it : view_points_)
    {
        const nlohmann::json& data = vp_it.data();

        assert (data.contains("type"));

        const string& type = data.at("type");

        if (!types_.contains(type.c_str()))
            types_.append(type.c_str());
    }

    if (types_ != old_types)
    {
        loginf << "ViewPointsTableModel: updateTypes: changed";

        emit typesChangedSignal(types_);
    }
}


void ViewPointsTableModel::updateStatuses()
{
    loginf << "ViewPointsTableModel: updateStatuses";

    QStringList old_statuses = statuses_;
    statuses_.clear();

    for (auto& vp_it : view_points_)
    {
        const nlohmann::json& data = vp_it.data();

        assert (data.contains("status"));

        const string& status = data.at("status");

        if (!statuses_.contains(status.c_str()))
            statuses_.append(status.c_str());
    }

    if (statuses_ != old_statuses)
    {
        loginf << "ViewPointsTableModel: updateStatuses: changed";

        emit statusesChangedSignal(statuses_);
    }
}
QStringList ViewPointsTableModel::types() const
{
    return types_;
}

QStringList ViewPointsTableModel::statuses() const
{
    return statuses_;
}

QStringList ViewPointsTableModel::tableColumns() const
{
    return table_columns_;
}

QStringList ViewPointsTableModel::defaultTableColumns() const
{
    return default_table_columns_;
}

const ViewPointCache& ViewPointsTableModel::viewPoints() const
{
    return view_points_;
}

bool ViewPointsTableModel::hasViewPoint (unsigned int id)
{
    return view_points_.get<vp_tag>().find(id) != view_points_.get<vp_tag>().end();
}

unsigned int ViewPointsTableModel::saveNewViewPoint(const nlohmann::json& data, bool update)
{
    unsigned int new_id = max_id_+1;

    assert (!hasViewPoint(new_id));

    nlohmann::json new_data = data;
    new_data["id"] = new_id;

    saveNewViewPoint(new_id, new_data, update); // auto increments max_id

    return new_id;
}

const ViewPoint& ViewPointsTableModel::saveNewViewPoint(unsigned int id, const nlohmann::json& data, bool update)
{
    if (hasViewPoint(id))
        throw std::runtime_error ("ViewPointsTableModel: addNewViewPoint: id "+std::to_string(id)+" already exists");

    unsigned int row = view_points_.size();

    if (update)
        beginInsertRows(QModelIndex(), row, row);

    nlohmann::json new_data = data;

    assert (new_data.is_object());
    json::object_t& new_data_ref = new_data.get_ref<json::object_t&>();

    view_points_.push_back({id, new_data_ref, view_manager_, true});

    if (id > max_id_)
        max_id_ = id;

    //    view_points_.emplace(std::piecewise_construct,
    //                         std::forward_as_tuple(id),   // args for key
    //                         std::forward_as_tuple(id, new_data_ref, view_manager_, true));  // args for mapped value

    assert (hasViewPoint(id));

    if (update)
    {
        emit endInsertRows();

        if (updateTableColumns()) // true if changed
            view_manager_.viewPointsWidget()->resizeColumnsToContents();

        updateTypes();
        updateStatuses();
    }

    return viewPoint(id);
}

//bool ViewPointsTableModel::existsViewPoint(unsigned int id)
//{
//    return view_points_.count(id) == 1;
//}

const ViewPoint& ViewPointsTableModel::viewPoint(unsigned int id)
{
    assert (hasViewPoint(id));
    return *view_points_.get<vp_tag>().find(id);
}

//void ViewPointsTableModel::removeViewPoint(unsigned int id)
//{
//    assert (existsViewPoint(id));

//    view_points_.erase(id);
//    ATSDB::instance().interface().deleteViewPoint(id);
//}

void ViewPointsTableModel::deleteAllViewPoints ()
{
    if (!view_points_.size())
        return;

    if (table_columns_.size() > default_table_columns_.size())
    {
        beginRemoveColumns(QModelIndex(), default_table_columns_.size(), table_columns_.size()-1);
        table_columns_ = default_table_columns_;
        endRemoveColumns();
    }

    view_manager_.unsetCurrentViewPoint();

    beginRemoveRows(QModelIndex(), 0, view_points_.size()-1); // TODO

    view_points_.clear();
    COMPASS::instance().interface().deleteAllViewPoints();

    endRemoveRows();
}

void ViewPointsTableModel::printViewPoints()
{
    for (auto& vp_it : view_points_)
        vp_it.print();
}

void ViewPointsTableModel::importViewPoints (const std::string& filename)
{
    loginf << "ViewPointsTableModel: importViewPoints: filename '" << filename << "'";

    try
    {
        if (!Files::fileExists(filename))
            throw std::runtime_error ("File '"+filename+"' not found.");

        std::ifstream ifs(filename);
        json j = json::parse(ifs);

        if (!j.contains("view_point_context"))
            throw std::runtime_error("File '"+filename+"' has no context information");

        json& context = j.at("view_point_context");

        if (!context.contains("version"))
            throw std::runtime_error("File '"+filename+"' context has no version");

        json& version = context.at("version");

        if (!version.is_string())
            throw std::runtime_error("File '"+filename+"' context version is not number");

        string version_str = version;

        if (version_str != "0.2") // old version, not possible
        {
            QMessageBox m_warning(QMessageBox::Warning, "Import View Points",
                                  ("View Points File has wrong version "+version_str
                                   +", only version 0.2 is supported in this version").c_str(),
                                  QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (version_str != "0.2")
            throw std::runtime_error("File '"+filename+"' context version "+version_str+" is not supported");

        loginf << "ViewPointsTableModel: importViewPoints: context '" << j.at("view_point_context").dump(4) << "'";

        if (!j.contains("view_points"))
            throw std::runtime_error ("File '"+filename+"' does not contain view points.");

        json& view_points = j.at("view_points");

        if (!view_points.is_array())
            throw std::runtime_error ("View points are not in an array.");

        unsigned int row_begin = view_points_.size();
        unsigned int row_end = row_begin+view_points.size()-1;

        beginInsertRows(QModelIndex(), row_begin, row_end);

        unsigned int id;
        for (auto& vp_it : view_points.get<json::array_t>())
        {
            if (!vp_it.contains("id"))
                throw std::runtime_error ("View point does not contain id");

            id = vp_it.at("id");

            if (!vp_it.contains("status"))
                vp_it["status"] = "open";

            saveNewViewPoint(id, vp_it, false);
        }

        endInsertRows();

        updateTableColumns();
        updateTypes();
        updateStatuses();

        //        if (view_points_widget_) // TODO
        //            view_points_widget_->update();

        QMessageBox m_info(QMessageBox::Information, "View Points Import File",
                           "File import: '"+QString(filename.c_str())+"' done.\n"
                           +QString::number(view_points.size())+" View Points added.", QMessageBox::Ok);
        m_info.exec();
    }
    catch (std::exception& e)
    {
        QMessageBox m_warning(QMessageBox::Warning, "View Points Import File",
                              "File import error: '"+QString(e.what())+"'.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }
}

void ViewPointsTableModel::exportViewPoints (const std::string& filename)
{
    loginf << "ViewPointsTableModel: exportViewPoints: filename '" << filename << "'";

    json data;

    data["view_point_context"] = json::object();
    json& context = data.at("view_point_context");
    context["version"] = "0.2";

    data["view_points"] = json::array();
    json& view_points = data.at("view_points");

    unsigned int cnt = 0;
    for (auto& vp_it : view_points_)
    {
        view_points[cnt] = vp_it.data();
        ++cnt;
    }

    std::ofstream file(filename);
    file << data.dump(4);

    QMessageBox m_info(QMessageBox::Information, "View Points Export File",
                       "File export: '"+QString(filename.c_str())+"' done.\n"
                       +QString::number(view_points.size())+" View Points saved.", QMessageBox::Ok);
    m_info.exec();
}

//void ViewPointsTableModel::update()
//{
//    loginf << "ViewPointsTableModel: update";
//    beginResetModel();
//    updateTableColumns();
//    endResetModel();
//}

unsigned int ViewPointsTableModel::getIdOf (const QModelIndex& index)
{
    assert (index.isValid());
    //    auto map_it = view_points_.begin();
    //    std::advance(map_it, index.row());
    //    assert (map_it != view_points_.end());

    assert (index.row() >= 0);
    assert (index.row() < view_points_.size());

    return view_points_.at(index.row()).id();

    //return map_it->first;
}

void ViewPointsTableModel::setStatus (const QModelIndex& row_index, const std::string& value)
{
    assert (row_index.isValid());

    QModelIndex status_index = index(row_index.row(), statusColumn(), QModelIndex());
    assert (status_index.isValid());

    setData(status_index, value.c_str(), Qt::EditRole);

    //    unsigned int id = getIdOf(index);
    //    assert (id < view_points_.size());
    //    loginf << "ViewPointsTableModel: setStatus: id " << id << " status " << value;
    //    view_points_.at(id).data()["status"] = value;
    //    view_points_.at(id).dirty(true);

    //    emit dataChanged(index, index, {Qt::UserRole});
}

