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
GPIO leds[6]; // Array per gestire i LED in modo ciclico

void InitGPIO() {
    // Inizializzazione ADC per i 6 LDR (Pin 15-20)
    for (int i = 0; i < LDR_NUM; i++) {
        adcConfig[i].InitSingle(hw.GetPin(15 + i));
    }
    // ADC per il potenziometro (Pin 21)
    adcConfig[6].InitSingle(hw.GetPin(21)); 

    hw.adc.Init(adcConfig, LDR_NUM + ADC_NUM);
    hw.adc.Start();

    // Inizializzazione Pulsante (D0)
    button.Init(daisy::seed::D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    // Inizializzazione LED (D1-D6) tramite ciclo
    for (int i = 0; i < 6; i++) {
        leds[i].Init(hw.GetPin(1 + i), GPIO::Mode::OUTPUT, GPIO::Pull::PULLUP);
    }
}


int main(void)
{
    hw.Configure();
    hw.Init();
    InitGPIO();
    //MidiUsbHandler::Config midi_cfg;
    //midi.Init(midi_cfg);
    hw.StartLog(false);

    uint8_t button_cnt = 0;
    float   pot_value = 0;
    float   thresholds[6] = {0};
    float   intorno = 20;
    float   ldr[LDR_NUM];
    bool    last_button_state = true;
    float ldr_mean = 0;



    while(button_cnt < 7) 
    {
        
        bool current_button_state = button.Read();
        if (current_button_state == false && last_button_state == true) {
            button_cnt++;
            hw.PrintLine("Setup Phase: %d", button_cnt);
            hw.DelayMs(50); 
        }
        last_button_state = current_button_state;

        pot_value = hw.adc.Get(6);
        for (int i = 0; i < LDR_NUM; i++) {
            ldr[i] = hw.adc.Get(i);
            System::DelayUs(50); 
        }
        //ldr_mean = (ldr[0] + ldr[1] + ldr[2] + ldr[3] + ldr[4] + ldr[5]) / LDR_NUM;
        ldr_mean = (ldr[4] + ldr[5]) / 2.0f;
  
      
        

        if (button_cnt == 0) {
            // Intorno State
            intorno =  (pot_value / 65535.0f) * 500.0f;
            hw.PrintLine("Setup INTORNO: %f| ", intorno);
            hw.PrintLine("LDR mean: %f", ldr_mean);
            if (intorno < ldr_mean){
                for(int i=0; i<6; i++) leds[i].Write(true);
            } else {
                for(int i=0; i<6; i++) leds[i].Write(false);
            }
            
        } 
        else if (button_cnt >= 1 && button_cnt <= 6) {
            // Calibrazione singola soglia LDR
            int i = button_cnt - 1;
            thresholds[i] = intorno  + (pot_value / 65535.0f) * 150.0f;
            
            if (ldr[i] + 20 > thresholds[i]) {
                leds[i].Write(false);
            } else {
                leds[i].Write(true);
            }
            
            hw.PrintLine("Setup LDR %d - Soglia: %f - value: %f", i, thresholds[i], ldr[i]);
        }
        
        hw.DelayMs(50);
    }

    // --- PLAY MODE (Loop infinito) ---
    hw.PrintLine("Setup completato. Entro in PLAY MODE.");
    for(int i=0; i<6; i++) leds[i].Write(false);
 
    bool is_playing[LDR_NUM] = {false}; 

    while(1)
    {
        for (int i = 0; i < LDR_NUM; i++) {
            ldr[i] = hw.adc.Get(i);
            
            System::DelayUs(50); 

            // 2. Tocco rilevato E la nota NON stava già suonando
            if (ldr[i] > thresholds[i] && is_playing[i] == false) {
                
                is_playing[i] = true; 
                
                leds[i].Write(true);
                //midi.SendNoteOn(0, V0 + i, 100);
                
                hw.DelayMs(100); // Durata della nota
                
                //midi.SendNoteOff(0, V0 + i, 0);
                leds[i].Write(false);
            }
            else if (ldr[i] < (thresholds[i] - 15) && is_playing[i] == true) {
                
                is_playing[i] = false; // Riattiva il sensore per il prossimo tocco
                
            }
        }
        
        // Un delay minimo per non saturare la CPU, ma mantenere reattività
        hw.DelayMs(10); 
    }
}