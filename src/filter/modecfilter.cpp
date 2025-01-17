#include "modecfilter.h"
#include "compass.h"
#include "modecfilterwidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

ModeCFilter::ModeCFilter(const std::string& class_id, const std::string& instance_id,
                       Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("min_value", &min_value_, -1000.0);
    registerParameter("max_value", &max_value_, 10000.0);
    registerParameter("null_wanted", &null_wanted_, false);

    name_ = "Mode C Codes";

    createSubConfigurables();
}

ModeCFilter::~ModeCFilter() {}

bool ModeCFilter::filters(const std::string& dbo_type)
{
    return COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_mc_.name()).existsIn(dbo_type);
}

std::string ModeCFilter::getConditionString(const std::string& dbcontent_name, bool& first)
{
    logdbg << "ModeCFilter: getConditionString: dbo " << dbcontent_name << " active " << active_;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_mc_.name()).existsIn(dbcontent_name))
        return "";

    stringstream ss;

    if (active_)
    {
        dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_mc_.name()).getFor(dbcontent_name);

        if (!first)
        {
            ss << " AND";
        }

        if (null_wanted_)
        {
            ss << " (" << var.dbColumnName() << " IS NULL OR";
            ss << " (" << var.dbColumnName() << " >= " << min_value_
               << " AND " << var.dbColumnName() << " <= " << max_value_ << "))";

        }
        else
        {
            ss << " (" << var.dbColumnName() << " >= " << min_value_
               << " AND " << var.dbColumnName() << " <= " << max_value_ << ")";
        }

        first = false;
    }

    logdbg << "ModeCFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void ModeCFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "ModeCFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("ModeCFilter: generateSubConfigurable: unknown class_id " + class_id);
}

DBFilterWidget* ModeCFilter::createWidget()
{
    return new ModeCFilterWidget(*this);
}


void ModeCFilter::checkSubConfigurables()
{
    logdbg << "ModeCFilter: checkSubConfigurables";
}


void ModeCFilter::reset()
{
    widget_->update();
}

void ModeCFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["Barometric Altitude Minimum"] = min_value_;
    filter["Barometric Altitude Maximum"] = max_value_;
    filter["Barometric Altitude NULL"] = null_wanted_;
}

void ModeCFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("Barometric Altitude Minimum"));
    min_value_ = filter.at("Barometric Altitude Minimum");

    assert (filter.contains("Barometric Altitude Maximum"));
    max_value_ = filter.at("Barometric Altitude Maximum");

    if (filter.contains("Barometric Altitude NULL"))
        null_wanted_ = filter.at("Barometric Altitude NULL");
    else
        null_wanted_ = false;


    if (widget())
        widget()->update();
}

bool ModeCFilter::activeInLiveMode()
{
    return true;
}

std::vector<size_t> ModeCFilter::filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer)
{
    std::vector<size_t> to_be_removed;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_mc_.name()).existsIn(dbcontent_name))
        return to_be_removed;

    dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_mc_.name()).getFor(dbcontent_name);

    assert (buffer->has<float> (var.name()));

    NullableVector<float>& data_vec = buffer->get<float> (var.name());

    float value;

    for (unsigned int cnt=0; cnt < buffer->size(); ++cnt)
    {
        if (data_vec.isNull(cnt))
        {
            if (!null_wanted_)
                to_be_removed.push_back(cnt);

            continue;
        }

        value = data_vec.get(cnt);

        if (value < min_value_ || value > max_value_)
            to_be_removed.push_back(cnt);
    }

    return to_be_removed;
}

float ModeCFilter::minValue() const
{
    return min_value_;
}

void ModeCFilter::minValue(float min_value)
{
    min_value_ = min_value;
}

float ModeCFilter::maxValue() const
{
    return max_value_;
}

void ModeCFilter::maxValue(float max_value)
{
    max_value_ = max_value;
}

bool ModeCFilter::nullWanted() const
{
    return null_wanted_;
}

void ModeCFilter::nullWanted(bool null_wanted)
{
    null_wanted_ = null_wanted;
}

