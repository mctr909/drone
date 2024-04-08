#include <xc.h>

#define _XTAL_FREQ 16000000    // delay用に必要(ｸﾛｯｸ16MHzを指定)

/*** ｺﾝﾌｨｸﾞﾚｰｼｮﾝ1の設定 ***/
#pragma config FOSC     = INTOSC // 内部ｸﾛｯｸを使用する(INTOSC)
#pragma config WDTE     = OFF    // ｳｵｯﾁﾄﾞｯｸﾞﾀｲﾏｰ(OFF)
#pragma config PWRTE    = ON     // 電源ONから64ms後にﾌﾟﾛｸﾞﾗﾑを開始する(ON)
#pragma config MCLRE    = OFF    // RA3ﾋﾟﾝを外部ﾘｾｯﾄ信号として使用する(OFF)
#pragma config CP       = OFF    // ﾌﾟﾛｸﾞﾗﾑﾒﾓﾘｰを保護しない(OFF)
#pragma config BOREN    = ON     // 電源電圧降下常時監視機能(ON)
#pragma config CLKOUTEN = OFF    // RA4ﾋﾟﾝをCLKOUTﾋﾟﾝとして使用する(OFF)

/*** ｺﾝﾌｨｸﾞﾚｰｼｮﾝ2の設定 ***/
#pragma config WRT    = OFF // Flashﾒﾓﾘｰの保護(OFF)
#pragma config STVREN = ON  // ｽﾀｯｸがｵｰﾊﾞｰﾌﾛｰやｱﾝﾀﾞｰﾌﾛｰしたらﾘｾｯﾄをする(ON)
#pragma config BORV   = HI  // 電源電圧降下常時監視電圧(2.5V)設定(HI)
#pragma config LPBOR  = OFF // 低消費電力ﾌﾞﾗｳﾝｱｳﾄﾘｾｯﾄ(OFF)
#pragma config LVP    = OFF // 低電圧書き込み機能の使用(OFF)

/*** TMR1ｶｳﾝﾀの初期化 ***/
#define TMR1_INIT (TMR1H = 128, TMR1L = 0, TMR1IF = 0)

/*** 周期測定用のﾋﾟﾝを割り当て ***/
#define DEBUG_CYCLE_TMR1 (RC4 ^= 1)
#define DEBUG_CYCLE_SENS (RA5 ^= 1)

/**************** ﾋﾟﾝ配置 ****************
 *          Vdd |1     14| Vss
 *          RA5 |2     13| RA0 AN0
 *      AN3 RA4 |3     12| RA1 AN1
 *          RA3 |4     11| RA2 AN2 PWM3
 * PWM1     RC5 |5     10| RC0 AN4      I2C clock
 *          RC4 |6      9| RC1 AN5 PWM4 I2C data
 * PWM2 AN7 RC3 |7      8| RC2 AN6
 *****************************************/

inline void
setup() {
    /*** ｸﾛｯｸの設定 ***/
    OSCCONbits.SCS = 2;   // 内部ｸﾛｯｸを使用(1=T1OSC, 2=内部ｸﾛｯｸ)
    OSCCONbits.IRCF = 15; // 16MHzに設定
                          // 11=1MHz, 12=2MHz, 13=4MHz, 14=8MHz, 15=16MHz

    /*** ﾋﾟﾝの入出力設定 ***/
    ANSELA = 0b00000011; // ｱﾅﾛｸﾞはAN0,AN1を使用し、残りをすべてﾃﾞｼﾞﾀﾙI/Oに割当
    ANSELC = 0b00000000;
    TRISA = 0b00000011;  // RA0ﾋﾟﾝ,RA1ﾋﾟﾝだけ入力、その他のﾋﾟﾝは出力に割当てる(RA3は入力専用)
    TRISC = 0b00000000;
    PORTA = 0b00000000;  // 出力ﾋﾟﾝの初期化(全てLOWにする)
    PORTC = 0b00000000;

    /*** AD変換の設定 ***/
    ADCON0bits.ADON = 1;   // AD変換有効化
    ADCON1bits.ADFM = 1;   // 読取値は右詰(0=左詰, 1=右詰)
    ADCON1bits.ADPREF = 0; // ﾘﾌｧﾚﾝｽはVDD(0=Vdd, 2=外部Pin, 3=内部基準電圧)
    ADCON1bits.ADCS = 6;   // A/D変換ｸﾛｯｸはFosc/64
                           // 0=Fosc/2, 1=Fosc/8, 2=Fosc/32, 3=FRC
                           // 4=Fosc/4, 5=Fosc/16, 6=Fosc/64, 7=FRC

    /*** PWM1の設定(RC5ﾋﾟﾝ) ***/
    PWM1CON = 0b11000000; // PWM機能を使用する(output is active-high)
    PWM1DCH = 0; // duty MSB8bit初期化
    PWM1DCL = 0; // duty LSB2bit初期化

    /*** PWM2の設定(RC3ﾋﾟﾝ) ***/
    PWM2CON = 0b11000000; // PWM機能を使用する(output is active-high)
    PWM2DCH = 0; // duty MSB8bit初期化
    PWM2DCL = 0; // duty LSB2bit初期化

    /*** PWM3の設定(RA2ﾋﾟﾝ) ***/
    PWM3CON = 0b11000000; // PWM機能を使用する(output is active-high)
    PWM3DCH = 0; // duty MSB8bit初期化
    PWM3DCL = 0; // duty LSB2bit初期化

    /*** PWMｶｳﾝﾀの設定 ***/
    T2CONbits.T2CKPS = 1;   // TMR2ﾌﾟﾘｽｹｰﾗ値を1/4倍に設定(0=1/1, 1=1/4, 2=1/16, 3=1/64)
    TMR2 = 0;               // TMR2ｶｳﾝﾀの初期化
    PR2 = 255;              // PWMの周期を設定(PWMｷｬﾘｱ周波数=Fosc*TMR2ﾌﾟﾘｽｹｰﾗ値/(PR2+1))
    TMR2ON = 1;             // TMR2(PWM)ｽﾀｰﾄ

    /*** 割り込みﾀｲﾏの設定 ***/   
    T1CONbits.T1CKPS = 0;   // TMR1ﾌﾟﾘｽｹｰﾗ値を1/1倍に設定(0=1/1, 1=1/2, 2=1/4, 3=1/8)
    T1CONbits.TMR1CS = 0;   // ｸﾛｯｸはFosc/4(0=Fosc/4, 1=Fosc, 2=T1CKI or T1OSC, 3=CPSOSC)
    T1CONbits.T1OSCEN = 0;  // T1OSCをOFF
    PEIE   = 1;             // 周辺装置割り込み有効
    GIE    = 1;             // 全割込み処理を許可する
    TMR1IE = 1;             // TMR1割込みを許可する
    TMR1_INIT;              // TMR1ｶｳﾝﾀの初期化
    TMR1ON = 1;             // TMR1ｽﾀｰﾄ
}
