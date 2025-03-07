# frozen_string_literal: true

using_task_library "nmea0183"
using_task_library "ais_base"

describe OroGen.nmea0183.AISTask do
    run_live

    describe "parse messages" do
        attr_reader :task

        before do
            @task, @io = setup_ais_task

            @msg01 = make_packet("!AIVDM,1,1,,A,15MgK45P3@G?fl0E`JbR0OwT0@MS,0*4E\r\n")
            @msg02 = make_packet("!AIVDM,1,1,,B,25Cjtd0Oj;Jp7ilG7=UkKBoB0<06,0*60\r\n")
            @msg03 = make_packet("!AIVDM,1,1,,A,38Id705000rRVJhE7cl9n;160000,0*40\r\n")
            @msg05 = make_packet(
                "!AIVDM,2,1,9,B,53nFBv01SJ<thHp6220H4heHTf2222222222221?50:"\
                "454o<`9QSlUDp,0*09\r\n"\
                "!AIVDM,2,2,9,B,888888888888880,2*2E\r\n"
            )
        end

        it "parses a msg01 into a position" do
            position, stats = expect_execution do
                syskit_write @io.out_port, @msg01
            end.to do
                [have_one_new_sample(task.positions_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 366_730_000, position.mmsi
            assert_in_delta -51.3 * Math::PI / 180, position.course_over_ground.rad, 1e-3
            assert_in_delta 37.80 * Math::PI / 180, position.latitude.rad, 1e-3
            assert_in_delta -122.39 * Math::PI / 180, position.longitude.rad, 1e-3
            assert_equal :STATUS_MOORED, position.status
            assert_equal 0, position.high_accuracy_position
            assert position.yaw.rad.nan?
            assert position.yaw_velocity.nan?
            assert_in_delta 10.7004352, position.speed_over_ground

            assert_equal 0, stats.discarded_sentences
            assert_equal 0, stats.invalid_messages
            assert_equal 1, stats.received_messages
            assert_equal 0, stats.ignored_messages
        end

        it "parses a msg02 into a position" do
            position, stats = expect_execution do
                syskit_write @io.out_port, @msg02
            end.to do
                [have_one_new_sample(task.positions_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 356_302_000, position.mmsi

            assert_equal 0, stats.discarded_sentences
            assert_equal 0, stats.invalid_messages
            assert_equal 1, stats.received_messages
            assert_equal 0, stats.ignored_messages
        end

        it "parses a msg03 into a position" do
            position, stats = expect_execution do
                syskit_write @io.out_port, @msg03
            end.to do
                [have_one_new_sample(task.positions_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 563_808_000, position.mmsi

            assert_equal 0, stats.discarded_sentences
            assert_equal 0, stats.invalid_messages
            assert_equal 1, stats.received_messages
            assert_equal 0, stats.ignored_messages
        end

        it "parses a msg05 into vessel and voyage info" do
            vessel, _, stats = expect_execution do
                syskit_write @io.out_port, @msg05
            end.to do
                [have_one_new_sample(task.vessels_information_port),
                 have_one_new_sample(task.voyages_information_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 258_315_000, vessel.mmsi
            assert_equal 6_514_895, vessel.imo
            assert_equal "LFNA", vessel.call_sign
            assert_equal "FALKVIK", vessel.name
            assert_equal :SHIP_TYPE_CARGO_NO_INFO, vessel.ship_type
            assert_equal :EPFD_GPS, vessel.epfd_fix
            assert_in_delta 3.8, vessel.draft, 1e-3
            assert_in_delta 50, vessel.length, 1e-3
            assert_in_delta 9, vessel.width, 1e-3
            assert_in_delta -15, vessel.reference_position.x, 1e-3
            assert_in_delta 0.5, vessel.reference_position.y, 1e-3
            assert_in_delta 0, vessel.reference_position.z, 1e-3

            assert_equal 0, stats.discarded_sentences
            assert_equal 0, stats.invalid_messages
            assert_equal 1, stats.received_messages
            assert_equal 0, stats.ignored_messages
        end

        it "ignores a message that is not of type 1, 2, 3 and 5" do
            msg04 = make_packet("!AIVDM,1,1,,A,403OviQuMGCqWrRO9>E6fE700@GO,0*4D\r\n")
            stats = expect_execution do
                syskit_write @io.out_port, msg04
            end.to { have_one_new_sample task.ais_stats_port }

            assert_equal 0, stats.discarded_sentences
            assert_equal 0, stats.invalid_messages
            assert_equal 1, stats.received_messages
            assert_equal 1, stats.ignored_messages
        end

        it "counts messages that are discarded because arriving out of order" do
            msg = "!AIVDM,3,1,3,B,I`1ifG20UrcNTFE?UgLeo@Dk:o6G4hhI8;?vW2?El>D,0*25\r\n"\
                "!AIVDM,3,1,3,B,I`1ifG20UrcNTFE?UgLeo@Dk:o6G4hhI8;?vW2?El>D,0*25\r\n"

            _, stats = expect_execution { syskit_write @io.out_port, make_packet(msg) }
                       .to do
                [have_one_new_sample(task.nmea_stats_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 1, stats.discarded_sentences
            assert_equal 0, stats.invalid_messages
            assert_equal 0, stats.received_messages
            assert_equal 0, stats.ignored_messages
        end

        it "corrects vessel's coordinates using yaw if corresponding " \
           "VesselInformation exists and 'use_sensor_offset_correction flag is on" do
            # reference_position = [100, 50, 0]
            vessel_info_msg =
                "!AIVDM,2,1,,B,55MgK40000000000003wwwwww40000000000000001T0j00" \
                "Ht0000000,0*77\r\n!AIVDM,2,2,,B,000000000000008,2*1F\r\n"

            expect_execution do
                syskit_write @io.out_port, make_packet(vessel_info_msg)
            end.to do
                [have_one_new_sample(task.vessels_information_port),
                 have_one_new_sample(task.voyages_information_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            # lat = 45; long = -120
            # yaw = 90
            position_msg = "!AIVDM,1,1,,B,15MgK4?P00GJch0Igth>42oh0000,0*20\r\n"

            position, = expect_execution do
                syskit_write @io.out_port, make_packet(position_msg)
            end.to do
                [have_one_new_sample(task.positions_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 366_730_000, position.mmsi
            assert_equal :POSITION_CENTERED_USING_HEADING, position.correction_status
            assert_in_delta 44.9991 * Math::PI / 180, position.latitude.rad, 1e-4
            assert_in_delta -119.9993 * Math::PI / 180, position.longitude.rad, 1e-4
        end

        it "corrects vessel's coordinates using course if corresponding " \
           "VesselInformation exists and 'use_sensor_offset_correction flag is on" do
            # reference_position = [100, 50, 0]
            vessel_info_msg =
                "!AIVDM,2,1,,B,55MgK40000000000003wwwwww40000000000000001T0j00" \
                "Ht0000000,0*77\r\n!AIVDM,2,2,,B,000000000000008,2*1F\r\n"

            expect_execution do
                syskit_write @io.out_port, make_packet(vessel_info_msg)
            end.to do
                [have_one_new_sample(task.vessels_information_port),
                 have_one_new_sample(task.voyages_information_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            # lat = 45; long = -120
            # cog = 90; sog = 10
            position_msg = "!AIVDM,1,1,,B,15MgK4?P1cGJch0Igth3Q?wh0000,0*0F\r\n"

            position, = expect_execution do
                syskit_write @io.out_port, make_packet(position_msg)
            end.to do
                [have_one_new_sample(task.positions_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 366_730_000, position.mmsi
            assert_equal :POSITION_CENTERED_USING_COURSE, position.correction_status
            assert_in_delta 44.9991 * Math::PI / 180, position.latitude.rad, 1e-4
            assert_in_delta -119.9993 * Math::PI / 180, position.longitude.rad, 1e-4
        end

        it "expects no correction if there's no corresponding vessel" do
            expect_execution do
                syskit_write @io.out_port, @msg05
            end.to do
                [have_one_new_sample(task.vessels_information_port),
                 have_one_new_sample(task.voyages_information_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            # lat = 45; long = -120
            position_msg = "!AIVDM,1,1,,B,15MgK4?P00GJch0Igth>42oh0000,0*20\r\n"

            position, = expect_execution do
                syskit_write @io.out_port, make_packet(position_msg)
            end.to do
                [have_one_new_sample(task.positions_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal :POSITION_RAW, position.correction_status
            assert_in_delta 45 * Math::PI / 180, position.latitude.rad, 1e-4
            assert_in_delta -120 * Math::PI / 180, position.longitude.rad, 1e-4
        end
    end

    describe "sensor offset correction flag off" do
        attr_reader :task

        before do
            @task, @io = setup_ais_task(use_sensor_offset_correction: false)
        end

        it "expects no correction if 'use_sensor_offset_correction' flag is off" do
            # reference_position = [100, 50, 0]
            vessel_info_msg =
                "!AIVDM,2,1,,B,55MgK40000000000003wwwwww40000000000000001T0j00" \
                "Ht0000000,0*77\r\n!AIVDM,2,2,,B,000000000000008,2*1F\r\n"

            expect_execution do
                syskit_write @io.out_port, make_packet(vessel_info_msg)
            end.to do
                [have_one_new_sample(task.vessels_information_port),
                 have_one_new_sample(task.voyages_information_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            # lat = 45; long = -120
            position_msg = "!AIVDM,1,1,,B,15MgK4?P1cGJch0Igth3Q?wh0000,0*0F\r\n"

            position, = expect_execution do
                syskit_write @io.out_port, make_packet(position_msg)
            end.to do
                [have_one_new_sample(task.positions_port),
                 have_one_new_sample(task.ais_stats_port)]
            end

            assert_equal 366_730_000, position.mmsi
            assert_equal :POSITION_RAW, position.correction_status
            assert_in_delta 45 * Math::PI / 180, position.latitude.rad, 1e-4
            assert_in_delta -120 * Math::PI / 180, position.longitude.rad, 1e-4
        end
    end

    def setup_ais_task(use_sensor_offset_correction: true) # rubocop:disable Metrics/AbcSize
        @task = syskit_deploy(
            OroGen.nmea0183.AISTask.deployed_as("ais_task")
        )
        @task.properties.utm_configuration = Types.gps_base.UTMConversionParameters.new(
            nwu_origin: Eigen::Vector3.new(1, 1, 0),
            utm_zone: 11,
            utm_north: true
        )
        @task.properties.use_sensor_offset_correction = use_sensor_offset_correction

        # This complicated setup works around that data readers and writers
        # in Syskit connect themselves only when their target tasks are running
        #
        # We need to be connected before configure
        io_reader_writer_m = Syskit::DataService.new_submodel do
            input_port "in", "/iodrivers_base/RawPacket"
            output_port "out", "/iodrivers_base/RawPacket"
        end

        cmp_m = Syskit::Composition.new_submodel do
            add io_reader_writer_m, as: "reader_writer"
            add OroGen.nmea0183.AISTask, as: "marnav"

            reader_writer_child.connect_to marnav_child.io_raw_in_port
            marnav_child.io_raw_out_port.connect_to reader_writer_child
        end
        cmp = syskit_stub_deploy_configure_and_start(
            cmp_m.use("marnav" => @task, "reader_writer" => io_reader_writer_m)
        )
        [@task, cmp.reader_writer_child]
    end

    def make_packet(sentence)
        Types.iodrivers_base.RawPacket.new(
            time: Time.now,
            data: sentence.each_byte.to_a
        )
    end
end
