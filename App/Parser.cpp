/*
 * Parser.cpp
 *
 *  Created on: Sep 18, 2024
 *      Author: tulip
 */

// Parser.cpp
#include <Parser.hpp>
#include <cstdio>   // for printf
#include <cstring>  // for strcpy, strlen
#include <cstdarg>  // Include for va_list and related functions
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "pattern_player.hpp"
#include "dma_buffer.h" // Assuming this is where uart_rx_stream_setup is defined
extern bool isMotor1Connected;
extern bool isMotor2Connected;
#define UART_BUFFER_SIZE 256

int current_duty_motor1 = 0;
int current_duty_motor2 = 0;
int period = 0;

/**************************************************************************/
/*!
    @brief  Instantiates a new DRV2605 class.
    @param i2c_handle Pointer to the I2C handle
*/
/**************************************************************************/

Parser::Parser() {
	// Constructor logic if needed
}

void Parser::handleReceivedJson(const char *json) {
	StaticJsonDocument<256> doc;
	DeserializationError error = deserializeJson(doc, json);
	if (error) {
		printf("deserializeJson() failed\r\n");
		return;
	}

	// Check json packet content
	if (doc.containsKey("cmd") && doc.containsKey("deney_no")) {

		// Retrive values with keys
		std::string cmd = doc["cmd"];
		//int channel = doc["deney_no"];
		//int period= doc["period"];

		if (cmd == "set") {
		    bool updated = false;

		    if (doc.containsKey("motor1_duty")) {
		        next_duty_motor1 = doc["motor1_duty"].as<int>();
		        updated = true;
		    }

		    if (doc.containsKey("motor2_duty")) {
		        next_duty_motor2 = doc["motor2_duty"].as<int>();
		        updated = true;
		    }

		    if (doc.containsKey("period")) {
		        next_period = doc["period"].as<int>();
		        updated = true;
		    }

		    if (updated) {
		        pendingUpdate = true;
		        sendStatusToPython();  // STM’den geri bildirim
		    }
		}

	else
	{
		printJson("STM: Invalid JSON format\r\n");
	}
}
}

void Parser::sendJsonMessage() {
	char json_buffer[50];
	StaticJsonDocument<128> doc;

	doc["adc"] = "channel_1";
	doc["value"] = 50;
	serializeJson(doc, json_buffer, sizeof(json_buffer));
	printJson("%s\r\n", json_buffer);
	doc.clear();
}


void Parser::sendPingMessage() {
    char ping_buffer[50];
    StaticJsonDocument<64> pingDoc;

    pingDoc["status"] = "ok";
    pingDoc["msg"] = "received";
    serializeJson(pingDoc, ping_buffer, sizeof(ping_buffer));
    printJson("%s\r\n", ping_buffer);
    pingDoc.clear();
}


void Parser::printJson(const char *format, ...) {
	char buffer[UART_BUFFER_SIZE];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, UART_BUFFER_SIZE, format, args);
	va_end(args);
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
			HAL_MAX_DELAY);
}

void Parser::sendStatusToPython() {
    StaticJsonDocument<250> doc;

    doc["cmd"] = "set";
    doc["motor1_duty"] = next_duty_motor1;
    doc["motor2_duty"] = next_duty_motor2;
    doc["period"] = next_period;
    doc["motor1_connected"] = isMotor1Connected ? true : false;
    doc["motor2_connected"] = isMotor2Connected ? true : false;

    char buffer[250];
    serializeJson(doc, buffer, sizeof(buffer));
    printJson("%s\r\n", buffer);
    doc.clear();
}



extern UART_HandleTypeDef huart2;
