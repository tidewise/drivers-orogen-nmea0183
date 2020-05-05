/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "MarnavProcessRawSentenceTask.hpp"

using namespace nmea0183::test;

MarnavProcessRawSentenceTask::MarnavProcessRawSentenceTask(std::string const& name)
    : MarnavProcessRawSentenceTaskBase(name)
{
}

MarnavProcessRawSentenceTask::~MarnavProcessRawSentenceTask()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See MarnavProcessRawSentenceTask.hpp for more detailed
// documentation about them.

bool MarnavProcessRawSentenceTask::configureHook()
{
    if (! MarnavProcessRawSentenceTaskBase::configureHook())
        return false;
    return true;
}
bool MarnavProcessRawSentenceTask::startHook()
{
    if (! MarnavProcessRawSentenceTaskBase::startHook())
        return false;
    return true;
}
void MarnavProcessRawSentenceTask::updateHook()
{
    MarnavProcessRawSentenceTaskBase::updateHook();
}
void MarnavProcessRawSentenceTask::processRawSentence(std::string const& s) {
    MarnavProcessRawSentenceTaskBase::processRawSentence(
        "$GPZDA,160012.71,11,03,2004,-1,00*7D"
    );
}
void MarnavProcessRawSentenceTask::errorHook()
{
    MarnavProcessRawSentenceTaskBase::errorHook();
}
void MarnavProcessRawSentenceTask::stopHook()
{
    MarnavProcessRawSentenceTaskBase::stopHook();
}
void MarnavProcessRawSentenceTask::cleanupHook()
{
    MarnavProcessRawSentenceTaskBase::cleanupHook();
}
