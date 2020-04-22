#ifndef nmea0183_TYPES_HPP
#define nmea0183_TYPES_HPP

#include <base/Time.hpp>

namespace nmea0183 {
    /** Statistics on sentences received at the NMEA level */
    struct NMEAStats {
        base::Time time;
        /** Count of messages that were rejected by marnav */
        uint32_t invalid_sentences = 0;
        /** Count of messages that were parsed by marnav */
        uint32_t received_sentences = 0;
        /**
         * Count of messages that were ignored by the specific task
         *
         * These are the messages received but not interpreted and
         * are included in \c received_messages as well
         */
        uint32_t ignored_sentences = 0;
    };

    /** Statistics on AIS messages */
    struct AISStats {
        base::Time time;
        /** Count of sentences that were discarded because their IDs were
         * not sequential (probable lost message)
         */
        uint32_t discarded_sentences = 0;
        /** Count of messages that were rejected during parsing by marnav */
        uint32_t invalid_messages = 0;
        /** Count of messages that were parsed by marnav */
        uint32_t received_messages = 0;
        /**
         * Count of messages that were ignored by the specific task
         *
         * These are the messages received but not interpreted and
         * are included in \c received_messages as well
         */
        uint32_t ignored_messages = 0;
    };
}

#endif

