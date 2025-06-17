/**********************************************************************************************************************
 * \file Gateway.c
 * \copyright Copyright (C) Infineon Technologies AG 2024
 *
 * \brief Source file for CAN to Ethernet Gateway functionality.
 * This file implements the logic for forwarding messages between the CAN and Ethernet peripherals
 * according to the specified UDP packet structure.
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "Gateway.h"
#include "MULTICAN.h"
#include "Ethernet_Example.h"
#include "Ifx_reg.h"

/*********************************************************************************************************************/
/*-------------------------------------------------Local Prototypes--------------------------------------------------*/
/*********************************************************************************************************************/
void forwardCanToEthernet(void);
void forwardEthernetToCan(void);

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/

void initGateway(void)
{
    /* No gateway-specific initialization needed at the moment */
}

void runGateway(void)
{
    forwardCanToEthernet();
    forwardEthernetToCan();
}

void forwardCanToEthernet(void)
{
    IfxMultican_Message rxMsg;
    /* Get a handle to the high-level CAN message object driver */
    IfxMultican_Can_MsgObj* rxMsgObjHandle = &g_multican.canRxMsgObj;

    /* Check if the dedicated RX message object has a new message */
    if (IfxMultican_Can_MsgObj_isRxPending(rxMsgObjHandle))
    {
        /* Read the message. Note: We get a pointer to the low-level HW registers
         * to check properties not available in the high-level IfxMultican_Message struct. */
        Ifx_CAN_MO* rxMo = IfxMultican_MsgObj_getPointer(g_multican.can.mcan, rxMsgObjHandle->msgObjId);

        IfxMultican_Can_MsgObj_readMessage(rxMsgObjHandle, &rxMsg);

        boolean isExtended = IfxMultican_MsgObj_isExtendedFrame(rxMo);
        boolean isRtr = FALSE;

        uint8 dlc = rxMsg.lengthCode;
        uint8 udpPayload[17];

        udpPayload[0] = 0x01;
        udpPayload[1] = 1;

        udpPayload[2] = (uint8)(rxMsg.id >> 0);
        udpPayload[3] = (uint8)(rxMsg.id >> 8);
        udpPayload[4] = (uint8)(rxMsg.id >> 16);
        udpPayload[5] = (uint8)(rxMsg.id >> 24);

        udpPayload[6] = isExtended;
        udpPayload[7] = isRtr;
        udpPayload[8] = dlc;

        uint8 can_data_bytes[8];
        can_data_bytes[0] = (uint8)(rxMsg.data[0] >> 0);
        can_data_bytes[1] = (uint8)(rxMsg.data[0] >> 8);
        can_data_bytes[2] = (uint8)(rxMsg.data[0] >> 16);
        can_data_bytes[3] = (uint8)(rxMsg.data[0] >> 24);
        can_data_bytes[4] = (uint8)(rxMsg.data[1] >> 0);
        can_data_bytes[5] = (uint8)(rxMsg.data[1] >> 8);
        can_data_bytes[6] = (uint8)(rxMsg.data[1] >> 16);
        can_data_bytes[7] = (uint8)(rxMsg.data[1] >> 24);

        for (int i = 0; i < dlc; i++)
        {
            udpPayload[9 + i] = can_data_bytes[i];
        }

        sendEthernetPacket(udpPayload, 9 + dlc);

        /*
         * FIX: Manually clear the Receive Pending flag.
         * This is the crucial step to prevent the same message from being read again.
         */
        IfxMultican_Can_MsgObj_clearRxPending(rxMsgObjHandle);
    }
}

void forwardEthernetToCan(void)
{
    uint8 ethPayload[17];
    uint16 payloadLen = receiveEthernetPacket(ethPayload, sizeof(ethPayload));

    if (payloadLen > 0)
    {
        // Packet validation
        if (payloadLen < 9 || ethPayload[0] != 0x01) { return; }
        uint8 dlc = ethPayload[8];
        if (dlc > 8 || payloadLen < (9 + dlc)) { return; }

        // Packet parsing
        uint32 can_id = ((uint32)ethPayload[2]) | ((uint32)ethPayload[3] << 8) | ((uint32)ethPayload[4] << 16) | ((uint32)ethPayload[5] << 24);
        boolean id_type = ethPayload[6];
        boolean frame_type = ethPayload[7];

        // Use the proven high-level transmit function.
        // This will only forward standard data frames as per the TX message object's initial configuration.
        if (frame_type == 0 && id_type == FALSE)
        {
            uint32 dataLow = 0;
            uint32 dataHigh = 0;
            for (int i = 0; i < dlc; i++)
            {
                if (i < 4) { dataLow |= ((uint32)ethPayload[9 + i] << (i * 8)); }
                else { dataHigh |= ((uint32)ethPayload[9 + i] << ((i - 4) * 8)); }
            }

            // Create the CAN message with the ID and payload from the UDP packet
            IfxMultican_Message canMsg;
            IfxMultican_Message_init(&canMsg, can_id, dataLow, dataHigh, (IfxMultican_DataLengthCode)dlc);

            // Send the message using the function we know works reliably
            transmitCanMessage(&canMsg);
        }
    }
}

/*
void forwardEthernetToCan(void)
{
    uint8 ethPayload[17];
    uint16 payloadLen = receiveEthernetPacket(ethPayload, sizeof(ethPayload));

    if (payloadLen < 9 || ethPayload[0] != 0x01)
    {
        return;
    }

    uint8 dlc = ethPayload[8];
    if (dlc > 8 || payloadLen < (9 + dlc))
    {
        return;
    }

    uint32 can_id = ((uint32)ethPayload[2]) |
                    ((uint32)ethPayload[3] << 8) |
                    ((uint32)ethPayload[4] << 16) |
                    ((uint32)ethPayload[5] << 24);

    boolean id_type = ethPayload[6];
    boolean frame_type = ethPayload[7];

    uint32 dataLow = 0;
    uint32 dataHigh = 0;

    if (frame_type == 0)
    {
        for (int i = 0; i < dlc; i++)
        {
            if (i < 4)
            {
                dataLow |= ((uint32)ethPayload[9 + i] << (i * 8));
            }
            else
            {
                dataHigh |= ((uint32)ethPayload[9 + i] << ((i - 4) * 8));
            }
        }
    }


    Ifx_CAN_MO* txMo = IfxMultican_MsgObj_getPointer(g_multican.can.mcan, TX_MSG_OBJ_ID);

    if (!IfxMultican_MsgObj_isTxPending(txMo))
    {
        IfxPort_togglePin(RX_ETH_LED_PORT, RX_ETH_LED_PIN);


        IfxMultican_Message canMsg;
        IfxMultican_Message_init(&canMsg, can_id, dataLow, dataHigh, (IfxMultican_DataLengthCode)dlc);
        transmitCanMessage(&canMsg);

        /*
        unsigned int control_word = 0;

        txMo->AR.B.IDE = id_type;
        txMo->AR.B.ID  = can_id << (id_type ? 0 : 18);
        txMo->FCR.B.DLC = (IfxMultican_DataLengthCode)dlc;

        if (frame_type == 1) // Remote Frame
        {
            control_word |= (1 << 21); // Set RTR bit
        }
        else // Data Frame
        {
            txMo->DATAL.U = dataLow;
            txMo->DATAH.U = dataHigh;
            control_word |= (1 << 25); // Set NEWDAT bit
        }

        control_word |= (1 << 23); // Set TXRQ bit

        txMo->CTR.U = control_word;

    }
}
*/
void initRXETHLed(void)
{
    IfxPort_setPinModeOutput(RX_ETH_LED_PORT, RX_ETH_LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinHigh(RX_ETH_LED_PORT, RX_ETH_LED_PIN);
}
