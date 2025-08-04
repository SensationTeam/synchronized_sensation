/*
 *
 *
 * */
//============================================================================
#include "dma_buffer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//============================================================================

UartStream_t* uart_rx_stream_setup(UART_HandleTypeDef *uart,
		DMA_HandleTypeDef *dma, uint16_t tmp_buffer_size,
		uint16_t main_buffer_size) {

	UartStream_t *stream = calloc(1, sizeof(UartStream_t));
	stream->link_dma = dma;
	stream->uart_handler = uart;
	stream->main_buffer_size = main_buffer_size;
	stream->tmp_buffer_size = tmp_buffer_size;
	stream->main_buffer = calloc(main_buffer_size, sizeof(uint8_t));
	stream->tmp_buffer = calloc(tmp_buffer_size, sizeof(uint8_t));

	HAL_UARTEx_ReceiveToIdle_DMA(stream->uart_handler, stream->tmp_buffer,
			stream->tmp_buffer_size);
	__HAL_DMA_DISABLE_IT(stream->link_dma, DMA_IT_HT);

	return stream;
}
//============================================================================
void uart_rx_stream_callback(UartStream_t *stream, uint16_t Size) {

	stream->last_position = stream->current_position;
	if (stream->last_position + Size > stream->main_buffer_size) {

		uint16_t datatocopy = stream->main_buffer_size - stream->last_position;
		memcpy((uint8_t*) stream->main_buffer + stream->last_position,
				(uint8_t*) stream->tmp_buffer, datatocopy);
		stream->last_position = 0;
		memcpy((uint8_t*) stream->main_buffer,
				(uint8_t*) stream->tmp_buffer + datatocopy,
				(Size - datatocopy));
		stream->current_position = (Size - datatocopy);
	} else {
		memcpy((uint8_t*) stream->main_buffer + stream->last_position,
				(uint8_t*) stream->tmp_buffer, Size);
		stream->current_position = Size + stream->last_position;
	}

	HAL_UARTEx_ReceiveToIdle_DMA(stream->uart_handler, stream->tmp_buffer,
			stream->tmp_buffer_size);
	__HAL_DMA_DISABLE_IT(stream->link_dma, DMA_IT_HT);
}

//============================================================================

/*
 * get single byte
 *
 * */

uint8_t stream_read_byte(UartStream_t *stream) {
    uint8_t tmp = 0x00;

    // Loop until a valid byte is read or the buffer is empty
    while (stream->read_position != stream->current_position) {
        // Read the byte at the current read position
        tmp = stream->main_buffer[stream->read_position];

        // Only advance the read position if the byte is not null
        if (tmp != 0x00) {
            stream->main_buffer[stream->read_position++] = 0x00; // Clear the byte after reading

            // Wrap the read position if it reaches the end of the buffer
            if (stream->read_position == stream->main_buffer_size) {
                stream->read_position = 0;
            }
            break; // Exit the loop since a valid byte was read
        } else {
            break; // Stop processing if the byte is null
        }
    }

    return tmp;
}



void stream_read_line(UartStream_t *stream, uint8_t *buffer) {

	//memset(buffer, '\0', sizeof(buffer));

	uint8_t *buff = buffer;

	do {
		*buff = stream_read_byte(stream);
	} while ((*buff != '\n') && (*buff++ != 0x00));

	*buff = '\0'; // override null character and make it a string by adding a null to the end
}



