#include <3ds.h>
#include "volume.h"

float Volume_ExtractVolume(int nul, int one, int slider)
{
    if(slider <= nul || one < nul)
        return 0;

    if(one == nul) //hardware returns 0 on divbyzero
        return (slider > one) ? 1.0F : 0;

    float ret = (float)(slider - nul) / (float)(one - nul);
    if((ret + (1 / 256.0F)) < 1.0F)
        return ret;
    else
        return 1.0F;
}

u32 Volume_GetCurrentValue(void)
{
    u8 volumeSlider[2], dspVolumeSlider[2];

    MCUHWC_ReadRegister(0x58, dspVolumeSlider, 2);  // Register-mapped ADC register
    MCUHWC_ReadRegister(0x27, volumeSlider + 0, 1); // Raw volume slider state
    MCUHWC_ReadRegister(0x09, volumeSlider + 1, 1); // Volume slider state

    float coe = Volume_ExtractVolume(dspVolumeSlider[0], dspVolumeSlider[1], volumeSlider[0]);
    return (u32)((coe * 100.0F) + (1 / 256.0F));
}