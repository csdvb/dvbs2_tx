#include <stdint.h>

#define APP_CONF_OK         0
#define APP_CONF_ERROR      1

typedef struct {
    uint64_t        rf_freq;
    double          sym_rate;
    double          ppm;
    double          bw;         // analog bandwidth
    int             pps;        // TS NULL packets per second to insert
    uint8_t         gain;
    bool            udp_input;
    bool            probe;
} app_conf_t;

int app_conf_init(app_conf_t * conf, int argc, char ** argv);
