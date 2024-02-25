#include "config.h"
#include "step24.h"

void __interrupt()
isr() {
    GIE = 0;
    if (TMR1IF) {
        DEBUG_CYCLE_TMR1;
        STEP24_SET_VELOCITY;
        TMR1_INIT;
    }
    GIE = 1;
}

void
main(void) {
    setup();

    char step = 0;
    char ii = 0;

    while(1) {
        CHS0 = 0;            // AN0から読込む
        GO_nDONE = 1;        // ｱﾅﾛｸﾞ値読取り開始を指示
        while(GO_nDONE);     // 読取り完了するまで待つ
        char adc_u = ADRESH; // PICは読取った値をADRESHとADRESLのレジスターにセットする

        CHS0 = 1;            // AN1から読込む
        GO_nDONE = 1;
        while(GO_nDONE);
        char adc_v = ADRESH;

        step24_set_phase(adc_u, adc_v);

        step24_set_duty(61, step);
        PWM1DCH = step24_duty_u;
        PWM2DCH = step24_duty_v;
        PWM3DCH = step24_duty_w;

        DEBUG_CYCLE_SENS;
        ii++;
        if(ii>=200) {
            ii = 0;
            step++;
            step %= 24;
        }
    }

}
