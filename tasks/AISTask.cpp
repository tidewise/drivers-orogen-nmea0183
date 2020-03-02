/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AISTask.hpp"

#include <iodrivers_base/ConfigureGuard.hpp>
#include <marnav/ais/message.hpp>
#include <marnav/ais/message_02.hpp>
#include <marnav/ais/message_03.hpp>
#include <nmea0183/Driver.hpp>
#include <nmea0183/AIS.hpp>

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
    unique_ptr<Driver> driver(new Driver());
    iodrivers_base::ConfigureGuard guard(this);
    if (!_io_port.get().empty()) {
        driver->openURI(_io_port.get());
    }
    setDriver(driver.get());

    if (!AISTaskBase::configureHook()) {
        return false;
    }

    m_driver = move(driver);
    m_ais.reset(new AIS(*m_driver));
    guard.commit();
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
}
void AISTask::processIO() {
    std::unique_ptr<marnav::ais::message> msg;
    try {
        msg = m_ais->readMessage();
    }
    catch(...) {
        return;
    }

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
            _ignored_messages.write(static_cast<int>(msg->type()));
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
    m_driver.release();
}
