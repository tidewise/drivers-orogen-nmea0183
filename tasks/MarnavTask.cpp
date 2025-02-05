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

    m_driver = move(driver);
    guard.commit();
    return true;
}
bool MarnavTask::startHook()
{
    if (! MarnavTaskBase::startHook()) {
        return false;
    }

    /** Flush any data already received. It might be a partial sentence, and
     * since some NMEA devices are slow-sending (e.g. AIS), simply relying on
     * receiving another message will timeout
     */
    static const int BUFFER_SIZE = marnav::nmea::sentence::max_length * 2;
    uint8_t buffer[BUFFER_SIZE];
    try {
        while(true) {
            m_driver->readPacket(buffer, BUFFER_SIZE, base::Time(), base::Time());
        }
    }
    catch(iodrivers_base::TimeoutError&) {}

    return true;
}
void MarnavTask::updateHook()
{
    MarnavTaskBase::updateHook();
}
void MarnavTask::processIO() {
    static const int BUFFER_SIZE = marnav::nmea::sentence::max_length * 2;
    uint8_t buffer[BUFFER_SIZE];
    try {
        int sentence_size = m_driver->readPacket(buffer, BUFFER_SIZE);

        char* buffer_s = reinterpret_cast<char*>(buffer);
        string sentence_string(buffer_s, buffer_s + sentence_size - 2);
        processRawSentence(sentence_string);
    }
    catch (MarnavParsingError const& e) {
        m_nmea_stats.invalid_sentences++;
        LOG_ERROR_S << "NMEA sentence not recognized by marnav: "
                    << e.what() << std::endl;
    }

    _nmea_stats.write(m_nmea_stats);
}

void MarnavTask::processRawSentence(std::string const& sentence_string) {
    unique_ptr<marnav::nmea::sentence> sentence;
    try {
        sentence = marnav::nmea::make_sentence(sentence_string);
    }
    catch (std::exception const& e) {
        m_nmea_stats.invalid_sentences++;
        LOG_ERROR_S << "NMEA sentence not recognized by marnav: "
                    << e.what() << std::endl;
        return;
    }

    m_nmea_stats.time = base::Time::now();
    m_nmea_stats.received_sentences++;

    bool processed = processSentence(*sentence);
    if (!processed) {
        m_nmea_stats.ignored_sentences++;
    }
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
