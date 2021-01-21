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

#include "histogramviewdatasource.h"

#include <QMessageBox>

#include "compass.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "job.h"
#include "logger.h"
#include "viewpoint.h"

#include <algorithm>
#include <cassert>

#include "json.hpp"

using namespace std;
using namespace nlohmann;

HistogramViewDataSource::HistogramViewDataSource(const std::string& class_id,
                                             const std::string& instance_id, Configurable* parent)
    : QObject(),
      Configurable(class_id, instance_id, parent),
      selection_entries_(ViewSelection::getInstance().getEntries())
{
    // registerParameter ("use_selection", &use_selection_, true);

    connect(&COMPASS::instance().objectManager(), SIGNAL(loadingStartedSignal()), this,
            SLOT(loadingStartedSlot()));

    for (auto& obj_it : COMPASS::instance().objectManager())
    {
        connect(obj_it.second, SIGNAL(newDataSignal(DBObject&)), this,
                SLOT(newDataSlot(DBObject&)));
        connect(obj_it.second, SIGNAL(loadingDoneSignal(DBObject&)), this,
                SLOT(loadingDoneSlot(DBObject&)));
    }

    createSubConfigurables();
}

HistogramViewDataSource::~HistogramViewDataSource()
{
    unshowViewPoint(nullptr); // removes tmps TODO not done yet

    if (set_)
    {
        delete set_;
        set_ = nullptr;
    }
}

void HistogramViewDataSource::generateSubConfigurable(const std::string& class_id,
                                                    const std::string& instance_id)
{
    logdbg << "HistogramViewDataSource: generateSubConfigurable: class_id " << class_id
           << " instance_id " << instance_id;

    if (class_id.compare("DBOVariableOrderedSet") == 0)
    {
        assert(set_ == 0);
        set_ = new DBOVariableOrderedSet(class_id, instance_id, this);
    }
    else
        throw std::runtime_error(
            "HistogramViewDataSource: generateSubConfigurable: unknown class_id " + class_id);
}

void HistogramViewDataSource::checkSubConfigurables()
{
    if (set_ == nullptr)
    {
        generateSubConfigurable("DBOVariableOrderedSet", "DBOVariableOrderedSet0");
        assert(set_);

        DBObjectManager& obj_man = COMPASS::instance().objectManager();

        if (obj_man.existsMetaVariable("rec_num"))
            set_->add(obj_man.metaVariable("rec_num"));
    }
}

void HistogramViewDataSource::unshowViewPoint (const ViewableDataConfig* vp)
{
    for (auto& var_it : temporary_added_variables_)
    {
        loginf << "HistogramViewDataSource: unshowViewPoint: removing var " << var_it.first << ", " << var_it.second;

        removeTemporaryVariable(var_it.first, var_it.second);
    }

    temporary_added_variables_.clear();
}

void HistogramViewDataSource::showViewPoint (const ViewableDataConfig* vp)
{
    assert (vp);
    const json& data = vp->data();

//    "context_variables": {
//        "Tracker": [
//            "mof_long",
//            "mof_trans",
//            "mof_vert"
//        ]
//    }

    if (data.contains("context_variables"))
    {
        const json& context_variables = data.at("context_variables");
        assert (context_variables.is_object());

        for (auto& obj_it : context_variables.get<json::object_t>())
        {
            string dbo_name = obj_it.first;
            json& variable_names = obj_it.second;

            assert (variable_names.is_array());

            for (auto& var_it : variable_names.get<json::array_t>())
            {
                string var_name = var_it;

                if (addTemporaryVariable(dbo_name, var_name))
                {
                    loginf << "HistogramViewDataSource: showViewPoint: added var " << dbo_name << ", " << var_name;
                    temporary_added_variables_.push_back({dbo_name, var_name});
                }
            }
        }
    }
}

bool HistogramViewDataSource::addTemporaryVariable (const std::string& dbo_name, const std::string& var_name)
{
    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    if (dbo_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaDBOVariable& meta_var = obj_man.metaVariable(var_name);
        if (!set_->hasMetaVariable(meta_var))
        {
            set_->add(meta_var);
            return true;
        }
        else
            return false;
    }
    else
    {
        assert (obj_man.existsObject(dbo_name));
        DBObject& obj = obj_man.object(dbo_name);

        assert (obj.hasVariable(var_name));
        DBOVariable& var = obj.variable(var_name);

        if (!set_->hasVariable(var))
        {
            set_->add(var);
            return true;
        }
        else
            return false;
    }
}

void HistogramViewDataSource::removeTemporaryVariable (const std::string& dbo_name, const std::string& var_name)
{
    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    if (dbo_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaDBOVariable& meta_var = obj_man.metaVariable(var_name);
        assert (set_->hasMetaVariable(meta_var));
        set_->removeMetaVariable(meta_var);
    }
    else
    {
        assert (obj_man.existsObject(dbo_name));
        DBObject& obj = obj_man.object(dbo_name);

        assert (obj.hasVariable(var_name));
        DBOVariable& var = obj.variable(var_name);
        assert (set_->hasVariable(var));
        set_->removeVariable(var);
    }
}

void HistogramViewDataSource::loadingStartedSlot()
{
    logdbg << "HistogramViewDataSource: loadingStartedSlot";
    emit loadingStartedSignal();
}

void HistogramViewDataSource::newDataSlot(DBObject& object)
{
    logdbg << "HistogramViewDataSource: newDataSlot: object " << object.name();

    std::shared_ptr<Buffer> buffer = object.data();
    assert(buffer);

    emit updateDataSignal(object, buffer);
}

void HistogramViewDataSource::loadingDoneSlot(DBObject& object)
{
    logdbg << "HistogramViewDataSource: loadingDoneSlot: object " << object.name();
}
