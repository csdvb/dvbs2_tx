
/* TX control interface through unix domain socket */

#ifdef __cplusplus
extern "C" {
#endif

/* returns 1 if there is a new TX power */
int ctl_if_poll(void);

int  ctl_if_get_tx_power(void);
void ctl_if_set_tx_power(int power);

#ifdef __cplusplus
}
#endif
