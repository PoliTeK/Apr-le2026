#include "daisy_seed.h"
#include "daisysp.h" 


#define LDR_NUM 6 
#define ADC_NUM 1 
#define V0 60      // Nota MIDI più bassa (C3)

using namespace daisy;
using namespace daisysp;

DaisySeed       hw;
MidiUsbHandler  midi;
AdcChannelConfig adcConfig[ADC_NUM + LDR_NUM]; 


GPIO button;
//GPIO leds[6]; 

// --- FUNZIONI MIDI PERSONALIZZATE (Risolvono l'errore undefined reference) ---
void SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint8_t msg[3];
    msg[0] = 0x90 | (channel & 0x0F); 
    msg[1] = note & 0x7F;             
    msg[2] = velocity & 0x7F;         
    midi.SendMessage(msg, 3);
}

void SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint8_t msg[3];
    msg[0] = 0x80 | (channel & 0x0F); 
    msg[1] = note & 0x7F;
    msg[2] = velocity;                       
    midi.SendMessage(msg, 3);
}

void InitGPIO() {
    for (int i = 0; i < LDR_NUM; i++) {
        adcConfig[i].InitSingle(hw.GetPin(15 + i));
    }
    adcConfig[6].InitSingle(hw.GetPin(21)); 

    hw.adc.Init(adcConfig, LDR_NUM + ADC_NUM, GPIO::Pull::PULLUP);
    hw.adc.Start();
    //hw.StartLog(false);

    button.Init(daisy::seed::D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    for (int i = 0; i < 6; i++) {
        //leds[i].Init(hw.GetPin(1 + i), GPIO::Mode::OUTPUT, GPIO::Pull::PULLUP);
    }
}

int main(void)
{
    hw.Configure();
    hw.Init();
    InitGPIO();
    
    MidiUsbHandler::Config midi_cfg;
    midi.Init(midi_cfg);


    uint8_t button_cnt = 0;
    float   pot_value = 0;
    float   thresholds[6] = {0};
    float   intorno = 20;
    float   ldr[LDR_NUM];
    bool    last_button_state = true;
    float   ldr_mean = 0;

    // --- SETUP MODE ---
    while(button_cnt < 7) 
    {
        bool current_button_state = button.Read();
        if (current_button_state == false && last_button_state == true) {
            button_cnt++;
            //hw.PrintLine("Setup Phase: %d", button_cnt);
            hw.DelayMs(50); 
        }
        last_button_state = current_button_state;

        pot_value = hw.adc.Get(6);
        for (int i = 0; i < LDR_NUM; i++) {
            ldr[i] = hw.adc.Get(i)/65535 * 127;
            System::DelayUs(50); 
        }


        if (button_cnt == 0) {
           //Ciao
        } 
        else if (button_cnt >= 1 && button_cnt <= 6) {
            int i = button_cnt - 1;
            thresholds[i] =  (pot_value / 65535.0f) * 127.0f;
            
            if (ldr[i]  > thresholds[i]) {
                SendNoteOn(0,0,thresholds[i]);
                SendNoteOn(1,0,ldr[i]);
            } else {
                SendNoteOff(0,0,thresholds[i]);
                SendNoteOff(1,0,ldr[i]);
            }
            //hw.PrintLine("Setup LDR %d - Soglia: %f - value: %f", i, thresholds[i], ldr[i]);
        }
        hw.DelayMs(50);
    }

    //for(int i=0; i<6; i++) leds[i].Write(false);
    bool is_playing[LDR_NUM] = {false}; 
    //hw.PrintLine("Setup completato. Entro in PLAY MODE.");

    // --- PLAY MODE ---
    while(1)
    {
        for (int i = 0; i < LDR_NUM; i++) {
            ldr[i] = (hw.adc.Get(i)/65535 * 127);
            System::DelayUs(50); 

            if (ldr[i] > thresholds[i] && is_playing[i] == false) {
                is_playing[i] = true; 
                //leds[i].Write(true);
                // CHIAMATA CORRETTA
                //hw.PrintLine("#%d toccato con valore %f", i, ldr[i]);
                SendNoteOn(0, V0 + i, 100); 
            }
            else if (ldr[i] < (thresholds[i] ) && is_playing[i] == true) {
                is_playing[i] = false; 
                //leds[i].Write(false);
                // CHIAMATA CORRETTA
                SendNoteOff(0, V0 + i, 0);
                //hw.PrintLine("#%d lasciato ", i);
            }
        }
        //hw.PrintLine("%d, %d, %d, %d, %d, %d", ldr[0], ldr[1], ldr[2], ldr[3], ldr[4], ldr[5]);
        hw.DelayMs(5); 
    }
}