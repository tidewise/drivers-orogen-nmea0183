/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "MarnavTask.hpp"
#include <base-logging/Logging.hpp>
#include <iodrivers_base/ConfigureGuard.hpp>
#include <nmea0183/Driver.hpp>
#include <nmea0183/Exceptions.hpp>

using namespace std;

using namespace marnav;
using namespace nmea0183;

MarnavTask::MarnavTask(std::string const& name)
    : MarnavTaskBase(name)
{
}

MarnavTask::~MarnavTask()
{
}

bool MarnavTask::configureHook()
{
    unique_ptr<Driver> driver(new Driver());
    iodrivers_base::ConfigureGuard guard(this);
    if (!_io_port.get().empty()) {
        driver->openURI(_io_port.get());
    }
    setDriver(driver.get());

    if (!MarnavTaskBase::configureHook()) {
        return false;
    }

    mDriver = move(driver);
    guard.commit();
    return true;
}
bool MarnavTask::startHook()
{
    if (! MarnavTaskBase::startHook()) {
        return false;
    }
    return true;
}
void MarnavTask::updateHook()
{
    MarnavTaskBase::updateHook();
}
void MarnavTask::processIO() {
    try {
        auto sentence = mDriver->readSentence();
        mNMEAStats.time = base::Time::now();
        mNMEAStats.received_sentences++;

        bool processed = processSentence(*sentence);
        if (!processed) {
            mNMEAStats.ignored_sentences++;
        }
    }
    catch (MarnavParsingError const& e) {
        mNMEAStats.invalid_sentences++;
        LOG_ERROR_S << "NMEA sentence not recognized by marnav: "
                    << e.what() << std::endl;
    }

    _nmea_stats.write(mNMEAStats);
}
void MarnavTask::errorHook()
{
    MarnavTaskBase::errorHook();
}
void MarnavTask::stopHook()
{
    MarnavTaskBase::stopHook();
}
void MarnavTask::cleanupHook()
{
    MarnavTaskBase::cleanupHook();
}
