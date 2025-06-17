/**********************************************************************************************************************
 * \file Gateway.h
 * \copyright Copyright (C) Infineon Technologies AG 2024
 *
 * \brief Header file for CAN to Ethernet Gateway functionality.
 *********************************************************************************************************************/
#ifndef GATEWAY_H_
#define GATEWAY_H_

#include "Ifx_Types.h"

#define RX_ETH_LED_PORT        &MODULE_P33
#define RX_ETH_LED_PIN         9
/**
 * \brief Initializes the gateway.
 */
void initGateway(void);

/**
 * \brief Runs the gateway logic. Should be called periodically in the main loop.
 */
void runGateway(void);

/**
 * \Initializes RX ETH LED
 */
void initRXETHLed(void);

#endif /* GATEWAY_H_ */
