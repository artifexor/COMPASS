#include "source/configurationdatasource.h"
#include "source/dbdatasource.h"
#include "datasourcemanager.h"
#include "logger.h"
#include "util/number.h"

#include <algorithm>

using namespace std;
using namespace nlohmann;
using namespace Utils;

namespace dbContent
{


ConfigurationDataSource::ConfigurationDataSource(const std::string& class_id, const std::string& instance_id,
                                                 DataSourceManager& ds_manager)
    : Configurable(class_id, instance_id, &ds_manager)
{
    registerParameter("ds_type", &ds_type_, "");
    registerParameter("sac", &sac_, 0);
    registerParameter("sic", &sic_, 0);

    assert (ds_type_.size());

    if (find(DataSourceManager::data_source_types_.begin(),
             DataSourceManager::data_source_types_.end(), ds_type_)
        == DataSourceManager::data_source_types_.end())
    {
        logerr << "ConfigurationDataSource: sac/sic " << sac_ << sic_ << " ds_type '" << ds_type_
               << "' wrong";
    }

    assert (find(DataSourceManager::data_source_types_.begin(),
                 DataSourceManager::data_source_types_.end(), ds_type_)
            != DataSourceManager::data_source_types_.end());

    registerParameter("name", &name_, "");
    registerParameter("has_short_name", &has_short_name_, false);
    registerParameter("short_name", &short_name_, "");

    registerParameter("info", &info_, {});

    assert (name_.size());

    if (has_short_name_ && !short_name_.size())
        has_short_name_ = false;

    parseNetworkLineInfo();
}

ConfigurationDataSource::~ConfigurationDataSource()
{

}

json ConfigurationDataSource::getAsJSON()
{
    json j;

    j["ds_type"] = ds_type_;

    j["sac"] = sac_;
    j["sic"] = sic_;

    j["name"] = name_;

    if (has_short_name_)
        j["short_name"] = short_name_;

    if (!info_.is_null())
        j["info"] = info_;

    return j;
}

void ConfigurationDataSource::setFromJSON(const json& j)
{
    logdbg << "ConfigurationDataSource: setFromJSON: '" << j.dump(4) << "'";

    assert(j.contains("ds_type"));

    ds_type_ = j["ds_type"];

    assert(j.contains("sac"));
    assert(j.contains("sic"));
    sac_ = j["sac"];
    sic_ = j["sic"];

    assert(j.contains("name"));
    name_ = j["name"];

    if (j.contains("short_name"))
    {
        has_short_name_ = true;
        short_name_ = j["short_name"];
    }
    else
        has_short_name_ = false;

    if (j.contains("info"))
        info_ = j["info"];
    else
        info_ = nullptr;

    parseNetworkLineInfo();
}

DBDataSource* ConfigurationDataSource::getAsNewDBDS()
{
    DBDataSource* new_ds = new DBDataSource();
    new_ds->id(Number::dsIdFrom(sac_, sic_));
    new_ds->dsType(ds_type_);
    new_ds->sac(sac_);
    new_ds->sic(sic_);
    new_ds->name(name_);

    if (has_short_name_)
        new_ds->shortName(short_name_);

    if (!info_.is_null())
        new_ds->info(info_.dump());

    loginf << "ConfigurationDataSource: getAsNewDBDS: name " << new_ds->name()
            << " sac/sic " << new_ds->sac() << "/" << new_ds->sic();

    return new_ds;
}

}
