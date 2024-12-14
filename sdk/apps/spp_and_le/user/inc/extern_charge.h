#ifndef __EXTERN_CHARGE_H__
#define __EXTERN_CHARGE_H__



#define TCFG_VBAT_5V_WAKE           IO_PORTB_05
#define TCFG_FULL_PORT              IO_PORTA_10


void ex_charge_printf(void);
void full_vbat_detect_handle(void);
void ex_charge_full_detect_disable(void);





#endif


