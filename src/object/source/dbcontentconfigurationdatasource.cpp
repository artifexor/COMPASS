#include "dbcontentconfigurationdatasource.h"
#include "dbcontentdbdatasource.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "util/number.h"

#include <algorithm>

using namespace std;
using namespace nlohmann;
using namespace Utils;

namespace DBContent
{

ConfigurationDataSource::ConfigurationDataSource(const std::string& class_id, const std::string& instance_id,
                                                 DBObjectManager& dbo_manager)
    : Configurable(class_id, instance_id, &dbo_manager)
{
    registerParameter("db_content_type", &db_content_type_, "");
    registerParameter("sac", &sac_, 0);
    registerParameter("sic", &sic_, 0);

    assert (db_content_type_.size());

    if (find(DBObjectManager::db_content_types_.begin(),
             DBObjectManager::db_content_types_.end(), db_content_type_)
        == DBObjectManager::db_content_types_.end())
    {
        logerr << "ConfigurationDataSource: sac/sic " << sac_ << sic_ << " db_content_type '" << db_content_type_
               << "' unknown";
    }

    assert (find(DBObjectManager::db_content_types_.begin(),
                 DBObjectManager::db_content_types_.end(), db_content_type_)
            != DBObjectManager::db_content_types_.end());

    registerParameter("name", &name_, "");
    registerParameter("has_short_name", &has_short_name_, false);
    registerParameter("short_name", &short_name_, "");

    registerParameter("info", &info_, {});

    assert (name_.size());

    if (has_short_name_)
        assert (short_name_.size());

}

ConfigurationDataSource::~ConfigurationDataSource()
{

}

json ConfigurationDataSource::getAsJSON()
{
    json j;

    j["db_content_type"] = db_content_type_;

    j["sac"] = sac_;
    j["sic"] = sic_;

    j["name"] = name_;

    if (has_short_name_)
        j["short_name"] = short_name_;

    j["info"] = info_;


    return j;
}

void ConfigurationDataSource::setFromJSON(json& j)
{
    j["db_content_type"] = db_content_type_;

    j["sac"] = sac_;
    j["sic"] = sic_;

    j["name"] = name_;

    if (has_short_name_)
        j["short_name"] = short_name_;

    j["info"] = info_;

    db_content_type_ = j.at("db_content_type");

    sac_ = j.at("sac");
    sic_ = j.at("sic");

    name_ = j.at("name");

    has_short_name_ = j.contains("short_name");
    if (has_short_name_)
        short_name_ = j.at("short_name");

    info_ = j.at("info");
}

DBDataSource* ConfigurationDataSource::getAsNewDBDS()
{
    DBContent::DBDataSource* new_ds = new DBContent::DBDataSource();
    new_ds->id(Number::dsIdFrom(sac_, sic_));
    new_ds->sac(sac_);
    new_ds->sic(sic_);
    new_ds->name(name_);

    if (has_short_name_)
        new_ds->shortName(short_name_);

    new_ds->info(info_);

    return new_ds;
}


}
