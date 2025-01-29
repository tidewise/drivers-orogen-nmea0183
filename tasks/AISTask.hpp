/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef NMEA0183_AISTASK_TASK_HPP
#define NMEA0183_AISTASK_TASK_HPP

#include "nmea0183/AISTaskBase.hpp"
#include <base/samples/RigidBodyState.hpp>
#include <gps_base/BaseTypes.hpp>
#include <gps_base/UTMConverter.hpp>
#include <optional>

namespace nmea0183 {
    class Driver;
    class AIS;

    /*! \class AISTask
     * \brief Parsing of an AIS NMEA0183 stream
     */
    class AISTask : public AISTaskBase {
        /** The base class is auto-generated by orogen to define the task's interface
         *
         * It is located in the .orogen/tasks folder
         */
        friend class AISTaskBase;

    protected:
        std::unique_ptr<AIS> m_AIS;
        AISStats m_AIS_stats;
        std::map<int, ais_base::VesselInformation> m_vessels;
        gps_base::UTMConverter m_UTM_converter;
        bool m_use_sensor_offset_correction;
        void processPositionReport(std::optional<ais_base::Position> position, int mmsi);
        bool processSentence(marnav::nmea::sentence const& sentence);
        std::optional<ais_base::VesselInformation> getCorrespondingVesselInfo(int mmsi);
        void updateKnownVessels(ais_base::VesselInformation info);

    public:
        /** TaskContext constructor for AISTask
         * \param name Name of the task. This name needs to be unique to make
         *             it identifiable via nameservices.
         * \param initial_state The initial TaskState of the TaskContext.
         *                      This is deprecated. It should always be the
         *                      configure state.
         */
        AISTask(std::string const& name = "nmea0183::AISTask");

        ~AISTask();

        /**
         * Hook called when the state machine transitions from PreOperational to
         * Stopped.
         *
         * If the code throws an exception, the transition will be aborted
         * and the component will end in the EXCEPTION state instead
         *
         * @return true if the transition can continue, false otherwise
         */
        bool configureHook();

        /**
         * Hook called when the state machine transition from Stopped to Running
         *
         * If the code throws an exception, the transition will be aborted
         * and the component will end in the EXCEPTION state instead
         *
         * @return true if the transition is successful, false otherwise
         */
        bool startHook();

        /**
         * Hook called on trigger in the Running state
         *
         * When this hook is exactly called depends on the chosen task's activity.
         * For instance, if the task context is declared as periodic in the orogen
         * specification, the task will be called at a fixed period.
         *
         * See Rock's documentation for a list of available triggering mechanisms
         *
         * The error(), exception() and fatal() calls, when called in this hook,
         * allow to get into the associated RunTimeError, Exception and
         * FatalError states.
         */
        void updateHook();

        /**
         * Hook called in the RuntimeError state, under the same conditions than
         * the updateHook
         *
         * Call recover() to go back in the Runtime state.
         */
        void errorHook();

        /**
         * Hook called when the component transitions out of the Running state
         *
         * This is called for any transition out of running, that is the
         * transitions to Stopped, Exception and Fatal
         */
        void stopHook();

        /**
         * Hook called on all transitions to PreOperational
         *
         * This is called for all transitions into PreOperational, that is
         * either from Stopped or Exception
         */
        void cleanupHook();
    };
}

#endif
