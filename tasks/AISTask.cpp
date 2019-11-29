/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AISTask.hpp"

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
    if (! AISTaskBase::configureHook())
        return false;
    return true;
}
bool AISTask::startHook()
{
    if (! AISTaskBase::startHook())
        return false;
    return true;
}
void AISTask::updateHook()
{
    AISTaskBase::updateHook();
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
}
