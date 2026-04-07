#include "daisy_seed.h"

using namespace daisy;

DaisySeed         hw;
AdcChannelConfig  adcConfig[6]; 

int main(void)
{
    hw.Configure();
    hw.Init();

    // Cambia in 'true' per aspettare l'apertura del monitor seriale sul PC
    hw.StartLog(true); 

    for (int i = 0; i < 6; i++) {
        adcConfig[i].InitSingle(hw.GetPin(15 + i));
    }
    hw.adc.Init(adcConfig, 6);
    hw.adc.Start();

    while(1)
    {
        float ldr0 = hw.adc.GetFloat(0)*1000;
        float ldr1 = hw.adc.GetFloat(1)*1000;
        float ldr2 = hw.adc.GetFloat(2)*1000;
        float ldr3 = hw.adc.GetFloat(3)*1000;
        float ldr4 = hw.adc.GetFloat(4)*1000;
        float ldr5 = hw.adc.GetFloat(5)*1000;

        hw.PrintLine("LDR1:%f, LDR2:%f, LDR3:%f, LDR4:%f, LDR5:%f, LDR6:%f", 
                      ldr0, ldr1, ldr2, ldr3, ldr4, ldr5);

        
        System::Delay(100); 
    }
}