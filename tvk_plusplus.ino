// see tutorial at http://learn.adafruit.com/trinket-usb-volume-knob

#include "TrinketHidCombo.h"

#define PIN_ENCODER_A      0
#define PIN_ENCODER_B      2
#define TRINKET_PINx       PINB
#define PIN_ENCODER_SWITCH 1

static uint8_t enc_prev_pos    = 0;
static uint8_t enc_flags       = 0;
static uint8_t sw_was_pressed  = 0;

uint8_t current_set[4];
uint8_t current_set_index = 0;

#define MMKEY   0x00
#define KEYCODE 0x01
#define SCROLL  0x02 // mouse scoll wheel

#define ROTATE_LEFT  2
#define ROTATE_RIGHT 1
#define BUTTON_PRESS 3

//typedef struct {
//  uint8_t event_type;
//  uint8_t event_code;
//}  event;
//
//event control_sets[] = { 
//  { MOUSEWHEEL, +5 }, { MOUSEWHEEL, -5 }, { KEYCODE, KEYCODE_HOME }
//};

const unsigned char control_sets[] = {
  SCROLL,  +5,                    -5,                    0,
  MMKEY,   MMKEY_VOL_UP,          MMKEY_VOL_DOWN,        MMKEY_MUTE,
  MMKEY,   MMKEY_SCAN_NEXT_TRACK, MMKEY_SCAN_PREV_TRACK, MMKEY_PLAYPAUSE,
  KEYCODE, KEYCODE_PAGE_DOWN,     KEYCODE_PAGE_UP,       KEYCODE_HOME,
  KEYCODE, KEYCODE_ARROW_DOWN,    KEYCODE_ARROW_UP,      KEYCODE_HOME
};

void setup()
{
  delay(1000); // brick protection
  
  change_command_set(current_set_index); // Set initial command set
  
  // set pins as input with internal pull-up resistors enabled
  pinMode(PIN_ENCODER_A, INPUT);
  pinMode(PIN_ENCODER_B, INPUT);
  digitalWrite(PIN_ENCODER_A, HIGH);
  digitalWrite(PIN_ENCODER_B, HIGH);

  pinMode(PIN_ENCODER_SWITCH, INPUT);
  // the switch is active-high, not active-low
  // since it shares the pin with Trinket's built-in LED
  // the LED acts as a pull-down resistor
  digitalWrite(PIN_ENCODER_SWITCH, LOW);

  TrinketHidCombo.begin(); // start the USB device engine and enumerate

  // get an initial reading on the encoder pins
  if (digitalRead(PIN_ENCODER_A) == LOW) {
    enc_prev_pos |= (1 << 0);
  }
  if (digitalRead(PIN_ENCODER_B) == LOW) {
    enc_prev_pos |= (1 << 1);
  }
}

void change_command_set(uint8_t set_index) {
  for(uint8_t j=0;j<4;j++) { current_set[j] = control_sets[(current_set_index*4)+j]; }
}

uint8_t next_set_index(uint8_t set_index, uint8_t maximum) {
  set_index++;
  if ((set_index+1) > maximum) { set_index = 0; }
  return set_index;
}

void send_key(uint8_t action) {
  if (current_set[0] == KEYCODE) {
    TrinketHidCombo.pressKey(0,current_set[action]);
    TrinketHidCombo.pressKey(0, 0);
  } else if (current_set[0] == MMKEY) { 
    TrinketHidCombo.pressMultimediaKey(current_set[action]);
  } else if (current_set[0] == SCROLL) {
    TrinketHidCombo.mouseMove(0, 0, current_set[action], 0);
  }
}

void loop()
{
  int8_t enc_action = 0; // 1 or -1 if moved, sign is direction

  // note: for better performance, the code will now use
  // direct port access techniques
  // http://www.arduino.cc/en/Reference/PortManipulation
  uint8_t enc_cur_pos = 0;
  // read in the encoder state first
  if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_A)) {
    enc_cur_pos |= (1 << 0);
  }
  if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_B)) {
    enc_cur_pos |= (1 << 1);
  }

  // if any rotation at all
  if (enc_cur_pos != enc_prev_pos)
  {
    if (enc_prev_pos == 0x00)
    {
      // this is the first edge
      if (enc_cur_pos == 0x01) {
        enc_flags |= (1 << 0);
      }
      else if (enc_cur_pos == 0x02) {
        enc_flags |= (1 << 1);
      }
    }

    if (enc_cur_pos == 0x03)
    {
      // this is when the encoder is in the middle of a "step"
      enc_flags |= (1 << 4);
    }
    else if (enc_cur_pos == 0x00)
    {
      // this is the final edge
      if (enc_prev_pos == 0x02) {
        enc_flags |= (1 << 2);
      }
      else if (enc_prev_pos == 0x01) {
        enc_flags |= (1 << 3);
      }

      // check the first and last edge
      // or maybe one edge is missing, if missing then require the middle state
      // this will reject bounces and false movements
      if (bit_is_set(enc_flags, 0) && (bit_is_set(enc_flags, 2) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;
      }
      else if (bit_is_set(enc_flags, 2) && (bit_is_set(enc_flags, 0) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;
      }
      else if (bit_is_set(enc_flags, 1) && (bit_is_set(enc_flags, 3) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;
      }
      else if (bit_is_set(enc_flags, 3) && (bit_is_set(enc_flags, 1) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;
      }

      enc_flags = 0; // reset for next time
    }
  }

  enc_prev_pos = enc_cur_pos;

  if (enc_action > 0) {
      send_key(ROTATE_RIGHT);
//    TrinketHidCombo.pressMultimediaKey(KEYCODE_ARROW_DOWN);
  }
  else if (enc_action < 0) {
      send_key(ROTATE_LEFT);
//    TrinketHidCombo.pressMultimediaKey(KEYCODE_ARROW_UP);
  }

  // remember that the switch is active-high
  if (bit_is_set(TRINKET_PINx, PIN_ENCODER_SWITCH)) 
  {
    if (sw_was_pressed == 0) // only on initial press, so the keystroke is not repeated while the button is held down
    {
//      TrinketHidCombo.pressMultimediaKey(KEYCODE_HOME);
// XXX FIXME
// eventually I want to have a long-press change the command set and leave the button press function as before.
current_set_index = next_set_index(current_set_index, (sizeof(control_sets) / 4));
change_command_set(current_set_index);
//      sendKey(BUTTON_PRESS);

      delay(5); // debounce delay
    }
    sw_was_pressed = 1;
  }
  else
  {
    if (sw_was_pressed != 0) {
      delay(5); // debounce delay
    }
    sw_was_pressed = 0;
  }

  TrinketHidCombo.poll(); // check if USB needs anything done
}
