// Wokwi Custom Chip - For information and examples see:
// https://link.wokwi.com/custom-chips-alpha
//
// SPDX-License-Identifier: MIT
// Created by Gadul on 2023
// Updated by IoT Solution Development Team on 2026/11/03
//

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

const int     ADDRESS                     = 0x23;
const double  BH1750_LUX_TO_RAW_FACTOR    = 1.2;
const int     BYTE_ISOLATION_VALUE        = 256;
const int     BH1750_MAX_INTENSITY_IN_LUX = 10000;

typedef enum
{
  EXPECTING_HIGH_BYTE_STATE,
  EXPECTING_LOW_BYTE_STATE,
} States;

typedef struct {
  pin_t pin_int;
  uint32_t light_level_attr;
  States state;
} chip_state_t;


static bool     on_i2c_connect(void *user_data, uint32_t address, bool connect);
static uint8_t  on_i2c_read(void *user_data);
static bool     on_i2c_write(void *user_data, uint8_t data);
static void     on_i2c_disconnect(void *user_data);

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  chip->pin_int = pin_init("ADDR", INPUT_PULLUP);
  
  const i2c_config_t i2c_config = {
    .user_data = chip,
    .address = ADDRESS,
    .scl = pin_init("SCL", INPUT),
    .sda = pin_init("SDA", INPUT),
    .connect = on_i2c_connect,
    .read = on_i2c_read,
    .write = on_i2c_write,
    .disconnect = on_i2c_disconnect, // Optional
  };
  i2c_init(&i2c_config);

  // This attribute can be edited by the user. It's defined in wokwi-custom-part.json:
  chip->light_level_attr = attr_init("LightLevel", BH1750_MAX_INTENSITY_IN_LUX);
}

bool on_i2c_connect(void *user_data, uint32_t address, bool connect) {
  chip_state_t *chip = user_data;
  chip->state = EXPECTING_HIGH_BYTE_STATE;
  return true; /* Ack */
}

uint8_t on_i2c_read(void *user_data) {
  chip_state_t *chip = user_data;
  const uint32_t light_level = attr_read(chip->light_level_attr) * BH1750_LUX_TO_RAW_FACTOR;
  uint8_t level;
  switch (chip->state)
  {
    case EXPECTING_HIGH_BYTE_STATE:
      level = light_level / BYTE_ISOLATION_VALUE;
      chip->state = EXPECTING_LOW_BYTE_STATE;
      break;
    case EXPECTING_LOW_BYTE_STATE:
      level = light_level % BYTE_ISOLATION_VALUE;
      chip->state = EXPECTING_HIGH_BYTE_STATE;
      break;
  }
  return level;
}

bool on_i2c_write(void *user_data, uint8_t data) {
  chip_state_t *chip = user_data;
  return true; // Ack
}

void on_i2c_disconnect(void *user_data) {
  // Do nothing
}
