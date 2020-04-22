/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef NMEA0183_MARNAVTASK_TASK_HPP
#define NMEA0183_MARNAVTASK_TASK_HPP

#include "nmea0183/MarnavTaskBase.hpp"
#include <marnav/nmea/sentence.hpp>

namespace nmea0183{
    class Driver;

    /*! \class MarnavTask
     *
     * Base implementation for components that read NMEA0183 sentences using
     * iodrivers_base and marnav
     */
    class MarnavTask : public MarnavTaskBase
    {
        friend class MarnavTaskBase;

    protected:
        std::unique_ptr<Driver> mDriver;
        NMEAStats mNMEAStats;

        virtual void processIO();

        /** Method that should be implemented in specific drivers to
         * handle incoming sentences
         *
         * @return true if the message was processed, false otherwise
         */
        virtual bool processSentence(marnav::nmea::sentence const& sentence) = 0;

    public:
        /** TaskContext constructor for MarnavTask
         */
        MarnavTask(std::string const& name = "nmea0183::MarnavTask");

        /** Default deconstructor of MarnavTask
         */
        ~MarnavTask();

        bool configureHook();

        bool startHook();
        void updateHook();
        void errorHook();
        void stopHook();
        void cleanupHook();
    };
}

#endif

