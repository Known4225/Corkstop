#define F_CPU 3333333

#include "tca.h"
#include <avr/interrupt.h>

ECODE tca_init() {
    /* enable overflow interrupt */
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;

    /* set Normal mode */
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;

    /* disable event counting */
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);

    /* set the period */
    TCA0.SINGLE.PER = 13020; // one second

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc /* set clock
    source (sys_clk/256) */
    | TCA_SINGLE_ENABLE_bm; /* start timer */
    return ECODE_OK;
}