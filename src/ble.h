/*******************************************************************************
 * @file        ble.h
 * @brief       BLE Header File
 * @author      Amey More, Amey.More@colorado.edu
 * @due         Oct 24, 2022
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
 *
 * @assignment  ecen5823-assignment9-ameyflash
 * @due         Oct 28, 2022
 *
 ******************************************************************************/

#ifndef BLE_H
#define BLE_H

#include "sl_bluetooth.h"
#include "scheduler.h"

#define UINT8_TO_BITSTREAM(p, n)  { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n)  { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                      *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
#define UINT32_TO_FLOAT(m, e)     (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

#define QUEUE_DEPTH      (16)

typedef struct {

  uint16_t  charHandle;     // Char handle from gatt_db.h
  size_t    bufferLength;   // Length of buffer in bytes to send
  uint32_t  bufferData;      // The actual data buffer for the indication.
                            // Need space for HTM (5 bytes) and button_state (2 bytes)
                            // indications, array [0] holds the flags byte.

} queue_struct_t;

// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition

typedef struct {
// values that are common to servers and clients
bd_addr myAddress;
uint8_t myAddress_type;

// values unique for server
// The advertising set handle allocated from Bluetooth stack.
uint8_t   advertisingSetHandle;
uint32_t  Advertising_min;
uint32_t  Advertising_max;
uint16_t  Advertising_duration;
uint8_t   Advertising_maxevents;

uint8_t   connectionHandle;
bool      connectionFlag;
uint16_t  min_interval;
uint16_t  max_interval;
uint16_t  latency;
uint16_t  timeout;
uint16_t  min_ce_length;
uint16_t  max_ce_length;

uint8_t   gattServerConnectionHandle;
uint8_t   gattServerStatusFlags;
uint16_t  gattServerClientConfigFlags;
bool      TemperatureIndicationFlag;
bool      InFlightFlag;

uint8_t   BondingFlag;
bool      ButtonIndicationFlag;

// values unique for client
uint16_t  scanInterval;
uint16_t  scanWindow;
uint32_t  HTMServiceHandle;
uint16_t  HTMCharacteristicsHandle;
int32_t   TemperatureValue;

bool      stateConnected;
bool      stateDisconnected;
bool      stateCompleted;
bool      stateCharacteristics;

uint32_t  ButtonServiceHandle;
uint16_t  ButtonCharacteristicsHandle;

} ble_data_struct_t;


static const uint8_t thermo_service[2] = { 0x09, 0x18 };
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

// Button Service UUID - 00000001-38c8-433e-87ec-652a2d136289
static const uint8_t button_service[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00 };
// Button Characteristics UUID - 00000002-38c8-433e-87ec-652a2d136289
static const uint8_t button_char[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00 };

// Function Prototypes
ble_data_struct_t *getbleData();
void handle_ble_event(sl_bt_msg_t *evt);
void print_temp_bt(uint32_t temperature);
void print_button_state_bt(uint8_t button_state);
void handle_server_events(sl_bt_msg_t *evt);
void handle_client_events(sl_bt_msg_t *evt);

bool     write_queue (uint16_t charHandle, size_t bufferLength, uint32_t bufferData);
bool     read_queue (uint16_t *charHandle, size_t *bufferLength, uint32_t *bufferData);
void     get_queue_status (uint32_t *_wptr, uint32_t *_rptr, bool *_full, bool *_empty);
uint32_t get_queue_depth (void);

#endif    //    BLE_H
