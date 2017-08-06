#include <stdint.h>

#define APP_CONF_OK         0
#define APP_CONF_ERROR      1

typedef struct {
    uint64_t        rf_freq;
    double          ppm;
    uint8_t         rf_gain;
    uint8_t         if_gain;
    bool            udp_input;
} app_conf_t;

int app_conf_init(app_conf_t * conf, int argc, char ** argv);
