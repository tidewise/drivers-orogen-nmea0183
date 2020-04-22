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
    if (! AISTaskBase::startHook()) {
        return false;
    }
    return true;
}
void AISTask::updateHook()
{
    AISTaskBase::updateHook();

    mAISStats.time = base::Time::now();
    mAISStats.discarded_sentences = mAIS->getDiscardedSentenceCount();
    _ais_stats.write(mAISStats);
}
bool AISTask::processSentence(marnav::nmea::sentence const& sentence) {
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
                    << e.what()
                    << std::endl;
        mAISStats.invalid_messages++;
        return true;
    }

    mAISStats.received_messages++;
    switch (msg->type()) {
        case ais::message_id::position_report_class_a: {
            auto msg01 = ais::message_cast<ais::message_01>(msg);
            _positions.write(AIS::getPosition(*msg01));
            break;
        }
        case ais::message_id::position_report_class_a_assigned_schedule: {
            auto msg02 = ais::message_cast<ais::message_02>(msg);
            _positions.write(AIS::getPosition(*msg02));
            break;
        }
        case ais::message_id::position_report_class_a_response_to_interrogation: {
            auto msg03 = ais::message_cast<ais::message_03>(msg);
            _positions.write(AIS::getPosition(*msg03));
            break;
        }
        case ais::message_id::static_and_voyage_related_data: {
            auto msg05 = ais::message_cast<ais::message_05>(msg);
            _vessels_information.write(AIS::getVesselInformation(*msg05));
            _voyages_information.write(AIS::getVoyageInformation(*msg05));
            break;
        }
        default:
            mAISStats.ignored_messages++;
            break;
    }
    return true;
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
