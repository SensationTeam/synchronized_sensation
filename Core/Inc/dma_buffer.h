
#ifndef DMA_BUFFER_H
#define DMA_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"
#include "usart.h"

//TODO: read_position
typedef struct {

	UART_HandleTypeDef* uart_handler;
	DMA_HandleTypeDef* link_dma;
	uint8_t * tmp_buffer;
	uint16_t tmp_buffer_size;
	uint8_t* main_buffer;
	uint16_t main_buffer_size;
	uint16_t current_position;
	uint16_t last_position;
	uint16_t read_position;
}UartStream_t;


extern UartStream_t* uart_rx_stream_setup(UART_HandleTypeDef *uart, DMA_HandleTypeDef *dma,
		uint16_t tmp_buffer_size,	uint16_t main_buffer_size );

extern void uart_rx_stream_callback(UartStream_t *stream, uint16_t Size);
extern void stream_read_line (UartStream_t *stream, uint8_t* buffer);
extern void stream_read_after_5aa5(UartStream_t *stream, uint8_t *buffer);

#ifdef __cplusplus
}
#endif


#endif  /**/
