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

#include "dbovariable.h"

#include <algorithm>

#include "compass.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariablewidget.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "metadbtable.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"

using namespace Utils;

std::map<DBOVariable::Representation, std::string> DBOVariable::representation_2_string_{
    {DBOVariable::Representation::STANDARD, "STANDARD"},
    {DBOVariable::Representation::SECONDS_TO_TIME, "SECONDS_TO_TIME"},
    {DBOVariable::Representation::DEC_TO_OCTAL, "DEC_TO_OCTAL"},
    {DBOVariable::Representation::DEC_TO_HEX, "DEC_TO_HEX"},
    {DBOVariable::Representation::FEET_TO_FLIGHTLEVEL, "FEET_TO_FLIGHTLEVEL"},
    {DBOVariable::Representation::DATA_SRC_NAME, "DATA_SRC_NAME"}};

std::map<std::string, DBOVariable::Representation> DBOVariable::string_2_representation_{
    {"STANDARD", DBOVariable::Representation::STANDARD},
    {"SECONDS_TO_TIME", DBOVariable::Representation::SECONDS_TO_TIME},
    {"DEC_TO_OCTAL", DBOVariable::Representation::DEC_TO_OCTAL},
    {"DEC_TO_HEX", DBOVariable::Representation::DEC_TO_HEX},
    {"FEET_TO_FLIGHTLEVEL", DBOVariable::Representation::FEET_TO_FLIGHTLEVEL},
    {"DATA_SRC_NAME", DBOVariable::Representation::DATA_SRC_NAME}};

DBOVariable::Representation DBOVariable::stringToRepresentation(
    const std::string& representation_str)
{
    assert(string_2_representation_.count(representation_str) == 1);
    return string_2_representation_.at(representation_str);
}

std::string DBOVariable::representationToString(Representation representation)
{
    assert(representation_2_string_.count(representation) == 1);
    return representation_2_string_.at(representation);
}

DBOVariable::DBOVariable(const std::string& class_id, const std::string& instance_id,
                         DBObject* parent)
    : Property(), Configurable(class_id, instance_id, parent), db_object_(parent)
{
    registerParameter("name", &name_, "");
    registerParameter("description", &description_, "");
    registerParameter("data_type_str", &data_type_str_, "");
    registerParameter("representation_str", &representation_str_, "");
    registerParameter("dimension", &dimension_, "");
    registerParameter("unit", &unit_, "");

    assert(name_.size() > 0);
    assert(data_type_str_.size() > 0);
    data_type_ = Property::asDataType(data_type_str_);

    if (representation_str_.size() == 0)
    {
        representation_str_ = representationToString(Representation::STANDARD);
    }

    representation_ = stringToRepresentation(representation_str_);

    // loginf  << "DBOVariable: constructor: name " << id_ << " unitdim '" << unit_dimension_ << "'
    // unitunit '" << unit_unit_ << "'";

    createSubConfigurables();
}

DBOVariable& DBOVariable::operator=(DBOVariable&& other)
//: Configurable(std::move(other))
{
    loginf << "DBOVariable: move operator: moving";

    data_type_ = other.data_type_;
    data_type_str_ = other.data_type_str_;

    name_ = other.name_;
    other.name_ = "";

    db_object_ = other.db_object_;
    other.db_object_ = nullptr;

    representation_str_ = other.representation_str_;
    other.representation_str_ = "";

    representation_ = other.representation_;
    other.representation_ = Representation::STANDARD;

    description_ = other.description_;
    other.description_ = "";

    min_max_set_ = other.min_max_set_;
    other.min_max_set_ = false;

    min_ = other.min_;
    other.min_ = "";

    max_ = other.max_;
    other.max_ = "";

    dimension_ = other.dimension_;
    other.dimension_ = "";

    unit_ = other.unit_;
    other.unit_ = "";

    schema_variables_ = other.schema_variables_;
    other.schema_variables_.clear();

    widget_ = other.widget_;
    if (widget_)
        widget_->setVariable(*this);
    other.widget_ = nullptr;

    other.configuration().updateParameterPointer("name", &name_);
    other.configuration().updateParameterPointer("description", &description_);
    other.configuration().updateParameterPointer("data_type_str", &data_type_str_);
    other.configuration().updateParameterPointer("representation_str", &representation_str_);
    other.configuration().updateParameterPointer("dimension", &dimension_);
    other.configuration().updateParameterPointer("unit", &unit_);

    // return *this;
    return static_cast<DBOVariable&>(Configurable::operator=(std::move(other)));
}

DBOVariable::~DBOVariable()
{
    for (auto it : schema_variables_)
        delete it.second;
    schema_variables_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void DBOVariable::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    if (class_id.compare("DBOSchemaVariableDefinition") == 0)
    {
        DBOSchemaVariableDefinition* definition =
            new DBOSchemaVariableDefinition(class_id, instance_id, this);
        assert(schema_variables_.find(definition->getSchema()) == schema_variables_.end());
        schema_variables_[definition->getSchema()] = definition;
    }
    else
        throw std::runtime_error("DBOVariable: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

// DBOVariable& DBOVariable::operator=(DBOVariable&& other)
//{
//    db_object_ = *other.db_object_;

//    representation_str_ = other.representation_str_;
//    other.representation_str_ = "";

//    representation_ = other.representation_;
//    other.representation_ = Representation.STANDARD;

//    description_ = other.description_;

//    min_max_set_ = other.min_max_set_;
//    min_ = min_;
//    max_ = max_;

//    dimension_ = dimension_;
//    unit_ = unit_;

//    schema_variables_.insert(make_move_iterator(begin(other.schema_variables_)),
//             make_move_iterator(end(other.schema_variables_)));

//    widget_ = other.widget_;

//    locked_ = other.locked_;

//    return *this;
//}

bool DBOVariable::operator==(const DBOVariable& var)
{
    if (dboName() != var.dboName())
        return false;
    if (data_type_ != var.data_type_)
        return false;
    if (name_.compare(var.name_) != 0)
        return false;

    return true;
}

void DBOVariable::print()
{
    loginf << "DBOVariable: print: dbo " << Configurable::parent().instanceId() << " id " << name_
           << " data type " << data_type_str_;
}

//    if (transform)
//    {
//        logdbg  << "DBOVariable: getValueFromRepresentation: var " << id_ << " representation " <<
//        representation_string;

//        DBOVariable *variable;

//        if (isMetaVariable())
//            variable = getFirst();
//        else
//            variable = this;

//        std::string meta_tablename = variable->getCurrentMetaTable ();
//        std::string table_varname = variable->getCurrentVariableName ();

//        DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema
//        ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

//        if (hasUnit () || table_column->hasUnit())
//        {
//            //loginf  << "var type " << variable->getDBOType() << " dim '" <<
//            variable->getUnitDimension() << "'"; if (variable->hasUnit () !=
//            table_column->hasUnit())
//            {
//                logerr << "DBOVariable: getValueFromRepresentation: unit transformation
//                inconsistent: var " << variable->getName ()
//                                            << " has unit " << hasUnit () << " table column " <<
//                                            table_column->getName() << " has unit "
//                                            << table_column->hasUnit();
//                throw std::runtime_error ("DBOVariable: getValueFromRepresentation: tranformation
//                error 1");
//            }

//            if (variable->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
//            {
//                logerr << "DBOVariable: getValueFromRepresentation: unit transformation
//                inconsistent: var "
//                        << variable->getName () << " has dimension " << getUnitDimension () << "
//                        table column "
//                        << table_column->getName() << " has dimension " <<
//                        table_column->getUnitDimension();
//                throw std::runtime_error ("DBOVariable: getValueFromRepresentation: tranformation
//                error 2");
//            }

//            Unit *unit = UnitManager::getInstance().getUnit (variable->getUnitDimension());
//            double factor = unit->getFactor (variable->getUnitUnit(),
//            table_column->getUnitUnit()); logdbg  << "DBOVariable: getValueFromRepresentation:
//            correct unit transformation with factor " << factor;

//            double var = doubleFromString(ss.str());
//            var *= factor;
//            std::string transformed = doubleToString (var);

//            logdbg  << "DBOVariable: getValueFromRepresentation: var " << id_ << " transformed
//            representation " << transformed; return transformed;
//        }
//        else
//            return ss.str();
//    }
//    else
//        return ss.str();
//}

void DBOVariable::checkSubConfigurables()
{
    //    if (!hasCurrentSchema())
    //    {
    //        std::string schema_name = COMPASS::instance().schemaManager().getCurrentSchemaName();
    //        std::string instance = schema_name+"0";
    //        std::string meta_table_name = dbo_parent_.name();

    //        loginf << "DBOVariable: checkSubConfigurables: creating new schema definition for " <<
    //        schema_name;

    //        Configuration &config = addNewSubConfiguration ("DBOSchemaVariableDefinition",
    //        instance);

    //        config.addParameterString ("schema", schema_name);
    //        config.addParameterString ("meta_table", meta_table_name);
    //        config.addParameterString ("variable_identifier", "");

    //        generateSubConfigurable("DBOSchemaVariableDefinition", instance);
    //    }
}

const std::string& DBOVariable::dboName() const
{
    assert(db_object_);
    return db_object_->name();
}

void DBOVariable::name(const std::string& name)
{
    loginf << "DBOVariable: name: old " << name_ << " new " << name;
    name_ = name;
}

bool DBOVariable::hasSchema(const std::string& schema) const
{
    // return schema_variables_.find (schema) != schema_variables_.end();
    assert(db_object_);
    return schema_variables_.find(schema) != schema_variables_.end() &&
           db_object_->hasMetaTable(schema);
}

const std::string& DBOVariable::metaTable(const std::string& schema) const
{
    assert(hasSchema(schema));
    // return schema_variables_.at(schema)->getMetaTable();
    assert(db_object_);
    return db_object_->metaTable(schema);
}

bool DBOVariable::hasVariableName(const std::string& schema) const
{
    if (!hasSchema(schema))
        return false;

    return schema_variables_.count(schema) != 0;
}

const std::string& DBOVariable::variableName(const std::string& schema) const
{
    assert(hasVariableName(schema));
    return schema_variables_.at(schema)->getVariableIdentifier();
}

void DBOVariable::setVariableName(const std::string& schema_name, const std::string& name)
{
    if (hasVariableName(schema_name))
    {
        logdbg << "DBOVariable: setVariableName: setting in existing schema def";
        schema_variables_.at(schema_name)->setVariableIdentifier(name);
    }
    else
    {
        logdbg << "DBOVariable: setVariableName: creating new";
        assert(db_object_);
        std::string var_instance = "DBOSchemaVariableDefinition" + db_object_->name() + name + "0";

        Configuration& var_configuration =
            addNewSubConfiguration("DBOSchemaVariableDefinition", var_instance);
        var_configuration.addParameterString("schema", schema_name);
        // var_configuration.addParameterString ("meta_table", meta.name());
        var_configuration.addParameterString("variable_identifier", name);

        generateSubConfigurable("DBOSchemaVariableDefinition", var_instance);
        assert(hasSchema(schema_name));
    }
}

bool DBOVariable::hasCurrentDBColumn() const
{
    assert(db_object_);

    if (!db_object_->hasCurrentMetaTable())
        return false;

    if (!hasCurrentSchema())
        return false;

    std::string meta_tablename = currentMetaTableString();
    std::string meta_table_varid = currentVariableIdentifier();

    logdbg << "DBOVariable: hasCurrentDBColumn: meta " << meta_tablename << " variable id "
           << meta_table_varid;

    assert(COMPASS::instance().schemaManager().hasCurrentSchema());
    assert(COMPASS::instance().schemaManager().getCurrentSchema().hasMetaTable(meta_tablename));

    return COMPASS::instance()
        .schemaManager()
        .getCurrentSchema()
        .metaTable(meta_tablename)
        .hasColumn(meta_table_varid);
}

const DBTableColumn& DBOVariable::currentDBColumn() const
{
    assert(hasCurrentDBColumn());

    std::string meta_tablename = currentMetaTableString();
    std::string meta_table_varid = currentVariableIdentifier();

    logdbg << "DBOVariable: currentDBColumn: meta " << meta_tablename << " variable id "
           << meta_table_varid;

    return COMPASS::instance()
        .schemaManager()
        .getCurrentSchema()
        .metaTable(meta_tablename)
        .column(meta_table_varid);
}

bool DBOVariable::isKey() { return hasCurrentDBColumn() && currentDBColumn().isKey(); }

bool DBOVariable::hasCurrentSchema() const
{
    return hasSchema(COMPASS::instance().schemaManager().getCurrentSchemaName());
}

const std::string& DBOVariable::currentMetaTableString() const
{
    assert(db_object_);
    assert(db_object_->hasCurrentMetaTable());
    return db_object_->currentMetaTable().name();
}

const MetaDBTable& DBOVariable::currentMetaTable() const
{
    assert(db_object_);
    assert(db_object_->hasCurrentMetaTable());
    return db_object_->currentMetaTable();
}

const std::string& DBOVariable::currentVariableIdentifier() const
{
    assert(hasCurrentSchema());
    std::string schema = COMPASS::instance().schemaManager().getCurrentSchemaName();
    assert(schema_variables_.find(schema) != schema_variables_.end());
    return schema_variables_.at(schema)->getVariableIdentifier();
}

void DBOVariable::setMinMax()
{
    assert(!min_max_set_);

    assert(db_object_);
    logdbg << "DBOVariable " << db_object_->name() << " " << name_ << ": setMinMax";

    if (!dbObject().existsInDB()  // object doesn't exist in this database
        || !dbObject().count() || !existsInDB())
    {
        min_ = NULL_STRING;
        max_ = NULL_STRING;
    }
    else
    {
        std::pair<std::string, std::string> min_max =
            COMPASS::instance().interface().getMinMaxString(*this);

        min_ = min_max.first;
        max_ = min_max.second;
    }

    min_max_set_ = true;

    logdbg << "DBOVariable: setMinMax: min " << min_ << " max " << max_;
}

std::string DBOVariable::getMinString()
{
    if (!min_max_set_)
        setMinMax();  // already unit transformed

    assert(min_max_set_);

    logdbg << "DBOVariable: getMinString: object " << dboName() << " name " << name()
           << " returning " << min_;
    return min_;
}

std::string DBOVariable::getMaxString()
{
    if (!min_max_set_)
        setMinMax();  // is already unit transformed

    assert(min_max_set_);

    logdbg << "DBOVariable: getMaxString: object " << dboName() << " name " << name()
           << " returning " << max_;
    return max_;
}

std::string DBOVariable::getMinStringRepresentation()
{
    if (representation_ == Representation::STANDARD)
        return getMinString();
    else
        return getRepresentationStringFromValue(getMinString());
}

std::string DBOVariable::getMaxStringRepresentation()
{
    if (representation_ == Representation::STANDARD)
        return getMaxString();
    else
        return getRepresentationStringFromValue(getMaxString());
}

DBOVariableWidget* DBOVariable::widget()
{
    if (!widget_)
    {
        widget_ = new DBOVariableWidget(*this);
        assert(widget_);
    }

    return widget_;
}

DBOVariable::Representation DBOVariable::representation() const { return representation_; }

const std::string& DBOVariable::representationString() const { return representation_str_; }

void DBOVariable::representation(const DBOVariable::Representation& representation)
{
    representation_str_ = representationToString(representation);
    representation_ = representation;
}

std::string DBOVariable::getRepresentationStringFromValue(const std::string& value_str) const
{
    logdbg << "DBOVariable: getRepresentationStringFromValue: value " << value_str << " data_type "
           << Property::asString(data_type_) << " representation "
           << representationToString(representation_);

    if (value_str == NULL_STRING)
        return value_str;

    if (representation_ == DBOVariable::Representation::STANDARD)
        return value_str;

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        {
            bool value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::CHAR:
        {
            char value = std::stoi(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::UCHAR:
        {
            unsigned char value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::INT:
        {
            int value = std::stoi(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::UINT:
        {
            unsigned int value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::LONGINT:
        {
            long value = std::stol(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::ULONGINT:
        {
            unsigned long value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::FLOAT:
        {
            float value = std::stof(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::DOUBLE:
        {
            double value = std::stod(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: getRepresentationStringFromValue: representation of string variable"
                " impossible");
        default:
            logerr << "DBOVariable: getRepresentationStringFromValue:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error(
                "DBOVariable: getRepresentationStringFromValue:: unknown property type " +
                Property::asString(data_type_));
    }
}

std::string DBOVariable::getValueStringFromRepresentation(
    const std::string& representation_str) const
{
    if (representation_str == NULL_STRING)
        return representation_str;

    assert(representation_ != DBOVariable::Representation::STANDARD);

    if (representation_ == DBOVariable::Representation::SECONDS_TO_TIME)
    {
        return String::getValueString(Utils::String::timeFromString(representation_str));
    }
    else if (representation_ == DBOVariable::Representation::DEC_TO_OCTAL)
    {
        return String::getValueString(Utils::String::intFromOctalString(representation_str));
    }
    else if (representation_ == DBOVariable::Representation::DEC_TO_HEX)
    {
        return String::getValueString(Utils::String::intFromHexString(representation_str));
    }
    else if (representation_ == DBOVariable::Representation::FEET_TO_FLIGHTLEVEL)
    {
        return String::getValueString(std::stod(representation_str) * 100.0);
    }
    else if (representation_ == DBOVariable::Representation::DATA_SRC_NAME)
    {
        assert(db_object_);
        if (db_object_->hasDataSources())
        {
            for (auto ds_it = db_object_->dsBegin(); ds_it != db_object_->dsEnd(); ++ds_it)
            {
                if ((ds_it->second.hasShortName() &&
                     representation_str == ds_it->second.shortName()) ||
                    representation_str == ds_it->second.name())
                {
                    return std::to_string(ds_it->first);
                }
            }
            // not found, return original
        }
        // has no datasources, return original

        return representation_str;
    }
    else
    {
        throw std::runtime_error(
            "Utils: String: getValueStringFromRepresentation: unknown representation " +
            std::to_string((int)representation_));
    }
}

std::string DBOVariable::multiplyString(const std::string& value_str, double factor) const
{
    logdbg << "DBOVariable: multiplyString: value " << value_str << " factor " << factor
           << " data_type " << Property::asString(data_type_);

    if (value_str == NULL_STRING)
        return value_str;

    std::string return_string;

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        {
            bool value = std::stoul(value_str);
            value = value && factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::CHAR:
        {
            char value = std::stoi(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::UCHAR:
        {
            unsigned char value = std::stoul(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::INT:
        {
            int value = std::stoi(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::UINT:
        {
            unsigned int value = std::stoul(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::LONGINT:
        {
            long value = std::stol(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::ULONGINT:
        {
            unsigned long value = std::stoul(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::FLOAT:
        {
            float value = std::stof(value_str);
            value *= factor;
            return_string = String::getValueString(value);
            break;
        }
        case PropertyDataType::DOUBLE:
        {
            double value = std::stod(value_str);
            value *= factor;
            return_string = String::getValueString(value);
            break;
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: multiplyString: multiplication of string variable impossible");
        default:
            logerr << "DBOVariable: multiplyString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("DBOVariable: multiplyString:: unknown property type " +
                                     Property::asString(data_type_));
    }

    logdbg << "DBOVariable: multiplyString: return value " << return_string;

    return return_string;
}

const std::string& DBOVariable::getLargerValueString(const std::string& value_a_str,
                                                     const std::string& value_b_str) const
{
    logdbg << "DBOVariable: getLargerValueString: value a " << value_a_str << " b " << value_b_str
           << " data_type " << Property::asString(data_type_);

    if (value_a_str == NULL_STRING || value_b_str == NULL_STRING)
    {
        if (value_a_str != NULL_STRING)
            return value_a_str;
        if (value_b_str != NULL_STRING)
            return value_b_str;
        return NULL_STRING;
    }

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        case PropertyDataType::UCHAR:
        case PropertyDataType::UINT:
        case PropertyDataType::ULONGINT:
        {
            if (std::stoul(value_a_str) > std::stoul(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::CHAR:
        case PropertyDataType::INT:
        {
            if (std::stoi(value_a_str) > std::stoi(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::LONGINT:
        {
            if (std::stol(value_a_str) > std::stol(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::FLOAT:
        {
            if (std::stof(value_a_str) > std::stof(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::DOUBLE:
        {
            if (std::stod(value_a_str) > std::stod(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: getLargerValueString: operation on string variable impossible");
        default:
            logerr << "DBOVariable: getLargerValueString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("DBOVariable: getLargerValueString:: unknown property type " +
                                     Property::asString(data_type_));
    }
}

const std::string& DBOVariable::getSmallerValueString(const std::string& value_a_str,
                                                      const std::string& value_b_str) const
{
    logdbg << "DBOVariable: getSmallerValueString: value a " << value_a_str << " b " << value_b_str
           << " data_type " << Property::asString(data_type_);

    if (value_a_str == NULL_STRING || value_b_str == NULL_STRING)
    {
        if (value_a_str != NULL_STRING)
            return value_a_str;
        if (value_b_str != NULL_STRING)
            return value_b_str;
        return NULL_STRING;
    }

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        case PropertyDataType::UCHAR:
        case PropertyDataType::UINT:
        case PropertyDataType::ULONGINT:
        {
            if (std::stoul(value_a_str) < std::stoul(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::CHAR:
        case PropertyDataType::INT:
        {
            if (std::stoi(value_a_str) < std::stoi(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::LONGINT:
        {
            if (std::stol(value_a_str) < std::stol(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::FLOAT:
        {
            if (std::stof(value_a_str) < std::stof(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::DOUBLE:
        {
            if (std::stod(value_a_str) < std::stod(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: getSmallerValueString: operation on string variable impossible");
        default:
            logerr << "DBOVariable: getSmallerValueString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("DBOVariable: getSmallerValueString:: unknown property type " +
                                     Property::asString(data_type_));
    }
}

bool DBOVariable::existsInDB() const
{
    assert(db_object_);

    if (!db_object_->hasCurrentMetaTable())
        return false;

    if (!hasCurrentDBColumn())
        return false;
    else
        return currentDBColumn().existsInDB();
}

std::string DBOVariable::getDataSourcesAsString(const std::string& value) const
{
    assert(db_object_);
    if (db_object_->hasDataSources())
    {
        for (auto ds_it = db_object_->dsBegin(); ds_it != db_object_->dsEnd(); ++ds_it)
        {
            if (std::to_string(ds_it->first) == value)
            {
                if (ds_it->second.hasShortName())
                    return ds_it->second.shortName();
                else
                    return ds_it->second.name();
            }
        }
        // not found, return original
    }

    // search for data sources in other dbos
    for (auto& dbo_it : COMPASS::instance().objectManager())
    {
        if (dbo_it.second->hasDataSources())
        {
            for (auto ds_it = dbo_it.second->dsBegin(); ds_it != dbo_it.second->dsEnd(); ++ds_it)
            {
                if (std::to_string(ds_it->first) == value)
                {
                    if (ds_it->second.hasShortName())
                        return ds_it->second.shortName();
                    else
                        return ds_it->second.name();
                }
            }
            // not found, return original
        }
    }

    // has no datasources, return original
    //loginf << "DBOVariable: getDataSourcesAsString: ds '" << value << "' not found";

    return value;
}

bool DBOVariable::onlyExistsInSchema(const std::string& schema_name)
{
    for (auto& def_it : schema_variables_)
    {
        if (def_it.first != schema_name)  // other found
            return false;
    }
    return true;  // no other found
}

void DBOVariable::removeInfoForSchema(const std::string& schema_name)
{
    loginf << "DBOVariable " << name() << ": removeVariableInfoForSchema: " << schema_name;

    if (schema_variables_.count(schema_name))
    {
        delete schema_variables_.at(schema_name);
        schema_variables_.erase(schema_name);
    }
}
