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

#ifndef EVALUATIONTARGETDATA_H
#define EVALUATIONTARGETDATA_H

#include "evaluationtargetposition.h"
#include "evaluationtargetvelocity.h"
#include "projection/transformation.h"

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <string>

class Buffer;
class EvaluationData;
class EvaluationManager;
//class Transformation;

//class OGRSpatialReference;
//class OGRCoordinateTransformation;

class TstDataMapping // mapping to respective ref data
{
public:
    float tod_ {0}; // tod of test

    bool has_ref1_ {false};
    float tod_ref1_ {0};

    bool has_ref2_ {false};
    float tod_ref2_ {0};

    bool has_ref_pos_ {false};
    EvaluationTargetPosition pos_ref_;
    EvaluationTargetVelocity posbased_spd_ref_;
};

class DataMappingTimes // mapping to respective tst data
{
public:
    float tod_ {0}; // tod of test

    bool has_other1_ {false};
    float tod_other1_ {0};

    bool has_other2_ {false};
    float tod_other2_ {0};
};

class EvaluationTargetData
{
public:
    EvaluationTargetData();
    EvaluationTargetData(unsigned int utn, EvaluationData& eval_data, EvaluationManager& eval_man);
    virtual ~EvaluationTargetData();

    void addRefIndex (float tod, unsigned int index);
    void addTstIndex (float tod, unsigned int index);

    bool hasData() const;
    bool hasRefData () const;
    bool hasTstData () const;

    void finalize () const;

    const unsigned int utn_{0};

    unsigned int numUpdates () const;
    unsigned int numRefUpdates () const;
    unsigned int numTstUpdates () const;

    float timeBegin() const;
    std::string timeBeginStr() const;
    float timeEnd() const;
    std::string timeEndStr() const;
    float timeDuration() const;

    std::vector<std::string> callsigns() const;
    std::string callsignsStr() const;

    std::vector<unsigned int> targetAddresses() const;
    std::string targetAddressesStr() const;

    std::vector<unsigned int> modeACodes() const;
    std::string modeACodesStr() const;

    bool hasModeC() const;
    int modeCMin() const;
    std::string modeCMinStr() const;
    int modeCMax() const;
    std::string modeCMaxStr() const;

    bool isPrimaryOnly () const;

    bool use() const;
    void use(bool use);

    const std::multimap<float, unsigned int>& refData() const;
    const std::multimap<float, unsigned int>& tstData() const;

    // ref
    bool hasRefDataForTime (float tod, float d_max) const;
    std::pair<float, float> refTimesFor (float tod, float d_max) const; // lower/upper times, -1 if not existing
    std::pair<EvaluationTargetPosition, bool> interpolatedRefPosForTime (float tod, float d_max) const;
    // bool ok
    std::pair<EvaluationTargetVelocity, bool> interpolatedRefPosBasedSpdForTime (float tod, float d_max) const;

    bool hasRefPosForTime (float tod) const;
    EvaluationTargetPosition refPosForTime (float tod) const;
    EvaluationTargetVelocity refPosBasedSpdForTime (float tod) const;
    std::pair<bool, float> estimateRefAltitude (float tod, unsigned int index) const;
    // estimate ref baro alt at tod,index TODO should be replaced by real altitude reconstructor

    bool hasRefCallsignForTime (float tod) const;
    std::string refCallsignForTime (float tod) const;

    bool hasRefModeAForTime (float tod) const; // only if set, is v, not g
    unsigned int refModeAForTime (float tod) const;

    bool hasRefModeCForTime (float tod) const; // only if set, is v, not g
    int refModeCForTime (float tod) const;

    bool hasRefTAForTime (float tod) const;
    unsigned int refTAForTime (float tod) const;

    std::pair<bool,bool> refGroundBitForTime (float tod) const; // has gbs, gbs true
    std::pair<bool,bool> interpolatedRefGroundBitForTime (float tod, float d_max) const; // has gbs, gbs true

    // test
    bool hasTstPosForTime (float tod) const;
    EvaluationTargetPosition tstPosForTime (float tod) const;
    std::pair<bool, float> estimateTstAltitude (float tod, unsigned int index) const;

    bool hasTstCallsignForTime (float tod) const;
    std::string tstCallsignForTime (float tod) const;

    bool hasTstModeAForTime (float tod) const; // only if set, is v, not g
    unsigned int tstModeAForTime (float tod) const;

    bool hasTstModeCForTime (float tod) const; // only if set, is v, not g
    int tstModeCForTime (float tod) const;

    bool hasTstGroundBitForTime (float tod) const; // only if set
    bool tstGroundBitForTime (float tod) const; // true is on ground

    bool hasTstTAForTime (float tod) const;
    unsigned int tstTAForTime (float tod) const;

    std::pair<bool,bool> tstGroundBitForTimeInterpolated (float tod) const; // has gbs, gbs true

    bool hasTstTrackNumForTime (float tod) const;
    unsigned int tstTrackNumForTime (float tod) const;

    // speed, track angle
    bool hasTstMeasuredSpeedForTime (float tod) const;
    float tstMeasuredSpeedForTime (float tod) const; // m/s

    bool hasTstMeasuredTrackAngleForTime (float tod) const;
    float tstMeasuredTrackAngleForTime (float tod) const; // deg

    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    bool hasPos() const;

    bool hasADSBInfo() const;
    std::set<unsigned int> mopsVersions() const;
    std::string mopsVersionsStr() const;

    bool hasNucpNic() const;
    std::string nucpNicStr() const;
    bool hasNacp() const;
    std::string nacpStr() const;

protected:
    //static bool in_appimage_;

    EvaluationData* eval_data_ {nullptr};
    EvaluationManager* eval_man_ {nullptr};

    bool use_ {true};

    std::multimap<float, unsigned int> ref_data_; // tod -> index
    mutable std::vector<unsigned int> ref_indexes_;

    std::multimap<float, unsigned int> tst_data_; // tod -> index
    mutable std::vector<unsigned int> tst_indexes_;

    mutable std::vector<std::string> callsigns_;
    mutable std::vector<unsigned int> target_addresses_;
    mutable std::vector<unsigned int> mode_a_codes_;

    mutable bool has_mode_c_ {false};
    mutable int mode_c_min_ {0};
    mutable int mode_c_max_ {0};

    mutable bool has_pos_ {false};
    mutable double latitude_min_ {0};
    mutable double latitude_max_ {0};

    mutable double longitude_min_ {0};
    mutable double longitude_max_ {0};

    mutable bool has_adsb_info_ {false};
    mutable std::set<unsigned int> mops_versions_;
    mutable bool has_nucp_nic_ {false};
    mutable unsigned int min_nucp_nic_, max_nucp_nic_;
    mutable bool has_nacp {false};
    mutable unsigned int min_nacp_, max_nacp_;

    mutable std::map<float, TstDataMapping> test_data_mappings_;

//    std::unique_ptr<OGRSpatialReference> wgs84_;
//    mutable std::unique_ptr<OGRSpatialReference> local_;
    //mutable std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart_;
    //mutable std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo_;
    mutable Transformation trafo_;

    void updateCallsigns() const;
    void updateTargetAddresses() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;
    //void updateADSBInfo() const;

    void calculateTestDataMappings() const;
    TstDataMapping calculateTestDataMapping(float tod) const; // test tod
    void addRefPositiosToMapping (TstDataMapping& mapping) const;
    void addRefPositiosToMappingFast (TstDataMapping& mapping) const;

    DataMappingTimes findTstTimes(float tod_ref) const; // ref tod
};

#endif // EVALUATIONTARGETDATA_H
