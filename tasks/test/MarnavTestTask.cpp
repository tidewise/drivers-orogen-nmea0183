/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "MarnavTestTask.hpp"

using namespace nmea0183::test;

MarnavTestTask::MarnavTestTask(std::string const& name)
    : MarnavTestTaskBase(name)
{
}

MarnavTestTask::~MarnavTestTask()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See MarnavTestTask.hpp for more detailed
// documentation about them.

bool MarnavTestTask::configureHook()
{
    if (! MarnavTestTaskBase::configureHook())
        return false;
    return true;
}
bool MarnavTestTask::startHook()
{
    if (! MarnavTestTaskBase::startHook())
        return false;
    return true;
}
void MarnavTestTask::updateHook()
{
    MarnavTestTaskBase::updateHook();
}
void MarnavTestTask::errorHook()
{
    MarnavTestTaskBase::errorHook();
}
bool MarnavTestTask::processSentence(marnav::nmea::sentence const& sentence) {
    _received_sentences.write(to_string(sentence));
    return sentence.id() == marnav::nmea::sentence_id::XDR;
}
void MarnavTestTask::stopHook()
{
    MarnavTestTaskBase::stopHook();
}
void MarnavTestTask::cleanupHook()
{
    MarnavTestTaskBase::cleanupHook();
}
