#ifndef __STEP24_H__
#define __STEP24_H__

#define STEP24_STEP_SHIFT  1
#define STEP24_STEP_MAX    23
#define STEP24_INDEX_BITS  4
#define STEP24_INDEX_MASK  0xF
#define STEP24_AMP_SHIFT   2
#define STEP24_AMP_MAX     61
#define STEP24_VALUE_MASK  0x3
#define STEP24_VALUE_MINUS 0x4
#define STEP24_VALUE_ZERO  0x8

#define STEP24_NEUTRAL 103

/******************************************************************************/
const char STEP24_VALUE[248] = {
	0, 0, 0, 0,
	3, 6, 7, 8,
	4, 7, 9, 10,
	5, 9, 11, 12,
	6, 10, 13, 14,
	7, 12, 15, 16,
	7, 13, 17, 18,
	8, 15, 18, 20,
	9, 16, 20, 22,
	10, 18, 22, 24,
	11, 19, 24, 26,
	11, 20, 26, 28,
	12, 22, 28, 30,
	13, 23, 30, 32,
	14, 25, 31, 34,
	15, 26, 33, 36,
	15, 28, 35, 38,
	16, 29, 37, 40,
	17, 31, 39, 42,
	18, 32, 41, 44,
	19, 34, 42, 46,
	20, 35, 44, 48,
	20, 36, 46, 50,
	21, 38, 48, 52,
	22, 39, 50, 54,
	23, 41, 52, 56,
	24, 42, 53, 58,
	24, 44, 55, 60,
	25, 45, 57, 62,
	26, 47, 59, 64,
	27, 48, 61, 66,
	28, 50, 63, 68,
	28, 51, 65, 70,
	29, 53, 66, 72,
	30, 54, 68, 74,
	31, 55, 70, 76,
	32, 57, 72, 78,
	33, 58, 74, 80,
	33, 60, 76, 82,
	34, 61, 77, 84,
	35, 63, 79, 86,
	36, 64, 81, 88,
	37, 66, 83, 90,
	37, 67, 85, 92,
	38, 69, 87, 94,
	39, 70, 89, 96,
	40, 71, 90, 98,
	41, 73, 92, 99,
	42, 74, 94, 101,
	42, 76, 96, 103,
	43, 77, 98, 105,
	44, 79, 100, 107,
	45, 80, 101, 109,
	46, 82, 103, 111,
	46, 83, 105, 113,
	47, 85, 107, 115,
	48, 86, 109, 117,
	49, 88, 111, 119,
	50, 89, 112, 121,
	50, 90, 114, 123,
	51, 92, 116, 125,
	52, 93, 118, 127
};
const char STEP24_INDEX[48] = {
    0x1, 0x71,
    0x2, 0x70,
    0x3, 0x78,
    0x3, 0x64,
    0x3, 0x55,
    0x3, 0x46,
    0x3, 0x87,
    0x2, 0x07,
    0x1, 0x17,
    0x0, 0x27,
    0x8, 0x37,
    0x4, 0x36,
    0x5, 0x35,
    0x6, 0x34,
    0x7, 0x38,
    0x7, 0x20,
    0x7, 0x11,
    0x7, 0x02,
    0x7, 0x83,
    0x6, 0x43,
    0x5, 0x53,
    0x4, 0x63,
    0x8, 0x73,
    0x0, 0x72
};

/******************************************************************************/
char step24_duty_u = 128;
char step24_duty_v = 128;
char step24_duty_w = 128;
char step24_phase = 0;

unsigned short step24_velocity = 0;
unsigned short step24_phase_sum = 0;

/******************************************************************************/
#define STEP24_SET_VELOCITY (step24_velocity = step24_phase_sum, step24_phase_sum = 0)

inline void
step24_set_duty(char amp, char step) {
    const char *p_idx = STEP24_INDEX + (char)(step << STEP24_STEP_SHIFT);
    char idx_u = *p_idx;
    char idx_v = *++p_idx;
    char idx_w = idx_v >> STEP24_INDEX_BITS;
    idx_v &= STEP24_INDEX_MASK;

    const char *p_value = STEP24_VALUE + (char)(amp << STEP24_AMP_SHIFT);
    step24_duty_u = p_value[idx_u & STEP24_VALUE_MASK];
    step24_duty_v = p_value[idx_v & STEP24_VALUE_MASK];
    step24_duty_w = p_value[idx_w & STEP24_VALUE_MASK];

    if (idx_u & STEP24_VALUE_ZERO) {
        step24_duty_u = 0;
    }
    if (idx_v & STEP24_VALUE_ZERO) {
        step24_duty_v = 0;
    }
    if (idx_w & STEP24_VALUE_ZERO) {
        step24_duty_w = 0;
    }

    if (idx_u & STEP24_VALUE_MINUS) {
        step24_duty_u = 128 - step24_duty_u;
    } else {
        step24_duty_u = 128 + step24_duty_u;
    }
    if (idx_v & STEP24_VALUE_MINUS) {
        step24_duty_v = 128 - step24_duty_v;
    } else {
        step24_duty_v = 128 + step24_duty_v;
    }
    if (idx_w & STEP24_VALUE_MINUS) {
        step24_duty_w = 128 - step24_duty_w;
    } else {
        step24_duty_w = 128 + step24_duty_w;
    }
}

inline void
step24_set_phase(char sens_u, char sens_v) {
    /* u相,v相からw相を得る
     * オーバーフロー対策のため範囲(0-128-255)を(64-128-191)に変換 */
    char wave_u = sens_u >> 1;
    char wave_v = sens_v >> 1;
    char wave_w = 255 - wave_u;
    wave_w -= wave_v;
    wave_v += 64;
    wave_u += 64;

    /* u相,v相,w相の各相に対して
     * adv: 1/48周期進んだ相と
     * del: 1/48周期遅れた相を作る */
    char wave_z, temp;
    wave_z = wave_v >> 1, wave_z >>= 1;
    temp = wave_z >> 1, temp >>= 1;
    wave_z -= temp;
    char u_del = wave_u - wave_z;
    char w_adv = wave_w - wave_z;
    wave_z = wave_w >> 1, wave_z >>= 1;
    temp = wave_z >> 1, temp >>= 1;
    wave_z -= temp;
    char v_del = wave_v - wave_z;
    char u_adv = wave_u - wave_z;
    wave_z = wave_u >> 1, wave_z >>= 1;
    temp = wave_z >> 1, temp >>= 1;
    wave_z -= temp;
    char w_del = wave_w - wave_z;
    char v_adv = wave_v - wave_z;

    /* 1/24周期単位の位相を得る */
    char detected_phase;
    if (STEP24_NEUTRAL < u_del) {
        if (u_adv <= STEP24_NEUTRAL) {
            detected_phase = 12;
        }
        if (u_adv < v_adv) {
            detected_phase = 1;
        }
    } else {
        if (STEP24_NEUTRAL < u_adv) {
            detected_phase = 0;
        }
        if (v_adv <= u_adv) {
            detected_phase = 13;
        }
    }
    if (STEP24_NEUTRAL < v_del) {
        if (v_adv <= STEP24_NEUTRAL) {
            detected_phase = 4;
        }
        if (v_adv < w_adv) {
            detected_phase = 17;
        }
    } else {
        if (STEP24_NEUTRAL < v_adv) {
            detected_phase = 16;
        }
        if (w_adv <= v_adv) {
            detected_phase = 5;
        }
    }
    if (STEP24_NEUTRAL < w_del) {
        if (w_adv <= STEP24_NEUTRAL) {
            detected_phase = 20;
        }
        if (w_adv < u_adv) {
            detected_phase = 9;
        }
    } else {
        if (STEP24_NEUTRAL < w_adv) {
            detected_phase = 8;
        }
        if (u_adv <= w_adv) {
            detected_phase = 21;
        }
    }
    if (u_del < w_del) {
        if (STEP24_NEUTRAL < u_adv) {
            detected_phase = 11;
        }
        if (w_adv < u_adv) {
            detected_phase = 22;
        }
    } else {
        if (u_adv <= STEP24_NEUTRAL) {
            detected_phase = 23;
        }
        if (u_adv <= w_adv) {
            detected_phase = 10;
        }
    }
    if (v_del < u_del) {
        if (STEP24_NEUTRAL < v_adv) {
            detected_phase = 3;
        }
        if (u_adv < v_adv) {
            detected_phase = 14;
        }
    } else {
        if (v_adv <= STEP24_NEUTRAL) {
            detected_phase = 15;
        }
        if (v_adv <= u_adv) {
            detected_phase = 2;
        }
    }
    if (w_del < v_del) {
        if (STEP24_NEUTRAL < w_adv) {
            detected_phase = 19;
        }
        if (v_adv < w_adv) {
            detected_phase = 6;
        }
    } else {
        if (w_adv <= STEP24_NEUTRAL) {
            detected_phase = 7;
        }
        if (w_adv <= v_adv) {
            detected_phase = 18;
        }
    }

    /* 位相変化を積算 */
    char phase_diff;
    if (detected_phase < step24_phase) {
        phase_diff = 24;
    } else {
        phase_diff = 0;
    }
    phase_diff += detected_phase;
    phase_diff -= step24_phase;
    if (phase_diff >= 12) {
        phase_diff = 0;
    }
    step24_phase_sum += phase_diff;
    step24_phase = detected_phase;
}

#endif /* __STEP24_H__ */
