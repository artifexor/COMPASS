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

#include "dbschema.h"

#include "dbschemawidget.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "logger.h"
#include "metadbtable.h"

#include <boost/algorithm/string.hpp>

DBSchema::DBSchema(const std::string& class_id, const std::string& instance_id,
                   Configurable* parent, DBInterface& db_interface)
    : Configurable(class_id, instance_id, parent), db_interface_(db_interface)
{
    createSubConfigurables();
}

DBSchema::~DBSchema()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : tables_)
        delete it.second;
    tables_.clear();

    for (auto it : meta_tables_)
        delete it.second;
    meta_tables_.clear();
}

void DBSchema::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "DBSchema: generateSubConfigurable: " << classId() << " instance " << instanceId();

    if (class_id.compare("DBTable") == 0)
    {
        loginf << "DBSchema: generateSubConfigurable: generating DBTable "
               << instance_id;
        DBTable* table = new DBTable("DBTable", instance_id, *this, db_interface_);
        assert(table->name().size() != 0);
        assert(tables_.find(table->name()) == tables_.end());
        tables_.insert(std::pair<std::string, DBTable*>(table->name(), table));
        logdbg << "DBSchema: generateSubConfigurable: generated DBTable "
               << table->name();
    }
    else if (class_id.compare("MetaDBTable") == 0)
    {
        logdbg << "DBSchema: generateSubConfigurable: generating MetaDBTable "
               << instance_id;
        MetaDBTable* meta_table = new MetaDBTable("MetaDBTable", instance_id, this, db_interface_);
        assert(meta_table->name().size() != 0);
        assert(meta_tables_.find(meta_table->name()) == meta_tables_.end());
        meta_tables_.insert(std::pair<std::string, MetaDBTable*>(meta_table->name(), meta_table));
        logdbg << "DBSchema: generateSubConfigurable: generated MetaDBTable "
               << meta_table->name();
    }
    else
        throw std::runtime_error("DBSchema: generateSubConfigurable: unknown class_id " + class_id);
}

DBTable& DBSchema::table(const std::string& name) const
{
    assert(tables_.find(name) != tables_.end());
    return *tables_.at(name);
}

void DBSchema::addTable(const std::string& name)
{
    assert(!hasTable(name));
    assert(!hasSubConfigurable("DBTable", name));

    Configuration& table_config = addNewSubConfiguration("DBTable", name);
    table_config.addParameterString("name", name);

    generateSubConfigurable("DBTable", name);
    assert(hasTable(name));

    emit changedSignal();
}

void DBSchema::deleteTable(const std::string& name)
{
    loginf << "DBSchema: deleteTable: name " << name;

    assert(hasTable(name));
    delete tables_.at(name);
    tables_.erase(name);
    assert(!hasTable(name));

    for (auto it : meta_tables_)
    {
        if (it.second->mainTableName() == name)
        {
            deleteMetaTable(it.first);
            continue;
        }

        if (it.second->hasSubTable(name))
            it.second->removeSubTable(name);
    }

    emit changedSignal();
}

void DBSchema::populateTable(const std::string& name)
{
    assert(hasTable(name));
    tables_.at(name)->populate();

    emit changedSignal();
}

bool DBSchema::hasMetaTable(const std::string& name) const
{
    return meta_tables_.find(name) != meta_tables_.end();
}

MetaDBTable& DBSchema::metaTable(const std::string& name) const
{
    assert(hasMetaTable(name));
    return *meta_tables_.at(name);
}

void DBSchema::addMetaTable(const std::string& name, const std::string& main_table_name)
{
    assert(!hasMetaTable(name));
    assert(!hasSubConfigurable("MetaDBTable", name));

    Configuration& table_config = addNewSubConfiguration("MetaDBTable", "MetaDBTable" + name);
    table_config.addParameterString("name", name);
    table_config.addParameterString("main_table_name", main_table_name);

    generateSubConfigurable("MetaDBTable", "MetaDBTable" + name);
    assert(hasMetaTable(name));

    emit changedSignal();
}

void DBSchema::deleteMetaTable(const std::string& name)
{
    assert(hasMetaTable(name));
    delete meta_tables_.at(name);
    meta_tables_.erase(name);

    emit changedSignal();
}

void DBSchema::updateTables()
{
    logdbg << "DBSchema: updateTables";
    std::map<std::string, DBTable*> old_tables = tables_;
    tables_.clear();

    for (auto it : old_tables)
    {
        tables_.insert(std::pair<std::string, DBTable*>(it.second->name(), it.second));
    }
}

void DBSchema::updateMetaTables()
{
    logdbg << "DBSchema: updateMetaTables";
    std::map<std::string, MetaDBTable*> old_meta_tables = meta_tables_;
    meta_tables_.clear();

    for (auto it : old_meta_tables)
    {
        meta_tables_.insert(
            std::pair<std::string, MetaDBTable*>(it.second->instanceId(), it.second));
    }
}

DBSchemaWidget* DBSchema::widget()
{
    if (!widget_)
    {
        widget_ = new DBSchemaWidget(*this);
    }

    assert(widget_);
    return widget_;
}

void DBSchema::lock()
{
    locked_ = true;

    for (auto& table_it : tables_)
        table_it.second->lock();

    for (auto& meta_table_it : meta_tables_)
        meta_table_it.second->lock();

    if (widget_)
        widget_->lock();
}

void DBSchema::updateOnDatabase()
{
    exists_in_db_ = false;

    for (auto& table_it : tables_)
    {
        table_it.second->updateOnDatabase();

        exists_in_db_ = exists_in_db_ | table_it.second->existsInDB();
    }

    for (auto& meta_table_it : meta_tables_)
        meta_table_it.second->updateOnDatabase();

    logdbg << "DBSchema: updateOnDatabase: exists in db " << exists_in_db_;
}
