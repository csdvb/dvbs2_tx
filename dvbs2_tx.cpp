/*
 * Simple DVB-S2 transmitter application based on gnuradio/gr-dtv
 *
 * Copyright 2017 Alexandru Csete OZ9AEC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <gnuradio/blocks/file_source.h>
#include <gnuradio/dtv/dvb_bbheader_bb.h>
#include <gnuradio/dtv/dvb_bbscrambler_bb.h>
#include <gnuradio/dtv/dvb_bch_bb.h>
#include <gnuradio/dtv/dvb_config.h>
#include <gnuradio/dtv/dvb_ldpc_bb.h>
#include <gnuradio/dtv/dvbs2_config.h>
#include <gnuradio/dtv/dvbs2_interleaver_bb.h>
#include <gnuradio/dtv/dvbs2_modulator_bc.h>
#include <gnuradio/dtv/dvbs2_physical_cc.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/top_block.h>
#include <osmosdr/sink.h>
#include <signal.h>
#include <stdio.h>
#include <vector>


#define CODE_RATE       gr::dtv::C3_5
#define CONSTELLATION   gr::dtv::MOD_8PSK
#define SYMBOL_RATE     8.0e6

static bool keep_running;

static void signal_handler(int signo)
{
    if (signo == SIGINT)
        fputs("\nCaught SIGINT\n", stderr);
    else if (signo == SIGTERM)
        fputs("\nCaught SIGTERM\n", stderr);
    else if (signo == SIGHUP)
        fputs("\nCaught SIGHUP\n", stderr);
    else if (signo == SIGPIPE)
        fputs("\nReceived SIGPIPE.\n", stderr);
    else
        fprintf(stderr, "\nCaught signal: %d\n", signo);

    keep_running = false;
}

int main(int argc, char **argv)
{
    gr::top_block_sptr                  tb;
    gr::blocks::file_source::sptr       ts_source;
    gr::dtv::dvb_bbheader_bb::sptr      bb_header;
    gr::dtv::dvb_bbscrambler_bb::sptr   bb_scrambler;
    gr::dtv::dvb_bch_bb::sptr           bch_enc;
    gr::dtv::dvb_ldpc_bb::sptr          ldpc_enc;
    gr::dtv::dvbs2_interleaver_bb::sptr interleaver;
    gr::dtv::dvbs2_modulator_bc::sptr   modulator;
    gr::dtv::dvbs2_physical_cc::sptr    pl_framer;
    gr::filter::fft_filter_ccf::sptr    filter;
    osmosdr::sink::sptr                 iq_sink;

    std::vector<float>                  filter_taps;


    // register signal handlers
    if (signal(SIGINT, signal_handler) == SIG_ERR)
        fputs("Warning: Can not install signal handler for SIGINT\n", stderr);
    if (signal(SIGTERM, signal_handler) == SIG_ERR)
        fputs("Warning: Can not install signal handler for SIGTERM\n", stderr);
    if (signal(SIGHUP, signal_handler) == SIG_ERR)
        fputs("Warning: Can not install signal handler for SIGHUP\n", stderr);
    if (signal(SIGPIPE, signal_handler) == SIG_ERR)
        fputs("Warning: Can not install signal handler for SIGPIPE\n", stderr);


    tb = gr::make_top_block("dvbs2_tx");
    ts_source = gr::blocks::file_source::make(sizeof(char), "/dev/stdin", false);

    bb_header = gr::dtv::dvb_bbheader_bb::make(gr::dtv::STANDARD_DVBS2,
                                               gr::dtv::FECFRAME_NORMAL,
                                               CODE_RATE,
                                               gr::dtv::RO_0_35,
                                               gr::dtv::INPUTMODE_NORMAL,
                                               gr::dtv::INBAND_OFF,
                                               0, 0);
    bb_scrambler = gr::dtv::dvb_bbscrambler_bb::make(gr::dtv::STANDARD_DVBS2,
                                                     gr::dtv::FECFRAME_NORMAL,
                                                     CODE_RATE);
    bch_enc = gr::dtv::dvb_bch_bb::make(gr::dtv::STANDARD_DVBS2,
                                        gr::dtv::FECFRAME_NORMAL,
                                        CODE_RATE);
    ldpc_enc = gr::dtv::dvb_ldpc_bb::make(gr::dtv::STANDARD_DVBS2,
                                        gr::dtv::FECFRAME_NORMAL,
                                        CODE_RATE,
                                        CONSTELLATION);
    interleaver = gr::dtv::dvbs2_interleaver_bb::make(gr::dtv::FECFRAME_NORMAL,
                                                      CODE_RATE,
                                                      CONSTELLATION);
    modulator = gr::dtv::dvbs2_modulator_bc::make(gr::dtv::FECFRAME_NORMAL,
                                                  CODE_RATE,
                                                  CONSTELLATION,
                                                  gr::dtv::INTERPOLATION_OFF);
    pl_framer = gr::dtv::dvbs2_physical_cc::make(gr::dtv::FECFRAME_NORMAL,
                                                 CODE_RATE,
                                                 CONSTELLATION,
                                                 gr::dtv::PILOTS_ON,
                                                 0);

    filter_taps = gr::filter::firdes::root_raised_cosine(1.0, 2*SYMBOL_RATE, SYMBOL_RATE, 0.35, 100);
    filter = gr::filter::fft_filter_ccf::make(1, filter_taps, 1);
    iq_sink = osmosdr::sink::make("hackrf");
    iq_sink->set_sample_rate(2*SYMBOL_RATE);
    iq_sink->set_center_freq(1280.0e6, 0);
    iq_sink->set_gain(14, "RF", 0);
    iq_sink->set_gain(47, "IF", 0);

    tb->connect(ts_source, 0, bb_header, 0);
    tb->connect(bb_header, 0, bb_scrambler, 0);
    tb->connect(bb_scrambler, 0, bch_enc, 0);
    tb->connect(bch_enc, 0, ldpc_enc, 0);
    tb->connect(ldpc_enc, 0, interleaver, 0);
    tb->connect(interleaver, 0, modulator, 0);
    tb->connect(modulator, 0, pl_framer, 0);
    tb->connect(pl_framer, 0, filter, 0);
    tb->connect(filter, 0, iq_sink, 0);

    tb->start();
    fputs("Flow graph running\n", stderr);
    keep_running = true;
    while (keep_running)
    {
        sleep(1);
    }

    fputs("Stopping flow graph\n", stderr);
    tb->stop();

    return 0;
}
