/*
 * Parser.h
 *
 *  Created on: Sep 18, 2024
 *      Author: tulip
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include "json/ArduinoJson.h"


class Parser {
public:
    Parser();
    void handleReceivedJson(const char* json);
    void sendJsonMessage();
    void sendPingMessage();
    void sendStatusToPython();
private:
    void printJson(const char* format, ...);
};




#endif /* PARSER_H_ */
