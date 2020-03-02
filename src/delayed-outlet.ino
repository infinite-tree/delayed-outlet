#include <Arduino.h>
#include <TM1637Display.h>

// Set the CLK and DIO pins for the display
#define CLK     2
#define DIO    3

// Pins for the encoder
#define ENC_DATA    8
#define ENC_CLK     9
#define ENC_BUTTON  10

// Relay PIN
#define RELAY       12



int enc_value = 0; 
int minutes = 0;
int last_state = 0;
int state = 0;
bool running = false;
unsigned long start_time = 0;
unsigned long current_time = 0;
unsigned long run_time = 0;
const unsigned long ONE_MINUTE = 60000;
const unsigned long FLASH_DELAY = 500;

//set up the 4-Digit Display.
TM1637Display display(CLK, DIO);
 
void setup()
{
    // Enable the encoder pins
    pinMode (ENC_CLK, INPUT_PULLUP);
    pinMode (ENC_DATA, INPUT_PULLUP);
    pinMode(ENC_BUTTON, INPUT_PULLUP);

    // Enable the RELAY
    pinMode(RELAY, OUTPUT);
    digitalWrite(RELAY, LOW);

    //set the diplay to maximum brightness
    display.setBrightness(0x0a);
    last_state = digitalRead(ENC_CLK);
    display.showNumberDec(minutes);
}
 
 
void loop()
{

    if (!running) {
        //
        // Handle Encoder Readings
        //
        state = digitalRead(ENC_CLK);    
        if (state != last_state) {
            if (digitalRead(ENC_DATA) != state) { 
                enc_value++;
            } else {
                enc_value--;
            }
            if (enc_value < 0) {
                enc_value = 0;
            }
            minutes = enc_value/2;
            if (enc_value % 2 == 0) {
                display.showNumberDec(minutes);
            }
        }
        last_state = state;
    } else {
        //
        // Handle timer
        //
        if (millis() - current_time > FLASH_DELAY) {
            display.clear();
        }
        if (millis() - current_time > FLASH_DELAY*2) {
            display.showNumberDec(minutes);
            current_time = millis();

            //
            // Update the time
            //
            unsigned long diff = current_time - start_time;
            unsigned long remaining = run_time + ONE_MINUTE - diff;
            minutes = int(remaining/ONE_MINUTE);
            display.showNumberDec(minutes);

            if (diff >= run_time) {
                running = false;
                digitalWrite(RELAY, LOW);
                enc_value = 0;
                minutes = 0;
                display.showNumberDec(minutes);
            }
        }
    }

    //
    // Handle Button Press
    //
    // There is too much line noise for external interrupts,
    // but this is surprisingly stable. All 3 consecutive
    // reads must be LOW for the button to register pressed.
    bool pressed = true;
    for (uint8_t x=0; x<3;x++) {
        if (digitalRead(ENC_BUTTON) == HIGH) {
            pressed = false;
            break;
        }
        delay(2);
    }
    if (pressed) {
        if (running) {
            // 
            // Stop the countdown
            // 
            running = false;
            minutes = 0;
            digitalWrite(RELAY, LOW);
            display.showNumberDec(minutes);
            return;
        }

        // Don't start on 0
        if (!running && minutes == 0) {
            return;
        }

        // 
        // Start the Timer
        // 
        display.clear();
        delay(250);
        display.showNumberDec(minutes);
        delay(250);
        display.clear();
        delay(250);
        display.showNumberDec(minutes);
        delay(250);
        display.clear();
        delay(250);
        display.showNumberDec(minutes);
        delay(250);
        display.clear();
        delay(1000);
        display.showNumberDec(minutes);
        delay(250);

        digitalWrite(RELAY, HIGH);
        running = true;
        enc_value = 0;
        start_time = millis();
        run_time = minutes * ONE_MINUTE;
        current_time = start_time;
        display.showNumberDec(minutes);
    }
}