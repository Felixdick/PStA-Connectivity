#ifndef ETHERNET_EXAMPLE_H_
#define ETHERNET_EXAMPLE_H_

#include "Ifx_Types.h"
#include "IfxEth.h"
#include "IfxPort.h"

/* MAC address for this device */
#define ETH_MAC_ADDR                {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}

/* --- UDP Packet Configuration --- */
#define ETH_BOARD_IP_ADDR           {192, 168, 1, 100}
#define ETH_DEST_IP_ADDR            {192, 168, 1, 1}
#define ETH_SRC_PORT                60000
#define ETH_DEST_PORT               40000

/* Interrupt priorities */
#define ISR_PRIORITY_ETH            6
#define ISR_PRIORITY_BLINKY_TIMER   8

/* LED for Ethernet Link status */
#define ALIVE_LED_PORT              &MODULE_P33
#define ALIVE_LED_PIN               6

/* LED for Heartbeat */
#define BLINKY_LED_PORT             &MODULE_P33
#define BLINKY_LED_PIN              13

/* LED for successful TX */
#define TX_SUCCESS_LED_PORT         &MODULE_P33
#define TX_SUCCESS_LED_PIN          7

/* LED for TX buffer acquisition failure */
#define DEBUG_LED_PORT        &MODULE_P33
#define DEBUG_LED_PIN         8

typedef struct
{
    IfxEth driver; /* ETH driver handle */
} App_Eth;

void initEthernet(void);
void initAliveLed(void);
void initBlinkyLed(void);
void initBlinkyTimer(void);
void initTxSuccessLed(void);
void initBufferFailLed(void);
void runEthernetLogic(void);

/* --- New and modified functions for Gateway --- */
void sendEthernetPacket(const uint8* payload, uint16 payload_len);
uint16 receiveEthernetPacket(uint8* buffer, uint16 max_len);

#endif /* ETHERNET_EXAMPLE_H_ */
