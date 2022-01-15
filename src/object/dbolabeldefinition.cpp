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

#include "dbolabeldefinition.h"
#include "buffer.h"
#include "dbobject.h"
#include "dbolabeldefinitionwidget.h"
#include "dbovariable.h"
#include "global.h"
#include "propertylist.h"
#include "compass.h"
#include "dbobjectmanager.h"
#include "metadbovariable.h"

#include <iostream>
#include <string>

using namespace std;

DBOLabelEntry::DBOLabelEntry(const std::string& class_id, const std::string& instance_id,
                             DBOLabelDefinition* parent)
    : Configurable(class_id, instance_id, parent), def_parent_(parent)
{
    registerParameter("variable_name", &variable_name_, "");
    registerParameter("show", &show_, false);
    registerParameter("prefix", &prefix_, "");
    registerParameter("suffix", &suffix_, "");

    createSubConfigurables();
}

DBOLabelEntry::~DBOLabelEntry() {}

std::string DBOLabelEntry::variableName() const { return variable_name_; }

void DBOLabelEntry::variableName(const std::string& variable_name)
{
    variable_name_ = variable_name;
}

bool DBOLabelEntry::show() const { return show_; }

void DBOLabelEntry::show(bool show)
{
    show_ = show;

    assert(def_parent_);
    def_parent_->labelDefinitionChangedSlot();
}

std::string DBOLabelEntry::prefix() const { return prefix_; }

void DBOLabelEntry::prefix(const std::string& prefix)
{
    prefix_ = prefix;

    assert(def_parent_);
    def_parent_->labelDefinitionChangedSlot();
}

std::string DBOLabelEntry::suffix() const { return suffix_; }

void DBOLabelEntry::suffix(const std::string& suffix)
{
    suffix_ = suffix;

    assert(def_parent_);
    def_parent_->labelDefinitionChangedSlot();
}

DBOLabelDefinition::DBOLabelDefinition(const std::string& class_id, const std::string& instance_id,
                                       DBContent* parent, DBContentManager& dbo_man)
    : Configurable(class_id, instance_id, parent), db_object_(*parent), dbo_man_(dbo_man)
{
    createSubConfigurables();
}

DBOLabelDefinition::~DBOLabelDefinition()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : entries_)
    {
        delete it.second;
    }
    entries_.clear();

    read_list_.clear();
}

DBContentVariableSet& DBOLabelDefinition::readList()
{
    updateReadList();
    return read_list_;
}

void DBOLabelDefinition::updateReadList()
{
    read_list_.clear();

    for (auto it : entries_)
    {
        if (it.second->show())
        {
            assert(db_object_.hasVariable(it.second->variableName()));
            read_list_.add(db_object_.variable(it.second->variableName()));
        }
    }
}

void DBOLabelDefinition::checkLabelDefinitions()
{
    loginf << "DBOLabelDefinition: checkLabelDefinitions: dbo " << db_object_.name();

    std::string variable_name;
    bool show = false;
    std::string prefix;
    std::string suffix;

    string dbo_name = db_object_.name();

    for (auto& var_it : db_object_.variables())
    {
        if (entries_.find(var_it->name()) == entries_.end())
        {
            variable_name = var_it->name();
            show = false;
            prefix = "";
            suffix = "";

            logdbg << "DBOLabelDefinition: checkSubConfigurables: new var " << variable_name;

            if (dbo_man_.metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(dbo_name)
                    && variable_name == dbo_man_.metaVariable(
                        DBContent::meta_var_datasource_id_.name()).getFor(dbo_name).name())
            {
                show = true;
                prefix = "S ";
            }
            else if (dbo_man_.metaVariable(DBContent::meta_var_tod_id_.name()).existsIn(dbo_name)
                    && variable_name == dbo_man_.metaVariable(
                        DBContent::meta_var_tod_id_.name()).getFor(dbo_name).name())
            {
                show = true;
                prefix = "T ";
            }
            else if (dbo_man_.metaVariable(DBContent::meta_var_m3a_id_.name()).existsIn(dbo_name)
                     && variable_name == dbo_man_.metaVariable(
                         DBContent::meta_var_m3a_id_.name()).getFor(dbo_name).name())
            {
                show = true;
                prefix = "A ";
            }
            else if (dbo_man_.metaVariable(DBContent::meta_var_ta_id_.name()).existsIn(dbo_name)
                     && variable_name == dbo_man_.metaVariable(
                         DBContent::meta_var_ta_id_.name()).getFor(dbo_name).name())
            {
                show = true;
                prefix = "AA ";
            }
            else if (dbo_man_.metaVariable(DBContent::meta_var_ti_id_.name()).existsIn(dbo_name)
                     && variable_name == dbo_man_.metaVariable(
                         DBContent::meta_var_ti_id_.name()).getFor(dbo_name).name())
            {
                show = true;
                prefix = "ID ";
            }
            else if (dbo_man_.metaVariable(DBContent::meta_var_track_num_id_.name()).existsIn(dbo_name)
                     && variable_name == dbo_man_.metaVariable(
                         DBContent::meta_var_track_num_id_.name()).getFor(dbo_name).name())
            {
                show = true;
                prefix = "TN ";
            }
            else if (dbo_man_.metaVariable(DBContent::meta_var_mc_id_.name()).existsIn(dbo_name)
                     && variable_name == dbo_man_.metaVariable(
                         DBContent::meta_var_mc_id_.name()).getFor(dbo_name).name())
            {
                show = true;
                prefix = "C ";
            }

            std::string instance_id = "LabelEntry" + variable_name + "0";

            Configuration& configuration = addNewSubConfiguration("LabelEntry", instance_id);
            configuration.addParameterString("variable_name", variable_name);
            configuration.addParameterBool("show", show);
            configuration.addParameterString("prefix", prefix);
            configuration.addParameterString("suffix", suffix);
            generateSubConfigurable("LabelEntry", instance_id);
        }
    }
}

void DBOLabelDefinition::generateSubConfigurable(const std::string& class_id,
                                                 const std::string& instance_id)
{
    if (class_id == "LabelEntry")
    {
        logdbg << "DBOLabelDefinition: generateSubConfigurable: instance_id " << instance_id;

        DBOLabelEntry* entry = new DBOLabelEntry(class_id, instance_id, this);
        assert(entries_.find(entry->variableName()) == entries_.end());
        entries_[entry->variableName()] = entry;
    }
    else
        throw std::runtime_error("DBOLabelDefinition: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void DBOLabelDefinition::checkSubConfigurables()
{
    logdbg << "DBOLabelDefinition: checkSubConfigurables: object " << db_object_.name();
}
DBOLabelEntry& DBOLabelDefinition::entry(const std::string& variable_name)
{
    return *entries_.at(variable_name);
}

DBContentLabelDefinitionWidget* DBOLabelDefinition::widget()
{
    if (!widget_)
    {
        //checkLabelDefinitions();

        widget_ = new DBContentLabelDefinitionWidget(this);
        assert(widget_);
    }
    return widget_;
}

std::map<unsigned int, std::string> DBOLabelDefinition::generateLabels(std::vector<unsigned int> rec_nums,
                                                              std::shared_ptr<Buffer> buffer,
                                                              int break_item_cnt)
{
    const PropertyList& buffer_properties = buffer->properties();
    assert(buffer_properties.size() >= read_list_.getSize());

    assert(buffer->size() == rec_nums.size());

    // check and insert strings for with rec_num
    std::map<unsigned int, std::string> labels;

    DBContentManager& dbo_man_ = COMPASS::instance().objectManager();

    string dbo_name = db_object_.name();

    assert (dbo_man_.existsMetaVariable(DBContent::meta_var_rec_num_id_.name()));
    assert (dbo_man_.metaVariable(DBContent::meta_var_rec_num_id_.name()).existsIn(dbo_name));

    DBContentVariable& rec_num_var = dbo_man_.metaVariable(DBContent::meta_var_rec_num_id_.name()).getFor(dbo_name);

    std::map<unsigned int, size_t> rec_num_to_index;
    NullableVector<unsigned int>& rec_num_list = buffer->get<unsigned int>(rec_num_var.dbColumnName());

    for (size_t cnt = 0; cnt < rec_num_list.size(); cnt++)
    {
        assert(!rec_num_list.isNull(cnt));
        int rec_num = rec_num_list.get(cnt);
        assert(labels.count(rec_num) == 0);
        rec_num_to_index[rec_num] = cnt;
    }

    PropertyDataType data_type;
    string db_col_name;
    std::string value_str;
    DBOLabelEntry* entry;
    bool null;

    int var_count = 0;
    for (DBContentVariable* variable : read_list_.getSet())
    {
        data_type = variable->dataType();
        db_col_name = variable->dbColumnName();
        value_str = NULL_STRING;
        entry = entries_[variable->name()];
        assert(entry);

        for (size_t cnt = 0; cnt < rec_num_list.size(); cnt++)
        {
            int rec_num = rec_num_list.get(cnt);  // already checked

            assert(rec_num_to_index.count(rec_num) == 1);
            size_t buffer_index = rec_num_to_index[rec_num];

            if (data_type == PropertyDataType::BOOL)
            {
                assert(buffer->has<bool>(db_col_name));
                null = buffer->get<bool>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<bool>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::CHAR)
            {
                assert(buffer->has<char>(db_col_name));
                null = buffer->get<char>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<char>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::UCHAR)
            {
                assert(buffer->has<unsigned char>(db_col_name));
                null = buffer->get<unsigned char>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<unsigned char>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::INT)
            {
                assert(buffer->has<int>(db_col_name));
                null = buffer->get<int>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<int>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::UINT)
            {
                assert(buffer->has<unsigned int>(db_col_name));
                null = buffer->get<unsigned int>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<unsigned int>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::LONGINT)
            {
                assert(buffer->has<long int>(db_col_name));
                null = buffer->get<long int>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<long int>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::ULONGINT)
            {
                assert(buffer->has<unsigned long int>(db_col_name));
                null = buffer->get<unsigned long int>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<unsigned long int>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::FLOAT)
            {
                assert(buffer->has<float>(db_col_name));
                null = buffer->get<float>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<float>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::DOUBLE)
            {
                assert(buffer->has<double>(db_col_name));
                null = buffer->get<double>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<double>(db_col_name).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::STRING)
            {
                assert(buffer->has<std::string>(db_col_name));
                null = buffer->get<std::string>(db_col_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                        buffer->get<std::string>(db_col_name).getAsString(buffer_index));
                }
            }
            else
                throw std::domain_error(
                    "DBOLabelDefinition: generateLabels: unknown property data type");

            if (!null)
            {
                if (var_count == break_item_cnt - 1)
                {
                    labels[rec_num] += "\n";
                }
                else if (labels[rec_num].size() != 0)
                {
                    labels[rec_num] += " ";
                }

                labels[rec_num] += entry->prefix() + value_str + entry->suffix();
            }
        }

        var_count++;

        if (var_count == break_item_cnt)
            var_count = 0;
    }

    if (read_list_.getSet().size() == 0)
        for (auto rec_num : rec_nums)
            labels[rec_num] = "Label Definition empty";

    return labels;
}

void DBOLabelDefinition::labelDefinitionChangedSlot()
{
    emit db_object_.labelDefinitionChangedSignal();
}
