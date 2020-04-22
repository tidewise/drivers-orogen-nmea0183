# frozen_string_literal: true

using_task_library "nmea0183"

describe OroGen.nmea0183.MarnavTask do
    run_live

    attr_reader :task

    before do
        @task = syskit_deploy(
            OroGen.nmea0183.test.MarnavTestTask.deployed_as("marnav_task")
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
            add OroGen.nmea0183.MarnavTask, as: "marnav"

            reader_writer_child.connect_to marnav_child.io_raw_in_port
            marnav_child.io_raw_out_port.connect_to reader_writer_child
        end
        cmp = syskit_stub_deploy_configure_and_start(
            cmp_m.use("marnav" => @task, "reader_writer" => io_reader_writer_m)
        )
        @io = cmp.reader_writer_child

        @xdr_sentence = make_packet("$IIXDR,C,19.52,C,TempAir*19\r\n")
        @zda_sentence = make_packet("$GPZDA,160012.71,11,03,2004,-1,00*7D\r\n")
        # Invalid number of fields, but format and checksum are valid
        @invalid_zda_sentence = make_packet("$GPZDA,160012.71,03,2004,-1,00*51\r\n")
    end

    it "passes the received sentences to the subclass" do
        sentence = expect_execution do
            syskit_write @io.out_port, @xdr_sentence
        end.to { have_one_new_sample task.received_sentences_port }
        assert_equal "$IIXDR,C,19.52,C,TempAir*19", sentence
    end

    it "counts received sentences" do
        stats = expect_execution do
            syskit_write @io.out_port, @xdr_sentence
        end.to { have_one_new_sample task.nmea_stats_port }
        assert_equal 0, stats.invalid_sentences
        assert_equal 1, stats.received_sentences
        assert_equal 0, stats.ignored_sentences
    end

    it "counts ignored sentences" do
        stats = expect_execution do
            syskit_write @io.out_port, @zda_sentence
        end.to { have_one_new_sample task.nmea_stats_port }
        assert_equal 0, stats.invalid_sentences
        assert_equal 1, stats.received_sentences
        assert_equal 1, stats.ignored_sentences
    end

    it "counts sentences not recognized by marnav" do
        stats = expect_execution do
            syskit_write @io.out_port, @invalid_zda_sentence
        end.to { have_one_new_sample task.nmea_stats_port }
        assert_equal 1, stats.invalid_sentences
        assert_equal 0, stats.received_sentences
        assert_equal 0, stats.ignored_sentences
    end

    def make_packet(sentence)
        Types.iodrivers_base.RawPacket.new(
            time: Time.now,
            data: sentence.each_byte.to_a
        )
    end
end
