# frozen_string_literal: true

using_task_library "nmea0183"

describe OroGen.nmea0183.GPSTask do
    run_live

    attr_reader :task

    before do
        @task = syskit_deploy(
            OroGen.nmea0183.GPSTask.deployed_as("gps_task")
        )

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
            add OroGen.nmea0183.GPSTask, as: "marnav"

            reader_writer_child.connect_to marnav_child.io_raw_in_port
            marnav_child.io_raw_out_port.connect_to reader_writer_child
        end
        cmp = syskit_stub_deploy_configure_and_start(
            cmp_m.use("marnav" => @task, "reader_writer" => io_reader_writer_m)
        )
        @io = cmp.reader_writer_child

        @rmc_msg = make_packet("$GNRMC,000848.00,V,2253.8645,S,04312.0880,W,,,060180,,,N*51\r\n")
        @gsa_msg = make_packet("$GNGSA,A,1,,,,,,,,,,,,,2.0,1.7,1.0*2B\r\n")
        @ais_msg = make_packet("!AIVDM,1,1,,A,15MgK45P3@G?fl0E`JbR0OwT0@MS,0*4E\r\n")
    end

    it "parses gsa and rmc messages into gps solution quality and position" do
        solution_quality = expect_execution do
            syskit_write @io.out_port, @gsa_msg
        end.to {
            have_one_new_sample(task.gps_solution_quality_port)
        }
        assert_equal(solution_quality.hdop, 1.7)
        assert_equal(solution_quality.pdop, 2.0)
        assert_equal(solution_quality.vdop, 1.0)
        position = expect_execution do
            syskit_write @io.out_port, @rmc_msg
        end.to {
            have_one_new_sample(task.gps_solution_port)
        }
        assert(position.latitude.nan?)
        assert(position.longitude.nan?)
        assert_equal(:INVALID, position.positionType)
        assert_equal(0, position.noOfSatellites)
    end

    it "increments the ignored sentences for non rmc or gsa messages" do
        stats = expect_execution do
            syskit_write @io.out_port, @ais_msg
        end.to {
            have_one_new_sample(task.nmea_stats_port)
        }
        assert_equal(stats.ignored_sentences, 1)
    end

    def make_packet(sentence)
        Types.iodrivers_base.RawPacket.new(
            time: Time.now,
            data: sentence.each_byte.to_a
        )
    end
end
