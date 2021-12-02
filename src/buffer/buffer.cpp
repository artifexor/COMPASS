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

#include "buffer.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "logger.h"
#include "nullablevector.h"
#include "string.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"

using namespace nlohmann;
using namespace std;

unsigned int Buffer::ids_ = 0;

/**
 * Creates an empty buffer withput an DBO type
 *
 */
Buffer::Buffer()
{
    logdbg << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    logdbg << "Buffer: constructor: end";
}

/**
 * Creates a buffer from a PropertyList and a DBO type. Sets member to initial values.
 *
 * \param member_list PropertyList defining all properties
 * \param type DBO type
 */
Buffer::Buffer(PropertyList properties, const string& dbo_name)
    : dbo_name_(dbo_name), last_one_(false)
{
    logdbg << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    for (unsigned int cnt = 0; cnt < properties.size(); cnt++)
        addProperty(properties.at(cnt));

    logdbg << "Buffer: constructor: end";
}

/**
 * Calls clear.
 */
Buffer::~Buffer()
{
    logdbg << "Buffer: destructor: dbo " << dbo_name_ << " id " << id_;

    properties_.clear();

    getArrayListMap<bool>().clear();
    getArrayListMap<char>().clear();
    getArrayListMap<unsigned char>().clear();
    getArrayListMap<int>().clear();
    getArrayListMap<unsigned int>().clear();
    getArrayListMap<long int>().clear();
    getArrayListMap<unsigned long int>().clear();
    getArrayListMap<float>().clear();
    getArrayListMap<double>().clear();
    getArrayListMap<string>().clear();

    data_size_ = 0;

    logdbg << "Buffer: destructor: end";
}

/**
 * \param id Unique property identifier
 * \param type Property data type
 *
 * \exception runtime_error if property id already in use
 */
void Buffer::addProperty(string id, PropertyDataType type)
{
    logdbg << "Buffer: addProperty:  id '" << id << "' type " << Property::asString(type);

    assert(!id.empty());

    if (properties_.hasProperty(id))
        throw runtime_error("Buffer: addProperty: property " + id + " already exists");

    Property property = Property(id, type);

    switch (type)
    {
        case PropertyDataType::BOOL:
            assert(getArrayListMap<bool>().count(id) == 0);
            getArrayListMap<bool>()[id] =
                shared_ptr<NullableVector<bool>>(new NullableVector<bool>(property, *this));
            break;
        case PropertyDataType::CHAR:
            assert(getArrayListMap<char>().count(id) == 0);
            getArrayListMap<char>()[id] =
                shared_ptr<NullableVector<char>>(new NullableVector<char>(property, *this));
            break;
        case PropertyDataType::UCHAR:
            assert(getArrayListMap<unsigned char>().count(id) == 0);
            getArrayListMap<unsigned char>()[id] = shared_ptr<NullableVector<unsigned char>>(
                new NullableVector<unsigned char>(property, *this));
            break;
        case PropertyDataType::INT:
            assert(getArrayListMap<int>().count(id) == 0);
            getArrayListMap<int>()[id] =
                shared_ptr<NullableVector<int>>(new NullableVector<int>(property, *this));
            break;
        case PropertyDataType::UINT:
            assert(getArrayListMap<unsigned int>().count(id) == 0);
            getArrayListMap<unsigned int>()[id] = shared_ptr<NullableVector<unsigned int>>(
                new NullableVector<unsigned int>(property, *this));
            break;
        case PropertyDataType::LONGINT:
            assert(getArrayListMap<long int>().count(id) == 0);
            getArrayListMap<long int>()[id] =
                shared_ptr<NullableVector<long>>(new NullableVector<long>(property, *this));
            break;
        case PropertyDataType::ULONGINT:
            assert(getArrayListMap<unsigned long int>().count(id) == 0);
            getArrayListMap<unsigned long int>()[id] =
                shared_ptr<NullableVector<unsigned long>>(
                    new NullableVector<unsigned long>(property, *this));
            break;
        case PropertyDataType::FLOAT:
            assert(getArrayListMap<float>().count(id) == 0);
            getArrayListMap<float>()[id] =
                shared_ptr<NullableVector<float>>(new NullableVector<float>(property, *this));
            break;
        case PropertyDataType::DOUBLE:
            assert(getArrayListMap<double>().count(id) == 0);
            getArrayListMap<double>()[id] = shared_ptr<NullableVector<double>>(
                new NullableVector<double>(property, *this));
            break;
        case PropertyDataType::STRING:
            assert(getArrayListMap<string>().count(id) == 0);
            getArrayListMap<string>()[id] = shared_ptr<NullableVector<string>>(
                new NullableVector<string>(property, *this));
            break;
        default:
            logerr << "Buffer: addProperty: unknown property type " << Property::asString(type);
            throw runtime_error("Buffer: addProperty: unknown property type " +
                                     Property::asString(type));
    }

    properties_.addProperty(id, type);

    logdbg << "Buffer: addProperty: end";
}

void Buffer::addProperty(const Property& property)
{
    addProperty(property.name(), property.dataType());
}

void Buffer::seizeBuffer(Buffer& org_buffer)
{
    logdbg << "Buffer: seizeBuffer: start";

    logdbg << "Buffer: seizeBuffer: size " << size() << " other size " << org_buffer.size();

    org_buffer.properties_.clear();

    seizeArrayListMap<bool>(org_buffer);
    seizeArrayListMap<char>(org_buffer);
    seizeArrayListMap<unsigned char>(org_buffer);
    seizeArrayListMap<int>(org_buffer);
    seizeArrayListMap<unsigned int>(org_buffer);
    seizeArrayListMap<long int>(org_buffer);
    seizeArrayListMap<unsigned long int>(org_buffer);
    seizeArrayListMap<float>(org_buffer);
    seizeArrayListMap<double>(org_buffer);
    seizeArrayListMap<string>(org_buffer);

    data_size_ += org_buffer.data_size_;

    if (org_buffer.lastOne())
        last_one_ = true;

    logdbg << "Buffer: seizeBuffer: end size " << size();
}

size_t Buffer::size() { return data_size_; }

void Buffer::cutToSize(size_t size)
{
    for (auto& it : getArrayListMap<bool>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<char>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<unsigned char>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<unsigned int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<long int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<unsigned long int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<float>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<double>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<string>())
        it.second->cutToSize(size);

    data_size_ = size;
}

const PropertyList& Buffer::properties() { return properties_; }

bool Buffer::firstWrite() { return data_size_ == 0; }

bool Buffer::isNone(const Property& property, unsigned int row_cnt)
{
    if (BUFFER_PEDANTIC_CHECKING)
        assert(row_cnt < data_size_);

    switch (property.dataType())
    {
        case PropertyDataType::BOOL:
            assert(getArrayListMap<bool>().count(property.name()));
            return getArrayListMap<bool>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::CHAR:
            assert(getArrayListMap<char>().count(property.name()));
            return getArrayListMap<char>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::UCHAR:
            assert(getArrayListMap<unsigned char>().count(property.name()));
            return getArrayListMap<unsigned char>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::INT:
            assert(getArrayListMap<int>().count(property.name()));
            return getArrayListMap<int>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::UINT:
            assert(getArrayListMap<unsigned int>().count(property.name()));
            return getArrayListMap<unsigned int>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::LONGINT:
            assert(getArrayListMap<long int>().count(property.name()));
            return getArrayListMap<long int>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::ULONGINT:
            assert(getArrayListMap<unsigned long int>().count(property.name()));
            return getArrayListMap<unsigned long int>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::FLOAT:
            assert(getArrayListMap<float>().count(property.name()));
            return getArrayListMap<float>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::DOUBLE:
            assert(getArrayListMap<double>().count(property.name()));
            return getArrayListMap<double>().at(property.name())->isNull(row_cnt);
        case PropertyDataType::STRING:
            assert(getArrayListMap<string>().count(property.name()));
            return getArrayListMap<string>().at(property.name())->isNull(row_cnt);
        default:
            logerr << "Buffer: isNone: unknown property type "
                   << Property::asString(property.dataType());
            throw runtime_error("Buffer: isNone: unknown property type " +
                                     Property::asString(property.dataType()));
    }
}

void Buffer::transformVariables(DBOVariableSet& list, bool dbcol2dbovar)
{
    logdbg << "Buffer: transformVariables: dbo '" << dbo_name_ << "' dbcol2dbovar " << dbcol2dbovar;

    vector<DBOVariable*>& variables = list.getSet();
    string variable_name;
    string db_column_name;

    string current_var_name;
    string transformed_var_name;

    for (auto var_it : variables)
    {
        logdbg << "Buffer: transformVariables: variable " << var_it->name() << " db column " << db_column_name;

        variable_name = var_it->name();
        db_column_name = var_it->dbColumnName();

        PropertyDataType data_type = var_it->dataType();

        if (dbcol2dbovar)
        {
            if (!properties_.hasProperty(db_column_name))
            {
                logerr << "Buffer: transformVariables: property '" << db_column_name << "' not found";
                continue;
            }

            assert(properties_.hasProperty(db_column_name));
            assert(properties_.get(db_column_name).dataType() == var_it->dataType());

            current_var_name = db_column_name;
            transformed_var_name = variable_name;
        }
        else
        {
            if (!properties_.hasProperty(var_it->name()))
            {
                logerr << "Buffer: transformVariables: variable '" << variable_name << "' not found";
                continue;
            }

            assert(properties_.hasProperty(variable_name));
            assert(properties_.get(variable_name).dataType() == var_it->dataType());

            current_var_name = variable_name;
            transformed_var_name = db_column_name;
        }

        // rename to reflect dbo variable
        if (current_var_name != transformed_var_name)
        {
            logdbg << "Buffer: transformVariables: renaming variable " << current_var_name
                   << " to variable name " << transformed_var_name;

            switch (data_type)
            {
                case PropertyDataType::BOOL:
                {
                    rename<bool>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::CHAR:
                {
                    rename<char>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::UCHAR:
                {
                    rename<unsigned char>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::INT:
                {
                    rename<int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::UINT:
                {
                    rename<unsigned int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::LONGINT:
                {
                    rename<long int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::ULONGINT:
                {
                    rename<unsigned long int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::FLOAT:
                {
                    rename<float>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::DOUBLE:
                {
                    rename<double>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::STRING:
                {
                    rename<string>(current_var_name, transformed_var_name);
                    break;
                }
                default:
                    logerr << "Buffer: transformVariables: unknown property type "
                           << Property::asString(data_type);
                    throw runtime_error("Buffer: transformVariables: unknown property type " +
                                             Property::asString(data_type));
            }
        }
    }
}

shared_ptr<Buffer> Buffer::getPartialCopy(const PropertyList& partial_properties)
{
    assert (size());
    shared_ptr<Buffer> tmp_buffer{new Buffer()};

    for (unsigned int cnt = 0; cnt < partial_properties.size(); ++cnt)
    {
        Property prop = partial_properties.at(cnt);

        logdbg << "Buffer: getPartialCopy: adding property " << prop.name();
        tmp_buffer->addProperty(prop);

        switch (prop.dataType())
        {
            case PropertyDataType::BOOL:
                logdbg << "Buffer: getPartialCopy: adding BOOL property " << prop.name()
                       << " size " << get<bool>(prop.name()).size();
                tmp_buffer->get<bool>(prop.name()).copyData(get<bool>(prop.name()));
                break;
            case PropertyDataType::CHAR:
                logdbg << "Buffer: getPartialCopy: adding CHAR property " << prop.name()
                          << " size " << get<char>(prop.name()).size();
                tmp_buffer->get<char>(prop.name()).copyData(get<char>(prop.name()));
                break;
            case PropertyDataType::UCHAR:
                logdbg << "Buffer: getPartialCopy: adding UCHAR property " << prop.name()
                          << " size " << get<unsigned char>(prop.name()).size();
                tmp_buffer->get<unsigned char>(prop.name())
                    .copyData(get<unsigned char>(prop.name()));
                break;
            case PropertyDataType::INT:
                logdbg << "Buffer: getPartialCopy: adding INT property " << prop.name()
                          << " size " << get<int>(prop.name()).size();
                tmp_buffer->get<int>(prop.name()).copyData(get<int>(prop.name()));
                break;
            case PropertyDataType::UINT:
                logdbg << "Buffer: getPartialCopy: adding UINT property " << prop.name()
                          << " size " << get<unsigned int>(prop.name()).size();
                tmp_buffer->get<unsigned int>(prop.name()).copyData(get<unsigned int>(prop.name()));
                break;
            case PropertyDataType::LONGINT:
                logdbg << "Buffer: getPartialCopy: adding LONGINT property " << prop.name()
                          << " size " << get<long int>(prop.name()).size();
                tmp_buffer->get<long int>(prop.name()).copyData(get<long int>(prop.name()));
                break;
            case PropertyDataType::ULONGINT:
                logdbg << "Buffer: getPartialCopy: adding ULONGINT property " << prop.name()
                          << " size " << get<unsigned long int>(prop.name()).size();
                tmp_buffer->get<unsigned long int>(prop.name())
                    .copyData(get<unsigned long int>(prop.name()));
                break;
            case PropertyDataType::FLOAT:
                logdbg << "Buffer: getPartialCopy: adding FLOAT property " << prop.name()
                          << " size " << get<float>(prop.name()).size();
                tmp_buffer->get<float>(prop.name()).copyData(get<float>(prop.name()));
                break;
            case PropertyDataType::DOUBLE:
                logdbg << "Buffer: getPartialCopy: adding bool property " << prop.name()
                          << " size " << get<double>(prop.name()).size();
                tmp_buffer->get<double>(prop.name()).copyData(get<double>(prop.name()));
                break;
            case PropertyDataType::STRING:
                logdbg << "Buffer: getPartialCopy: adding STRING property " << prop.name()
                          << " size " << get<string>(prop.name()).size();
                tmp_buffer->get<string>(prop.name()).copyData(get<string>(prop.name()));
                break;
            default:
                logerr << "Buffer: getPartialCopy: unknown property type "
                       << Property::asString(prop.dataType());
                throw runtime_error("Buffer: getPartialCopy: unknown property type " +
                                         Property::asString(prop.dataType()));
        }
    }

    assert (tmp_buffer->size());

    return tmp_buffer;
}

nlohmann::json Buffer::asJSON()
{
    json j;

    for (unsigned int cnt=0; cnt < data_size_; ++cnt)
    {
        j[cnt] = json::object();

        for (auto& it : getArrayListMap<bool>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<char>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<unsigned char>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<unsigned int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<long int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<unsigned long int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<float>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<double>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<string>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
    }

    return j;
}
