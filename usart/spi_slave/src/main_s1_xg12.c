/**************************************************************************//**
 * @main_series1_XG12.c
 * @brief Demonstrates USART1 as SPI slave.
 * @version 0.0.1
 ******************************************************************************
 * @section License
 * <b>Copyright 2018 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

#define RX_BUFFER_SIZE   10
#define TX_BUFFER_SIZE   RX_BUFFER_SIZE

uint8_t RxBuffer[RX_BUFFER_SIZE];
uint8_t RxBufferIndex = 0;

uint8_t TxBuffer[TX_BUFFER_SIZE] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9};
uint8_t TxBufferIndex = 1;

/**************************************************************************//**
 * @brief USART2 RX IRQ Handler
 *****************************************************************************/
void USART2_RX_IRQHandler(void)
{
  if (USART2->STATUS & USART_STATUS_RXDATAV)
  {
    // Read data
    RxBuffer[RxBufferIndex++] = USART_Rx(USART2);

    // Sending data, the USART_Tx function checks that the Tx buffer is clear before sending
    USART_Tx(USART2, TxBuffer[TxBufferIndex++]);

    if (RxBufferIndex == RX_BUFFER_SIZE)
    {
      // Putting a break point here to view the full RxBuffer,
      // The RxBuffer should be: 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
      RxBufferIndex = 0;
    }

    if(TxBufferIndex == TX_BUFFER_SIZE)
    {
      TxBufferIndex = 0;
    }
  }
}

/**************************************************************************//**
 * @brief Initialize USART2
 *****************************************************************************/
void initUSART2 (void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART2, true);

	// Configure GPIO mode
	GPIO_PinModeSet(gpioPortA, 8, gpioModeInput, 1);    // US2_CLK is input
	GPIO_PinModeSet(gpioPortA, 9, gpioModeInput, 1);    // US2_CS is input
	GPIO_PinModeSet(gpioPortA, 6, gpioModeInput, 1);    // US2_TX (MOSI) is input
	GPIO_PinModeSet(gpioPortA, 7, gpioModePushPull, 1); // US2_RX (MISO) is push pull

	// Start with default config, then modify as necessary
	USART_InitSync_TypeDef config = USART_INITSYNC_DEFAULT;
	config.master    = false;
	config.clockMode = usartClockMode0; // clock idle low, sample on rising/first edge
	config.msbf      = true;            // send MSB first
        config.enable    = false;           // making sure to keep USART disabled until we've set everything up
	USART_InitSync(USART2, &config);

	// Set USART pin locations
	USART2->ROUTELOC0 = (USART_ROUTELOC0_CLKLOC_LOC1) | // US2_CLK       on location 1 = PA8 per datasheet section 6.4 = EXP Header pin 8
	                    (USART_ROUTELOC0_CSLOC_LOC1)  | // US2_CS        on location 1 = PA9 per datasheet section 6.4 = EXP Header pin 10
	                    (USART_ROUTELOC0_TXLOC_LOC1)  | // US2_TX (MOSI) on location 1 = PA6 per datasheet section 6.4 = EXP Header pin 4
	                    (USART_ROUTELOC0_RXLOC_LOC1);   // US2_RX (MISO) on location 1 = PA7 per datasheet section 6.4 = EXP Header pin 6

	// Enable USART pins
	USART2->ROUTEPEN = USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_CSPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN;

	// Enable USART2 RX interrupts
	USART_IntClear(USART2, USART_IF_RXDATAV);
	USART_IntEnable(USART2, USART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(USART2_RX_IRQn);
	NVIC_EnableIRQ(USART2_RX_IRQn);

	// Pre-loading our TXDATA register so our slave's echo can be in synch with the master
	USART2->TXDATA = TxBuffer[0];

	// Enable USART2
	USART_Enable(USART2, usartEnable);
}

/**************************************************************************//**
 * @brief Main function
 *****************************************************************************/
int main(void)
{
  // Initialize chip
  CHIP_Init();

  // Initialize USART2 as SPI slave
  initUSART2();

  //packets will begin coming in as soon as the master code starts
  while(1);
}

