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

    m_use_sensor_offset_correction = _use_sensor_offset_correction.get();
    m_ais.reset(new AIS(*mDriver));
    return true;
}
bool AISTask::startHook()
{
    if (!AISTaskBase::startHook()) {
        return false;
    }

    m_UTM_converter.setParameters(_utm_configuration.get());
    return true;
}
void AISTask::updateHook()
{
    AISTaskBase::updateHook();

    m_ais_stats.time = base::Time::now();
    m_ais_stats.discarded_sentences = m_ais->getDiscardedSentenceCount();
    _ais_stats.write(m_ais_stats);
}
bool AISTask::processSentence(marnav::nmea::sentence const& sentence)
{
    if (sentence.id() != nmea::sentence_id::VDM) {
        return false;
    }

    unique_ptr<marnav::ais::message> msg;
    try {
        msg = m_ais->processSentence(sentence);
        if (!msg) {
            return true;
        }
    }
    catch (MarnavParsingError const& e) {
        LOG_ERROR_S << "error reported by marnav while creating an AIS message: "
                    << e.what() << std::endl;
        m_ais_stats.invalid_messages++;
        return true;
    }

    m_ais_stats.received_messages++;
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
            m_vessels[vesselInformation.mmsi] = vesselInformation;
            _vessels_information.write(vesselInformation);
            _voyages_information.write(AIS::getVoyageInformation(*msg05));
            break;
        }
        default:
            m_ais_stats.ignored_messages++;
            break;
    }
    return true;
}
void AISTask::processPositionReport(ais_base::Position const& position, int mmsi)
{
    if (!m_use_sensor_offset_correction) {
        _positions.write(position);
        return;
    }

    auto vessel = getCorrespondingVesselInfo(mmsi);
    if (vessel.has_value()) {
        auto corrected_position = AIS::applyPositionCorrection(position,
            vessel->reference_position,
            m_UTM_converter);
        _positions.write(corrected_position);
    }
    else {
        _positions.write(position);
    }
}
std::optional<ais_base::VesselInformation> AISTask::getCorrespondingVesselInfo(int mmsi)
{
    if (m_vessels.find(mmsi) != m_vessels.end()) {
        return m_vessels.at(mmsi);
    }

    return {};
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
