﻿#include "dbcontent/dbcontent.h"
#include "asterixpostprocessjob.h"
#include "dbcontent/dbcontentmanager.h"
#include "datasourcemanager.h"
#include "buffer.h"
#include "compass.h"
#include "mainwindow.h"
#include "projectionmanager.h"
#include "projection.h"
#include "json.hpp"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"

#include "boost/date_time/posix_time/posix_time.hpp"

const float tod_24h = 24 * 60 * 60;

using namespace std;
using namespace nlohmann;
using namespace Utils;


bool ASTERIXPostprocessJob::current_date_set_ = false;
boost::posix_time::ptime ASTERIXPostprocessJob::current_date_;
boost::posix_time::ptime ASTERIXPostprocessJob::previous_date_;
bool ASTERIXPostprocessJob::did_recent_time_jump_ = false;
bool ASTERIXPostprocessJob::had_late_time_ = false;

ASTERIXPostprocessJob::ASTERIXPostprocessJob(map<string, shared_ptr<Buffer>> buffers,
                                             const boost::posix_time::ptime& date,
                                             bool override_tod_active, float override_tod_offset,
                                             bool do_timestamp_checks)
    : Job("ASTERIXPostprocessJob"),
      buffers_(move(buffers)),
      override_tod_active_(override_tod_active), override_tod_offset_(override_tod_offset),
      do_timestamp_checks_(do_timestamp_checks)
{
//    network_time_offset_ = COMPASS::instance().mainWindow().importASTERIXFromNetworkTimeOffset();

    if (!current_date_set_) // init if first time
    {
        current_date_ = date;
        previous_date_ = current_date_;

        current_date_set_ = true;
    }
}

ASTERIXPostprocessJob::~ASTERIXPostprocessJob() { logdbg << "ASTERIXPostprocessJob: dtor"; }

void ASTERIXPostprocessJob::run()
{
    logdbg << "ASTERIXPostprocessJob: run: num buffers " << buffers_.size();

    started_ = true;

    if (override_tod_active_)
        doTodOverride();

//    if (network_time_offset_)
//        doNetworkTimeOverride();

    if (do_timestamp_checks_) // out of sync issue during 24h replay
        doFutureTimestampsCheck();

    doTimeStampCalculation();
    doRadarPlotPositionCalculations();
    doGroundSpeedCalculations();

    //    boost::posix_time::time_duration time_diff = boost::posix_time::microsec_clock::local_time() - start_time;
    //    double ms = time_diff.total_milliseconds();
    //    loginf << "UGA Buffer sort took " << String::timeStringFromDouble(ms / 1000.0, true);

    done_ = true;
}

void ASTERIXPostprocessJob::doTodOverride()
{
    loginf << "ASTERIXPostprocessJob: doTodOverride: offset " << override_tod_offset_;

    assert (override_tod_active_);

    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).existsIn(buf_it.first));

        dbContent::Variable& tod_var = obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index))
            {
                float& tod_ref = tod_vec.getRef(index);

                tod_ref += override_tod_offset_;

                // check for out-of-bounds because of midnight-jump
                while (tod_ref < 0.0f)
                    tod_ref += tod_24h;
                while (tod_ref > tod_24h)
                    tod_ref -= tod_24h;

                assert(tod_ref >= 0.0f);
                assert(tod_ref <= tod_24h);
            }
        }
    }
}

//void ASTERIXPostprocessJob::doNetworkTimeOverride()
//{
//    assert (network_time_offset_);

//    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

//    unsigned int buffer_size;

//    for (auto& buf_it : buffers_)
//    {
//        buffer_size = buf_it.second->size();

//        assert (obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).existsIn(buf_it.first));

//        dbContent::Variable& tod_var = obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).getFor(buf_it.first);

//        Property tod_prop {tod_var.name(), tod_var.dataType()};

//        assert (buf_it.second->hasProperty(tod_prop));

//        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

//        for (unsigned int index=0; index < buffer_size; ++index)
//        {
//            if (!tod_vec.isNull(index))
//            {
//                float& tod_ref = tod_vec.getRef(index);

//                tod_ref += network_time_offset_;

//                // check for out-of-bounds because of midnight-jump
//                while (tod_ref < 0.0f)
//                    tod_ref += tod_24h;
//                while (tod_ref > tod_24h)
//                    tod_ref -= tod_24h;

//                assert(tod_ref >= 0.0f);
//                assert(tod_ref <= tod_24h);
//            }
//        }
//    }
//}

const double TMAX_FUTURE_OFFSET = 3*60.0;
const double T24H_OFFSET = 5*60.0;

void ASTERIXPostprocessJob::doFutureTimestampsCheck()
{
    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    using namespace boost::posix_time;

    auto p_time = microsec_clock::universal_time (); // UTC


    double current_time_utc = p_time.time_of_day().total_milliseconds() / 1000.0;
    double tod_utc_max = (p_time + Time::partialSeconds(TMAX_FUTURE_OFFSET)).time_of_day().total_milliseconds() / 1000.0;

    bool in_vicinity_of_24h_time = current_time_utc <= T24H_OFFSET || current_time_utc >= (tod_24h - T24H_OFFSET);

    loginf << "ASTERIXPostprocessJob: doFutureTimestampsCheck: maximum time is "
           << String::timeStringFromDouble(tod_utc_max) << " 24h vicinity " << in_vicinity_of_24h_time;

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).existsIn(buf_it.first));

        dbContent::Variable& tod_var = obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        std::tuple<bool,float,float> min_max_tod = tod_vec.minMaxValues();

        if (get<0>(min_max_tod))
            loginf << "ASTERIXPostprocessJob: doFutureTimestampsCheck: " << buf_it.first
                   << " min tod " << String::timeStringFromDouble(get<1>(min_max_tod))
                   << " max " << String::timeStringFromDouble(get<2>(min_max_tod));

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index))
            {
                if (in_vicinity_of_24h_time)
                {
                     // not at end of day and bigger than max
                    if (tod_vec.get(index) < (tod_24h - T24H_OFFSET) && tod_vec.get(index) > tod_utc_max)
                    {
                        logwrn << "ASTERIXPostprocessJob: doFutureTimestampsCheck: vic doing " << buf_it.first
                               << " cutoff tod index " << index
                               << " tod " << String::timeStringFromDouble(tod_vec.get(index));

                        buf_it.second->cutToSize(index);
                        break;
                    }
                }
                else
                {
                    if (tod_vec.get(index) > tod_utc_max)
                    {
                        logwrn << "ASTERIXPostprocessJob: doFutureTimestampsCheck: doing " << buf_it.first
                               << " cutoff tod index " << index
                               << " tod " << String::timeStringFromDouble(tod_vec.get(index));

                        buf_it.second->cutToSize(index);
                        break;
                    }
                }
            }
        }
    }

    // remove empty buffers

    std::map<std::string, std::shared_ptr<Buffer>> tmp_data = buffers_;

    for (auto& buf_it : tmp_data)
        if (!buf_it.second->size())
            buffers_.erase(buf_it.first);
}

void ASTERIXPostprocessJob::doTimeStampCalculation()
{
    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        // tod
        assert (obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).existsIn(buf_it.first));
        dbContent::Variable& tod_var = obj_man.metaVariable(DBContent::meta_var_time_of_day_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};
        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        // timestamp
        assert (obj_man.metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(buf_it.first));
        dbContent::Variable& timestamp_var = obj_man.metaVariable(DBContent::meta_var_timestamp_.name()).getFor(buf_it.first);

        Property timestamp_prop {timestamp_var.name(), timestamp_var.dataType()};
        assert (!buf_it.second->hasProperty(timestamp_prop));
        buf_it.second->addProperty(timestamp_prop);

        NullableVector<boost::posix_time::ptime>& timestamp_vec =
                buf_it.second->get<boost::posix_time::ptime>(timestamp_var.name());

        float tod;
        //boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
        boost::posix_time::ptime timestamp;
        bool in_vicinity_of_24h_time = false; // within 5min of 24hrs
        bool outside_vicinity_of_24h_time = false; // outside of 10min to 24hrs

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index))
            {
                tod = tod_vec.get(index);

                had_late_time_ |= (tod >= (tod_24h - 300.0)); // within last 5min before midnight

                in_vicinity_of_24h_time = tod <= 300.0 || tod >= (tod_24h - 300.0); // within 10 minutes of midnight

                if (tod > tod_24h/2) // late
                    outside_vicinity_of_24h_time = tod <= tod_24h - 600.0;
                else // early
                    outside_vicinity_of_24h_time = tod >= 600.0 ;

                if (in_vicinity_of_24h_time)
                    logdbg << "ASTERIXPostprocessJob: doTimeStampCalculation: tod " << String::timeStringFromDouble(tod)
                           << " in 24h vicinity, had_late_time_ " << had_late_time_
                           << " did_recent_time_jump " << did_recent_time_jump_;

                if (did_recent_time_jump_ && outside_vicinity_of_24h_time) // clear if set and 10min outside again
                {
                    loginf << "ASTERIXPostprocessJob: doTimeStampCalculation: clearing did_recent_time_jump_";
                    did_recent_time_jump_ = false;
                }

                if (in_vicinity_of_24h_time) // check if timejump and assign timestamp to correct day
                {
                    // check if timejump (if not yet done)
                    if (!did_recent_time_jump_ && had_late_time_ && tod <= 300.0) // not yet handled timejump
                    {
                        current_date_ += boost::posix_time::seconds((unsigned int) tod_24h);
                        previous_date_ = current_date_ - boost::posix_time::seconds((unsigned int) tod_24h);

                        loginf << "ASTERIXPostprocessJob: doTimeStampCalculation: detected time-jump from "
                               << " current " << Time::toDateString(current_date_)
                               << " previous " << Time::toDateString(previous_date_);

                        did_recent_time_jump_ = true;
                    }

                    if (tod <= 300.0) // early time of current day
                        timestamp = current_date_ + boost::posix_time::millisec((unsigned int) (tod * 1000));
                    else // late time of previous day
                        timestamp = previous_date_ + boost::posix_time::millisec((unsigned int) (tod * 1000));
                }
                else // normal timestamp
                {
                    timestamp = current_date_ + boost::posix_time::millisec((unsigned int) (tod * 1000));
                }

                if (in_vicinity_of_24h_time)
                    logdbg << "ASTERIXPostprocessJob: doTimeStampCalculation: tod " << String::timeStringFromDouble(tod)
                           << " timestamp " << Time::toString(timestamp);

                if (outside_vicinity_of_24h_time)
                {
                    did_recent_time_jump_ = false;
                    had_late_time_ = false;
                }

                logdbg << "ASTERIXPostprocessJob: doTimeStampCalculation: tod " << String::timeStringFromDouble(tod)
                       << " ts " << Time::toString(timestamp);

                timestamp_vec.set(index, timestamp);
            }
        }
    }
}

void ASTERIXPostprocessJob::doRadarPlotPositionCalculations()
{
    // radar calculations

    string dbcontent_name;

    string datasource_var_name;
    string range_var_name;
    string azimuth_var_name;
    string altitude_var_name;
    string latitude_var_name;
    string longitude_var_name;

    // do radar position projection

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();
    ProjectionManager& proj_man = ProjectionManager::instance();

    assert(proj_man.hasCurrentProjection());
    Projection& projection = proj_man.currentProjection();

    unsigned int ds_id;
    double azimuth_deg;
    double azimuth_rad;
    double range_nm;
    double range_m;
    double altitude_ft;
    bool has_altitude;
    double lat, lon;

    bool ret;

    unsigned int transformation_errors;

    for (auto& buf_it : buffers_)
    {
        dbcontent_name = buf_it.first;

        if (dbcontent_name != "CAT001" && dbcontent_name != "CAT010" && dbcontent_name != "CAT048")
            continue;

        shared_ptr<Buffer> buffer = buf_it.second;
        unsigned int buffer_size = buffer->size();
        assert(buffer_size);

        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));
        assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_range_));
        assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_azimuth_));
        assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_altitude_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

        dbContent::Variable& datasource_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_);
        dbContent::Variable& range_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_range_);
        dbContent::Variable& azimuth_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_azimuth_);
        dbContent::Variable& altitude_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_altitude_);
        dbContent::Variable& latitude_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_);
        dbContent::Variable& longitude_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_);

        datasource_var_name = datasource_var.name();
        range_var_name = range_var.name();
        azimuth_var_name = azimuth_var.name();
        altitude_var_name = altitude_var.name();
        latitude_var_name = latitude_var.name();
        longitude_var_name = longitude_var.name();

        assert (datasource_var.dataType() == PropertyDataType::UINT);
        assert (range_var.dataType() == PropertyDataType::DOUBLE);
        assert (azimuth_var.dataType() == PropertyDataType::DOUBLE);
        assert (altitude_var.dataType() == PropertyDataType::FLOAT);
        assert (latitude_var.dataType() == PropertyDataType::DOUBLE);
        assert (longitude_var.dataType() == PropertyDataType::DOUBLE);

        assert (buffer->has<unsigned int>(datasource_var_name));
        assert (buffer->has<double>(range_var_name));
        assert (buffer->has<double>(azimuth_var_name));
        assert (buffer->has<float>(altitude_var_name));

        if (!buffer->has<double>(latitude_var_name))
            buffer->addProperty(latitude_var_name, PropertyDataType::DOUBLE);

        if (!buffer->has<double>(longitude_var_name))
            buffer->addProperty(longitude_var_name, PropertyDataType::DOUBLE);

        transformation_errors = 0;

        NullableVector<unsigned int>& datasource_vec = buffer->get<unsigned int>(datasource_var_name);
        NullableVector<double>& range_vec = buffer->get<double>(range_var_name);
        NullableVector<double>& azimuth_vec = buffer->get<double>(azimuth_var_name);
        NullableVector<float>& altitude_vec = buffer->get<float>(altitude_var_name);
        NullableVector<double>& latitude_vec = buffer->get<double>(latitude_var_name);
        NullableVector<double>& longitude_vec = buffer->get<double>(longitude_var_name);


        // set up projections

        for (auto ds_id_it : datasource_vec.distinctValues())
        {
            if (!projection.hasCoordinateSystem(ds_id_it))
            {
                if (!ds_man.hasConfigDataSource(ds_id_it) && !ds_man.hasDBDataSource(ds_id_it))
                {
                    logwrn << "ASTERIXPostprocessJob: doRadarPlotPositionCalculations: data source id "
                           << ds_id_it << " not defined in config or db";
                    continue;
                }

                //                if (!dbo_man.hasDataSource(ds_id_it))
                //                {
                //                    if(dbo_man.canAddNewDataSourceFromConfig(ds_id_it))
                //                        dbo_man.addNewDataSource(ds_id_it);
                //                    else
                //                    {
                //                        logerr << "ASTERIXPostprocessJob: run: ds id " << ds_id_it << " unknown";
                //                        continue;
                //                    }
                //                }

                //                assert (dbo_man.hasDataSource(ds_id_it));

                if (ds_man.hasConfigDataSource(ds_id_it))
                {
                    dbContent::ConfigurationDataSource& data_source = ds_man.configDataSource(ds_id_it);

                    if (data_source.hasFullPosition())
                    {
                        loginf << "ASTERIXPostprocessJob: doRadarPlotPositionCalculations: adding proj ds " << ds_id_it
                               << " lat/long " << data_source.latitude()
                               << "," << data_source.longitude()
                               << " alt " << data_source.altitude();

                        projection.addCoordinateSystem(ds_id_it, data_source.latitude(),
                                                       data_source.longitude(),
                                                       data_source.altitude());
                    }
                    else
                        logerr << "ASTERIXPostprocessJob: doRadarPlotPositionCalculations: config ds "
                               << data_source.name()
                               << " defined but missing position info";
                }
                else if (ds_man.hasDBDataSource(ds_id_it))
                {
                    dbContent::DBDataSource& data_source = ds_man.dbDataSource(ds_id_it);

                    if (data_source.hasFullPosition())
                    {
                        loginf << "ASTERIXPostprocessJob: run: adding proj ds " << ds_id_it
                               << " lat/long " << data_source.latitude()
                               << "," << data_source.longitude()
                               << " alt " << data_source.altitude();

                        projection.addCoordinateSystem(ds_id_it, data_source.latitude(),
                                                       data_source.longitude(),
                                                       data_source.altitude());
                    }
                    else
                        logerr << "ASTERIXPostprocessJob: run: ds " << data_source.name()
                               << " defined but missing position info";
                }
            }

        }

        for (unsigned int cnt = 0; cnt < buffer_size; cnt++)
        {
            // load buffer data

            if (datasource_vec.isNull(cnt))
            {
                logerr << "ASTERIXPostprocessJob: run: data source null";
                continue;
            }
            ds_id = datasource_vec.get(cnt);

            if (azimuth_vec.isNull(cnt) || range_vec.isNull(cnt))
            {
                logdbg << "ASTERIXPostprocessJob: run: position null";
                continue;
            }

            if (!latitude_vec.isNull(cnt) && !longitude_vec.isNull(cnt))
            {
                logdbg << "ASTERIXPostprocessJob: run: position already set";
                continue;
            }

            azimuth_deg = azimuth_vec.get(cnt);
            range_nm = range_vec.get(cnt);

            //loginf << "azimuth_deg " << azimuth_deg << " range_nm " << range_nm;

            has_altitude = !altitude_vec.isNull(cnt);
            if (has_altitude)
                altitude_ft = altitude_vec.get(cnt);
            else
                altitude_ft = 0.0;  // has to assumed in projection later on

            azimuth_rad = azimuth_deg * DEG2RAD;

            range_m = 1852.0 * range_nm;

            if (!projection.hasCoordinateSystem(ds_id))
            {
                transformation_errors++;
                continue;
            }

            ret = projection.polarToWGS84(ds_id, azimuth_rad, range_m, has_altitude,
                                          altitude_ft, lat, lon);

            if (!ret)
            {
                transformation_errors++;
                continue;
            }

            latitude_vec.set(cnt, lat);
            longitude_vec.set(cnt, lon);
        }
    }

    // do first buffer sorting

    //    loginf << "ASTERIXPostprocessJob: run: sorting buffers";

    //    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    //    for (auto& buf_it : buffers_)
    //    {
    //        logdbg << "ASTERIXPostprocessJob: run: sorting buffer " << buf_it.first;

    //        assert (dbo_man.existsMetaVariable(DBContent::meta_var_tod_id_.name()));
    //        assert (dbo_man.metaVariable(DBContent::meta_var_tod_id_.name()).existsIn(buf_it.first));

    //        DBOVariable& tod_var = dbo_man.metaVariable(DBContent::meta_var_tod_id_.name()).getFor(buf_it.first);

    //        Property prop {tod_var.name(), tod_var.dataType()};

    //        logdbg << "ASTERIXPostprocessJob: run: sorting by variable " << prop.name() << " " << prop.dataTypeString();

    //        assert (buf_it.second->hasProperty(prop));

    //        buf_it.second->sortByProperty(prop);
    //    }

}

void ASTERIXPostprocessJob::doGroundSpeedCalculations()
{
    string dbcontent_name;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    string vx_var_name;
    string vy_var_name;
    string speed_var_name;
    string track_angle_var_name;

    double speed_ms, bearing_rad;

    for (auto& buf_it : buffers_)
    {
        dbcontent_name = buf_it.first;

        if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_vx_)
                || !dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_vy_))
            continue;

        shared_ptr<Buffer> buffer = buf_it.second;
        unsigned int buffer_size = buffer->size();
        assert(buffer_size);

        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

        dbContent::Variable& vx_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vx_);
        dbContent::Variable& vy_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vy_);
        dbContent::Variable& speed_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_);
        dbContent::Variable& track_angle_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_);

        vx_var_name = vx_var.name();
        vy_var_name = vy_var.name();
        speed_var_name = speed_var.name();
        track_angle_var_name = track_angle_var.name();

        assert (vx_var.dataType() == PropertyDataType::DOUBLE);
        assert (vy_var.dataType() == PropertyDataType::DOUBLE);
        assert (speed_var.dataType() == PropertyDataType::DOUBLE);
        assert (track_angle_var.dataType() == PropertyDataType::DOUBLE);

        if (!buffer->has<double>(vx_var_name) || !buffer->has<double>(vy_var_name))
            continue; // cant calculate

        if (buffer->has<double>(speed_var_name) && buffer->has<double>(track_angle_var_name))
            continue; // no need for calculation

        if (!buffer->has<double>(speed_var_name))
            buffer->addProperty(speed_var_name, PropertyDataType::DOUBLE); // add if needed

        if (!buffer->has<double>(track_angle_var_name))
            buffer->addProperty(track_angle_var_name, PropertyDataType::DOUBLE); // add if needed

        NullableVector<double>& vx_vec = buffer->get<double>(vx_var_name);
        NullableVector<double>& vy_vec = buffer->get<double>(vy_var_name);
        NullableVector<double>& speed_vec = buffer->get<double>(speed_var_name);
        NullableVector<double>& track_angle_vec = buffer->get<double>(track_angle_var_name);

        for (unsigned int index=0; index < buffer_size; index++)
        {
            if (vx_vec.isNull(index) || vy_vec.isNull(index))
                continue;

            speed_ms = sqrt(pow(vx_vec.get(index), 2)+pow(vy_vec.get(index), 2)) ; // for 1s
            bearing_rad = atan2(vx_vec.get(index), vy_vec.get(index));

            speed_vec.set(index, speed_ms * M_S2KNOTS);
            track_angle_vec.set(index, bearing_rad * RAD2DEG);
        }
    }
}
