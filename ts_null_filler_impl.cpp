/*
 * Simple DVB-S2 transmitter application based on gnuradio/gr-dtv
 *
 * Copyright 2018 Alexandru Csete OZ9AEC.
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

#include <gnuradio/io_signature.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "time.h"
#include "ts_null_filler_impl.h"


#define TS_RATE     1512500         // bytes/sec
#define BUF_LEN     (TS_RATE/1)     // 1 second


ts_null_filler_bb::sptr ts_null_filler_bb::make(int pps)
{
    return gnuradio::get_initial_sptr(new ts_null_filler_bb_impl(pps));
}

ts_null_filler_bb_impl::ts_null_filler_bb_impl(int pps) :
    gr::block("ts_null_filler_bb",
              gr::io_signature::make(1, 1, sizeof(unsigned char)),
              gr::io_signature::make(1, 1, sizeof(unsigned char)))
{
    set_alignment(TS_PACKET_SIZE);

	/* TS NULL packet */
	memset(null_ts_packet, 0, TS_PACKET_SIZE);
	null_ts_packet[0] = 0x47;
	null_ts_packet[1] = 0x1F;
	null_ts_packet[2] = 0xFF;
	null_ts_packet[3] = 0x10;

    null_ts_num = pps;
    null_ts_dt_ms = 1000;

    tlast = time_ms();
}

ts_null_filler_bb_impl::~ts_null_filler_bb_impl()
{
}

void ts_null_filler_bb_impl::forecast(int noutput_items,
                                 gr_vector_int &ninput_items_required)
{
    ninput_items_required[0] = noutput_items;
}

int ts_null_filler_bb_impl::general_work(int noutput_items,
                                    gr_vector_int &ninput_items,
                                    gr_vector_const_void_star &input_items,
                                    gr_vector_void_star &output_items)
{
    (void) ninput_items;
    const unsigned char *in = (const unsigned char *) input_items[0];
    unsigned char *out = (unsigned char *) output_items[0];

    uint64_t    tnow = time_ms();

    if (null_ts_num > 0 &&
        tnow - tlast > null_ts_dt_ms &&
        noutput_items % TS_PACKET_SIZE == 0 &&
        noutput_items > null_ts_num * TS_PACKET_SIZE)
    {
        //fprintf(stderr, "Inserting %d NULL packets\n", null_ts_num);
        tlast = tnow;

        memcpy(out, in, noutput_items - null_ts_num * TS_PACKET_SIZE);

        int i;
        for (i = null_ts_num; i > 0; i--)
            memcpy(&out[noutput_items - i * TS_PACKET_SIZE], null_ts_packet, TS_PACKET_SIZE);

        consume_each(noutput_items - null_ts_num * TS_PACKET_SIZE);
    }
    else
    {
        memcpy(out, in, noutput_items);
        consume_each(noutput_items);
    }

    return noutput_items;
}
