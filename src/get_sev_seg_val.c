#include "pico/stdlib.h"

uint16_t get_sev_seg_val(uint16_t num) {
    uint16_t sev_seg_val = 0;
        switch (num){
            case 0:
                sev_seg_val = 0x3F;
                break;
            case 1:
                sev_seg_val = 0x06;
                break;
            case 2:
                sev_seg_val = 0x5B;
                break;
            case 3:
                sev_seg_val = 0x4F;
                break;
            case 4:
                sev_seg_val = 0x66;
                break;
            case 5:
                sev_seg_val = 0x6D;
                break;
            case 6:
                sev_seg_val = 0x7D;
                break;
            case 7:
                sev_seg_val = 0x07;
                break;
            case 8:
                sev_seg_val = 0x7F;
                break;
            case 9:
                sev_seg_val = 0x67;
                break;
            default:
                sev_seg_val = 0x40; //7 segment value for a dash (only middle line lit up) to know something isnt working properly, as this condition should never be hit
        }
    return(sev_seg_val);
}