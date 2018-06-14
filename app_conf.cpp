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
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "app_conf.h"

#define PROG_NAME "dvbs2_tx"


struct option options[] = {
    /* simple switches with no argument */
    { "help",          no_argument,       0, 'h'},
    { "probe",         no_argument,       0, 'p'},

    /* input settings */
    { "udp",           no_argument,       0, 'u'},
    { "frequency",     required_argument, 0, 'f'},
    { "correction",    required_argument, 0, 'c'},
    { "symbol-rate",   required_argument, 0, 'r'},
    { "bandwidth",     required_argument, 0, 'b'},
    { "gain",          required_argument, 0, 'g'},
    { "null-pps",      required_argument, 0, 'n'},

    { 0, 0, 0, 0}
};

static void help(void)
{
    fprintf(stdout, "%s",
        "\n Usage: " PROG_NAME " [options]\n"
        "\n"
        "  -h        --help\n"
        "  -p        --probe\n"
        "  -u        --udp\n"
        "  -f 1.2G   --frequency=1280M\n"
        "  -r 8.0M   --symbol-rate=8000k\n"
        "  -c 12.5   --correction=12.5\n"
        "  -b 7M     --bandwidth=7000k\n"
        "  -g 25     --gain=25     (0...61)\n"
        "  -n 5      --null-pps=5  (0...10)\n"
        "\n"
        );
}

/* Parse frequency string as double */
static double parse_freq_double(char * freq_str)
{
    size_t n = strlen(freq_str);
    double multi = 1.0;

    if (n == 0)
        return 0;

    switch (freq_str[n-1])
    {
        case 'k':
        case 'K':
            multi = 1.e3;
            break;

        case 'M':
            multi = 1.e6;
            break;

        case 'G':
            multi = 1.e9;
            break;
    }

    return multi * atof(freq_str);
}

/* Parse frequency string as uint64. */
static uint64_t parse_freq_u64(char * freq_str)
{
    size_t n = strlen(freq_str);
    double multi = 1.0;
    double res;

    if (n == 0)
        return 0;

    switch (freq_str[n-1])
    {
        case 'k':
        case 'K':
            multi = 1.e3;
            break;

        case 'M':
            multi = 1.e6;
            break;

        case 'G':
            multi = 1.e9;
            break;
    }

    res = multi * atof(freq_str);
    if (res < 0.0)
        res = 0;

    return (uint64_t)(res);
}

static void print_conf(app_conf_t * conf)
{
    fprintf(stderr, "Transmitter configuration:\n"
            "   TS input: %s\n"
            "   TS  corr: %d PPS\n"
            "   RF  freq: %"PRIu64" Hz\n"
            "   Sym rate: %.3f kps\n"
            "  Bandwidth: %.3f kHz\n"
            "   Freq cor: %.2f ppm\n"
            "       Gain: %u\n",
            conf->udp_input ? "UDP" : "stdin", conf->pps,
            conf->rf_freq, 1.0e-3 * conf->sym_rate, conf->bw * 1.0e-3,
            conf->ppm, conf->gain);
}

int app_conf_init(app_conf_t * conf, int argc, char ** argv)
{
    int         option;
    int         idx;

    conf->rf_freq = 1280000000;
    conf->sym_rate = 6250.0e3;
    conf->bw = 0.0;
    conf->ppm = 0.0;
    conf->pps = 5;
    conf->gain = 0;
    conf->udp_input = false;
    conf->probe = false;

    fprintf(stderr, "\n %s %s\n", PROG_NAME, VERSION);

    if (argc > 1)
    {
        while ((option = getopt_long(argc, argv, "c:f:r:b:g:n:puh", options, &idx)) != -1)
        {
            switch (option)
            {
            case 'c':
                conf->ppm = atof(optarg);
                break;
            case 'f':
                conf->rf_freq = parse_freq_u64(optarg);
                break;
            case 'r':
                conf->sym_rate = parse_freq_double(optarg);
                break;
            case 'b':
                conf->bw = parse_freq_double(optarg);
                break;
            case 'u':
                conf->udp_input = true;
                break;
            case 'p':
                conf->probe = true;
                break;
            case 'g':
                conf->gain = atoi(optarg);
                break;
            case 'n':
                conf->pps = atoi(optarg);
                if (conf->pps < 0)
                    conf->pps = 0;
                else if (conf->pps > 10)
                    conf->pps = 10;
                break;
            case 'h':
            default:
                help();
                return APP_CONF_ERROR;
            }
        }
    }
    print_conf(conf);

    return APP_CONF_OK;
}
