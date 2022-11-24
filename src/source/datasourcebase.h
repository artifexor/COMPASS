#ifndef DBCONTENT_DATASOURCEBASE_H
#define DBCONTENT_DATASOURCEBASE_H

#include <json.hpp>
#include "datasourcelineinfo.h"

#include <string>

namespace dbContent
{

class DataSourceBase
{
public:
    DataSourceBase();

    std::string dsType() const;
    void dsType(const std::string& ds_type);

    unsigned int sac() const;
    void sac(unsigned int sac);

    unsigned int sic() const;
    void sic(unsigned int sic);

    virtual unsigned int id() const; // from sac/sic

    std::string name() const;
    void name(const std::string &name);

    bool hasShortName() const;
    void removeShortName();
    void shortName(const std::string& short_name);
    const std::string& shortName() const;

    void info(const std::string& info);
    nlohmann::json& info(); // for direct use, var->value
    std::string infoStr();

    bool hasUpdateInterval() const;
    void removeUpdateInterval();
    void updateInterval (float value);
    float updateInterval () const;

    bool hasPosition() const;
    bool hasFullPosition() const;

    void latitude (double value);
    double latitude () const;

    void longitude (double value);
    double longitude () const;

    void altitude (double value);
    double altitude () const;

    bool hasRadarRanges() const;
    void addRadarRanges();
    std::map<std::string, double> radarRanges() const;
    void radarRange (const std::string& key, const double range);
    void removeRadarRange(const std::string& key);

    bool hasRadarAccuracies() const;
    void addRadarAccuracies();
    std::map<std::string, double> radarAccuracies() const;
    void radarAccuracy (const std::string& key, const double value);

    bool hasNetworkLines() const;
    void addNetworkLines();
    std::map<std::string, std::shared_ptr<DataSourceLineInfo>> networkLines() const;
    bool hasNetworkLine (const std::string& key) const;
    void createNetworkLine (const std::string& key);
    std::shared_ptr<DataSourceLineInfo> networkLine (const std::string& key); // creates if not exists

    void setFromJSONDeprecated (const nlohmann::json& j);
    void setFromJSON (const nlohmann::json& j);

protected:
    std::string ds_type_;

    unsigned int sac_{0};
    unsigned int sic_{0};

    std::string name_;

    bool has_short_name_{false};
    std::string short_name_;

    nlohmann::json info_;

    std::map<std::string, std::shared_ptr<DataSourceLineInfo>> line_info_;

    void parseNetworkLineInfo();
};

}

#endif // DBCONTENT_DATASOURCEBASE_H