#include <stdint.h>

typedef struct {
    uint64_t        freq;
} app_conf_t;

#define APP_CONF_OK     0

int load_conf(app_conf_t * conf, int argc, char ** argv);
