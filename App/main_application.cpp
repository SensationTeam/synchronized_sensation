#include "pattern_player.hpp"
#include "ADAFRUIT_DRV2605.hpp"
#include "Parser.hpp"
#include "main.h"
#include "usart.h"
#include "i2c.h"
#include "tim.h"
#include "dma_buffer.h"
#include "application.hpp"

Adafruit_DRV2605 haptic1(&hi2c1, &htim2); // First motor (I2C3)
Adafruit_DRV2605 haptic2(&hi2c3, &htim2); // Second motor (I2C1)

extern UartStream_t *python_port; // UART RX Ring Buffer
Parser parser;

/**
 * Function Prototypes
 **/


/**
 * Timer callback function to restart vibration
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2)
    {
        PatternTimerCallback();
    }
}

/**
 * Setup function
 */
extern "C" void Setup() {
    //Start UART DMA based streaming
	printf("📡 System Booting...\r\n");
	for (int i = 0; i < 10; i++) {
	        printf("Boot log: %d\r\n", i);
	        HAL_Delay(500);}

    python_port = uart_rx_stream_setup(&huart2, &hdma_usart2_rx, 256, 1500);

    // Initialize pattern player
    PatternPlayer_Init();

    // Start the timer in interrupt mode
    HAL_TIM_Base_Start_IT(&htim2);
}



/**
 * Loop function to keep sending haptic signals
 */
extern "C" void Loop() {

    uint8_t dataBuffer[256];
    stream_read_line(python_port, dataBuffer);

    if (*dataBuffer) {
        printf("Python: %s\r\n", dataBuffer);
        parser.handleReceivedJson((const char*) dataBuffer);
    }

    //parser.sendJsonMessage();
}
