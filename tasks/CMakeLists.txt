# Generated from orogen/lib/orogen/templates/tasks/CMakeLists.txt

include(nmea0183TaskLib)
ADD_LIBRARY(${NMEA0183_TASKLIB_NAME} SHARED
    ${NMEA0183_TASKLIB_SOURCES})
add_dependencies(${NMEA0183_TASKLIB_NAME}
    regen-typekit)


TARGET_LINK_LIBRARIES(${NMEA0183_TASKLIB_NAME}
    ${OrocosRTT_LIBRARIES}
    ${QT_LIBRARIES}
    ${NMEA0183_TASKLIB_DEPENDENT_LIBRARIES})
SET_TARGET_PROPERTIES(${NMEA0183_TASKLIB_NAME}
    PROPERTIES LINK_INTERFACE_LIBRARIES "${NMEA0183_TASKLIB_INTERFACE_LIBRARIES}")
SET_TARGET_PROPERTIES(${NMEA0183_TASKLIB_NAME}
    PROPERTIES INTERFACE_LINK_LIBRARIES "${NMEA0183_TASKLIB_INTERFACE_LIBRARIES}")

INSTALL(TARGETS ${NMEA0183_TASKLIB_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib/orocos)

INSTALL(FILES ${NMEA0183_TASKLIB_HEADERS}
    DESTINATION include/orocos/nmea0183)

