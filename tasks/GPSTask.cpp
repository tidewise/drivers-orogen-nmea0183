/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "GPSTask.hpp"
#include <base-logging/Logging.hpp>

using namespace std;
using namespace nmea0183;
using namespace marnav;

GPSTask::GPSTask(std::string const& name)
    : GPSTaskBase(name)
{
}

GPSTask::~GPSTask()
{
}

bool GPSTask::configureHook()
{
    if (!GPSTaskBase::configureHook()) {
        return false;
    }
    return true;
}
bool GPSTask::startHook()
{
    if (!GPSTaskBase::startHook()) {
        return false;
    }
    return true;
}
void GPSTask::updateHook()
{
    GPSTaskBase::updateHook();
}
bool GPSTask::processSentence(marnav::nmea::sentence const& sentence)
{
    if (sentence.id() == nmea::sentence_id::RMC) {
        auto rmc = nmea::sentence_cast<nmea::rmc>(&sentence);
        if (rmc && m_gsa) {
            auto gps_solution = nmea0183::GPS::getSolution(*rmc, *m_gsa);
            _gps_solution.write(gps_solution);
        }
        return true;
    }
    if (sentence.id() == nmea::sentence_id::GSA) {
        auto gsa = nmea::sentence_cast<nmea::gsa>(&sentence);
        if (gsa) {
            m_gsa = make_unique<marnav::nmea::gsa>(*gsa);
            auto gps_solution_quality = nmea0183::GPS::getSolutionQuality(*m_gsa);
            _gps_solution_quality.write(gps_solution_quality);
        }
        return true;
    }
    return false;
}
void GPSTask::errorHook()
{
    GPSTaskBase::errorHook();
}
void GPSTask::stopHook()
{
    GPSTaskBase::stopHook();
}
void GPSTask::cleanupHook()
{
    GPSTaskBase::cleanupHook();
}
