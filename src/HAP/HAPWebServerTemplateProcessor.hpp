//
// HAPWebServerTemplateProcessor.hpp
// Homekit
//
//  Created on: 12.1.2020
//      Author: michael
//
#ifndef HAPWEBSERVERTEMPLATEPROCESSOR_HPP_
#define HAPWEBSERVERTEMPLATEPROCESSOR_HPP_

#include <HTTPSServer.hpp>
#include <functional>
#include "HAPGlobals.hpp"

#ifndef HAP_WEBSERVER_USE_SPIFFS
#define HAP_WEBSERVER_USE_SPIFFS    0
#endif

#if HAP_WEBSERVER_USE_SPIFFS
#include "HAPFileReader.hpp"
#endif

// Easier access to the classes of the server
using namespace httpsserver;

#define OPENING_CURLY_BRACKET_CHAR '{'
#define CLOSING_CURLY_BRACKET_CHAR '}'
#define ESCAPE_CHAR 92 // char: '\'
#define BUFFER_SIZE HTTPS_KEEPALIVE_CACHESIZE // Buffer size for file reading

#define HAP_WEBSERVER_TEMPLATE_PROCESSING_DEBUG     0
#define HAP_WEBSERVER_TEMPLATE_PROCESSING_CHUNKED   0   // not yet working :(


class HAPWebServerTemplateProcessor {

public:
#if HAP_WEBSERVER_USE_SPIFFS
    static bool processAndSend(HTTPResponse * res, const String& filePath, const std::function<void(const String&, HTTPResponse*)> getKeyValueCallback, const uint16_t statusCode = 200, const std::string statusText = "OK", const std::string contentType = "text/html");
#endif    
    static bool processAndSendEmbedded(HTTPResponse * res, const uint8_t* startIndex, const uint8_t* endIndex, const std::function<void(const String&, HTTPResponse*)> getKeyValueCallback, const uint16_t statusCode = 200, const std::string statusText = "OK", const std::string contentType = "text/html");

#if HAP_WEBSERVER_TEMPLATE_PROCESSING_CHUNKED
    static void sendChunk(HTTPResponse * res, const String& message);
    static void sendChunk(HTTPResponse * res, const char* message);
    static void sendChunk(HTTPResponse * res, const uint8_t* message, const size_t length);
#endif

private:
    static void sendError(HTTPResponse * res, const String& errorDescription);

};

#endif