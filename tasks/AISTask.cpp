/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AISTask.hpp"

#include <base-logging/Logging.hpp>

#include <iodrivers_base/ConfigureGuard.hpp>
#include <marnav/ais/message.hpp>
#include <marnav/ais/message_02.hpp>
#include <marnav/ais/message_03.hpp>
#include <nmea0183/AIS.hpp>
#include <nmea0183/Driver.hpp>
#include <nmea0183/Exceptions.hpp>

using namespace std;

using namespace marnav;
using namespace nmea0183;

AISTask::AISTask(std::string const& name)
    : AISTaskBase(name)
{
}

AISTask::~AISTask()
{
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See AISTask.hpp for more detailed
// documentation about them.

bool AISTask::configureHook()
{
    if (!AISTaskBase::configureHook()) {
        return false;
    }

    mAIS.reset(new AIS(*mDriver));
    return true;
}
bool AISTask::startHook()
{
    if (!AISTaskBase::startHook()) {
        return false;
    }

    mUTMConverter.setParameters(_utm_configuration.get());
    return true;
}
void AISTask::updateHook()
{
    AISTaskBase::updateHook();

    mAISStats.time = base::Time::now();
    mAISStats.discarded_sentences = mAIS->getDiscardedSentenceCount();
    _ais_stats.write(mAISStats);
}
base::Vector3d sensorDataToWorld(base::Vector3d sensor2vessel_pos,
    Eigen::Quaterniond vessel2world_ori)
{
    base::Vector3d sensor2world_pos;
    sensor2world_pos = vessel2world_ori * sensor2vessel_pos;

    return sensor2world_pos;
}
base::samples::RigidBodyState AISTask::convertGPSToUTM(ais_base::Position position)
{
    gps_base::Solution sensor2world_solution;
    sensor2world_solution.latitude = position.latitude.getDeg();
    sensor2world_solution.longitude = position.longitude.getDeg();

    base::samples::RigidBodyState sensor2world_in_UTM;
    sensor2world_in_UTM.position =
        mUTMConverter.convertToUTM(sensor2world_solution).position;

    return sensor2world_in_UTM;
}
ais_base::Position AISTask::convertUTMToGPSInWorldFrame(
    base::samples::RigidBodyState sensor2world_in_UTM,
    base::Vector3d sensor2world_pos)
{
    base::samples::RigidBodyState vessel2world_in_UTM;
    vessel2world_in_UTM.position = sensor2world_in_UTM.position + sensor2world_pos;

    gps_base::Solution vessel2world_in_GPS;
    vessel2world_in_GPS = mUTMConverter.convertUTMToGPS(vessel2world_in_UTM);

    ais_base::Position vessel2world_pos;
    vessel2world_pos.latitude = base::Angle::fromDeg(vessel2world_in_GPS.latitude);
    vessel2world_pos.longitude = base::Angle::fromDeg(vessel2world_in_GPS.longitude);

    return vessel2world_pos;
}
bool AISTask::processSentence(marnav::nmea::sentence const& sentence)
{
    if (sentence.id() != nmea::sentence_id::VDM) {
        return false;
    }

    unique_ptr<marnav::ais::message> msg;
    try {
        msg = mAIS->processSentence(sentence);
        if (!msg) {
            return true;
        }
    }
    catch (MarnavParsingError const& e) {
        LOG_ERROR_S << "error reported by marnav while creating an AIS message: "
                    << e.what() << std::endl;
        mAISStats.invalid_messages++;
        return true;
    }

    mAISStats.received_messages++;
    switch (msg->type()) {
        case ais::message_id::position_report_class_a: {
            auto msg01 = ais::message_cast<ais::message_01>(msg);
            auto position = AIS::getPosition(*msg01);
            processPositionReport(position, position.mmsi);
            break;
        }
        case ais::message_id::position_report_class_a_assigned_schedule: {
            auto msg02 = ais::message_cast<ais::message_02>(msg);
            auto position = AIS::getPosition(*msg02);
            processPositionReport(position, position.mmsi);
            break;
        }
        case ais::message_id::position_report_class_a_response_to_interrogation: {
            auto msg03 = ais::message_cast<ais::message_03>(msg);
            auto position = AIS::getPosition(*msg03);
            processPositionReport(position, position.mmsi);
            break;
        }
        case ais::message_id::static_and_voyage_related_data: {
            auto msg05 = ais::message_cast<ais::message_05>(msg);
            auto vesselInformation = AIS::getVesselInformation(*msg05);
            auto voyageInformation = AIS::getVoyageInformation(*msg05);
            addToMap(vesselInformation);
            _vessels_information.write(vesselInformation);
            _voyages_information.write(voyageInformation);
            break;
        }
        default:
            mAISStats.ignored_messages++;
            break;
    }
    return true;
}
void AISTask::processPositionReport(ais_base::Position& position, int mmsi)
{
    auto vessel = getCorrespondingVesselInfo(mmsi);
    if (vessel.has_value()) {
        Eigen::Quaterniond vessel2world_ori(
            Eigen::AngleAxisd(position.course_over_ground.getRad(),
                Eigen::Vector3d::UnitZ()));
        auto sensor2world_pos =
            sensorDataToWorld(vessel->reference_position, vessel2world_ori);
        auto pos =
            convertUTMToGPSInWorldFrame(convertGPSToUTM(position), sensor2world_pos);
        position.latitude = pos.latitude;
        position.longitude = pos.longitude;
    }
    _positions.write(position);
}
std::optional<ais_base::VesselInformation> AISTask::getCorrespondingVesselInfo(int mmsi)
{
    if (mVessels.find(mmsi) != mVessels.end()) {
        return mVessels.at(mmsi);
    }

    return std::nullopt;
}
void AISTask::addToMap(ais_base::VesselInformation info)
{
    if (mVessels.find(info.mmsi) == mVessels.end()) {
        mVessels[info.mmsi] = info;
    }
}
void AISTask::errorHook()
{
    AISTaskBase::errorHook();
}
void AISTask::stopHook()
{
    AISTaskBase::stopHook();
}
void AISTask::cleanupHook()
{
    AISTaskBase::cleanupHook();
    mDriver.release();
}
