/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "MULTICAN.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
AppMulticanType   g_multican;

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/

/* Function to initialize MULTICAN module, node and message objects for TX and RX */
void initMultican(void)
{
    /* enable CAN module */
    IfxCpu_enableInterrupts();

    /* Disable watchdogs */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Create module configuration and initialize the module*/
    IfxMultican_Can_initModuleConfig(&g_multican.canConfig, &MODULE_CAN);
    IfxMultican_Can_initModule(&g_multican.can, &g_multican.canConfig);

    /* Create node configuration and initialize the node */
    IfxMultican_Can_Node_initConfig(&g_multican.canNodeConfig, &g_multican.can);
    g_multican.canNodeConfig.baudrate = 500000; /* 500 kBit/s */
    g_multican.canNodeConfig.nodeId    = IfxMultican_NodeId_1;
    g_multican.canNodeConfig.rxPin = &IfxMultican_RXD1B_P14_1_IN;
    g_multican.canNodeConfig.txPin = &IfxMultican_TXD1_P14_0_OUT;
    g_multican.canNodeConfig.txPinMode = IfxPort_OutputMode_pushPull;
    g_multican.canNodeConfig.rxPinMode = IfxPort_InputMode_pullUp;
    IfxMultican_Can_Node_init(&g_multican.canNode, &g_multican.canNodeConfig);

    /* Create and initialize the transmit message object */
    IfxMultican_Can_MsgObj_initConfig(&g_multican.canMsgObjConfig, &g_multican.canNode);
    g_multican.canMsgObjConfig.msgObjId             = TX_MSG_OBJ_ID; /* Use macro for TX MO ID */
    g_multican.canMsgObjConfig.messageId            = CAN_MESSAGE_ID_TX;
    g_multican.canMsgObjConfig.frame                = IfxMultican_Frame_transmit;
    g_multican.canMsgObjConfig.control.messageLen   = IfxMultican_DataLengthCode_8;
    g_multican.canMsgObjConfig.control.extendedFrame = FALSE;
    IfxMultican_Can_MsgObj_init(&g_multican.canMsgObj, &g_multican.canMsgObjConfig);

    /* Create and initialize the receive message object */
    IfxMultican_Can_MsgObj_initConfig(&g_multican.canMsgObjConfig, &g_multican.canNode);
    g_multican.canMsgObjConfig.msgObjId              = RX_MSG_OBJ_ID; /* Use macro for RX MO ID */
    g_multican.canMsgObjConfig.messageId             = CAN_MESSAGE_ID_RX;
    g_multican.canMsgObjConfig.frame                 = IfxMultican_Frame_receive;
    g_multican.canMsgObjConfig.control.messageLen    = IfxMultican_DataLengthCode_8;
    g_multican.canMsgObjConfig.control.extendedFrame = FALSE;
    g_multican.canMsgObjConfig.acceptanceMask        = 0x7FFFFFFF; /* Accept any standard message ID */
    IfxMultican_Can_MsgObj_init(&g_multican.canRxMsgObj, &g_multican.canMsgObjConfig);
}

/**
 * \brief Transmits a CAN message.
 * \param message Pointer to the message that should be transmitted.
 */
void transmitCanMessage(IfxMultican_Message *message)
{
    /* Loop until the message is accepted for transmission */
    while (IfxMultican_Can_MsgObj_sendMessage(&g_multican.canMsgObj, message) == IfxMultican_Status_notSentBusy)
    {
        /* Wait for the transmit buffer to be free */
    }
}

/**
 * \brief Checks for and reads a CAN message.
 * \param msg Pointer to a message structure where the received data will be stored.
 * \return Returns the status of the message read operation (e.g., newData, noNewData).
 */
IfxMultican_Status receiveCanMessage(IfxMultican_Message* msg)
{
    /* Initialize the message structure to be populated */
    IfxMultican_Message_init(msg, 0, 0, 0, IfxMultican_DataLengthCode_8);

    /* Read the message from the hardware message object */
    /* This function also clears the RX pending flag in the background */
    return IfxMultican_Can_MsgObj_readMessage(&g_multican.canRxMsgObj, msg);
}
