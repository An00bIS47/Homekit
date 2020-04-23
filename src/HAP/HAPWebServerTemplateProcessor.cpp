//
// HAPWebServerTemplateProcessor.cpp
// Homekit
//
//  Created on: 12.1.2020
//      Author: michael
//

#include "HAPWebServerTemplateProcessor.hpp"

#if HAP_WEBSERVER_TEMPLATE_PROCESSING_DEBUG	
#include "HAPLogger.hpp"
#endif

#if HAP_WEBSERVER_USE_SPIFFS
bool HAPWebServerTemplateProcessor::processAndSend(HTTPResponse * res, const String& filePath, const std::function<void(const String&, HTTPResponse*)> getKeyValueCallback, const uint16_t statusCode, const std::string statusText, const std::string contentType){
    // Opening the file
    HAPFileReader reader;
    if (!reader.open(filePath)) {
        sendError(res, "Unable to open " + filePath);
        return false;
    }

    // Preparing output
    res->setStatusCode(statusCode);
    res->setStatusText(statusText);
    res->setHeader("Content-Type", contentType);    
    res->setHeader("Cache-Control", "no-cache");
    res->setHeader("Connection", "keep-alive");

    // Processing the file one char at the time
    char buffer[BUFFER_SIZE];
    unsigned int index = 0;
    unsigned int lastCurlyBracePosition;
    char ch;

    while (reader.readChar(ch)) {
        // Template handling
        if (ch == OPENING_CURLY_BRACKET_CHAR)
        {
            // Clear out buffer if there is not enough space to handle 2 other readings
            if (index >= BUFFER_SIZE - 3)
            {
                buffer[index] = '\0';
                res->print((const char*)buffer);        
                index = 0;
            }

            lastCurlyBracePosition = reader.getCurrentPosition();
            buffer[index] = ch;
            index++;

        // Read the 2 next char to detect templating syntax
            for (unsigned int i = 0; i < 2 && reader.readChar(ch) ; i++)
            {
                buffer[index] = ch;
                index++;
            }

            // When we encounter two opening curly braces, it's a template syntax that we must handle
            if (index >= 3 && buffer[index - 2] == OPENING_CURLY_BRACKET_CHAR && buffer[index - 3] == OPENING_CURLY_BRACKET_CHAR)
            {
                // if the last char is an escape char, remove it form the template and don't do the substitution
                if (buffer[index - 1] == ESCAPE_CHAR)
                {
                    index--;
                }
                else
                {
                    // if the buffer contain more than the last 3 chars, send it's content omitting the last 3 chars 
                    if (index > 3)
                    {
                        buffer[index - 3] = '\0';
                        res->print((const char*)buffer);  
                    }
            
                    // Set the first char of the key into the buffer as it was not an escape char
                    buffer[0] = ch;
                    index = 1;

                    // Extract the key for substitution
                    bool found = false;
                    char lastChar = ' ';

                    while (!found && index < BUFFER_SIZE - 2 && reader.readChar(ch) && !reader.endOfLine())
                    {
                        if (ch == CLOSING_CURLY_BRACKET_CHAR && lastChar == CLOSING_CURLY_BRACKET_CHAR) {
                            found = true;
                        } else if (ch != CLOSING_CURLY_BRACKET_CHAR) {
                        buffer[index] = ch;
                            index++;
                        }

                        lastChar = ch;
                    }

                    // Check for bad exit.
                    if (!found) {
                        sendError(res, "Parsing error, opening curly bracket found without corresponding closing brackets in file " + filePath + " at line " + String(reader.getCurrentLine()) + " position " + String(lastCurlyBracePosition) + "\n");
                        return false;
                    }

                    // Get key value
                    buffer[index] = '\0';
                    // res->print(getKeyValueCallback(buffer));
                    getKeyValueCallback(buffer, res);
                    index = 0;
                }
            }
        }
        else 
        {
            buffer[index] = ch;
            index++;

            if (index >= BUFFER_SIZE-1) {
                buffer[BUFFER_SIZE-1] = '\0';
                res->print((const char*)buffer);  
                index = 0;
            }
        }
    }

    if (index > 0) {
        buffer[index] = '\0';
        res->print((const char*)buffer);  
    }

  return true;
}
#endif



void HAPWebServerTemplateProcessor::sendError(HTTPResponse * res, const String& errorDescription) {
    res->setStatusCode(500);
    res->setStatusText("Internal Server Error");
    res->setHeader("Content-Type", "text/html");    
    res->setHeader("Cache-Control", "no-cache");

    res->print("<br/><b>Error:</b> " + errorDescription);
    res->print("");
    res->println("Error : " + errorDescription);
}


bool HAPWebServerTemplateProcessor::processAndSendEmbedded(HTTPResponse * res, const uint8_t* startIndex, const uint8_t* endIndex, const std::function<void(const String&, HTTPResponse*)> getKeyValueCallback, const uint16_t statusCode, const std::string statusText, const std::string contentType){
    // Processing the file one char at the time
    char buffer[BUFFER_SIZE];
    unsigned int index = 0;
    unsigned int lastCurlyBracePosition;
    char ch;
	unsigned int curReadIndex = 0;
	unsigned int curLine = 0;

	// Preparing output
    res->setStatusCode(statusCode);
    res->setStatusText(statusText);
    res->setHeader("Content-Type", contentType);    
    res->setHeader("Cache-Control", "no-cache");
	res->setHeader("Connection", "keep-alive");
		
	while (curReadIndex <= (endIndex - startIndex) ) {
		ch = startIndex[curReadIndex++];		

		if (ch == OPENING_CURLY_BRACKET_CHAR) {
			// Clear out buffer if there is not enough space to handle 2 other readings
			if (index >= BUFFER_SIZE - 3) {
				buffer[index] = '\0';
				res->print((const char*)buffer);  
				// sendChunk(res, (const char*)buffer);

#if HAP_WEBSERVER_TEMPLATE_PROCESSING_DEBUG				
				LogE("curReadIndex: " + String(curReadIndex), true);
				Serial.print((const char*)buffer);
#endif

				index = 0;
			}


			lastCurlyBracePosition = curReadIndex;			

			buffer[index] = ch;
			index++;

			// Read the 2 next char to detect templating syntax
			for (unsigned int i = 0; i < 2 ; i++) {
				ch = startIndex[curReadIndex++];				
				buffer[index] = ch;
				index++;
			}

			// When we encounter two opening curly braces, it's a template syntax that we must handle
			if (index >= 3 && buffer[index - 2] == OPENING_CURLY_BRACKET_CHAR && buffer[index - 3] == OPENING_CURLY_BRACKET_CHAR) {
				// if the last char is an escape char, remove it form the template and don't do the substitution
				if (buffer[index - 1] == ESCAPE_CHAR) {
					index--;
				} else {
					// if the buffer contain more than the last 3 chars, send it's content omitting the last 3 chars 
					if (index > 3) {
						buffer[index - 3] = '\0';
						res->print((const char*)buffer); 
						// sendChunk(res, (const char*)buffer);
#if HAP_WEBSERVER_TEMPLATE_PROCESSING_DEBUG
						LogE("curReadIndex: " + String(curReadIndex), true);
						Serial.print((const char*)buffer); 
#endif
					}
				
					// Set the first char of the key into the buffer as it was not an escape char
					buffer[0] = ch;
					index = 1;

					// Extract the key for substitution
					bool found = false;
					char lastChar = ' ';

					bool isEndOfLine = false;

					while (!found && index < BUFFER_SIZE - 2 && !isEndOfLine) {	
						ch = startIndex[curReadIndex++];						
						if (ch == 10 || ch == 13) {
							isEndOfLine = true;
							curLine++;
							break;
						}
						if (ch == CLOSING_CURLY_BRACKET_CHAR && lastChar == CLOSING_CURLY_BRACKET_CHAR) {
							found = true;
						} else if (ch != CLOSING_CURLY_BRACKET_CHAR) {
							buffer[index] = ch;
							index++;
						}

						lastChar = ch;
					}

					// Check for bad exit.
					if (!found) {
						sendError(res, "Parsing error, opening curly bracket found without corresponding closing brackets at line " + String(curLine) + " position " + String(lastCurlyBracePosition) + "\n");
#if HAP_WEBSERVER_TEMPLATE_PROCESSING_DEBUG							
						LogE("ERROR: Parsing template file failed! Opening curly bracket found without corresponding closing bracket at line " + String(curLine) + " position " + String(lastCurlyBracePosition), true);
#endif						
						return false;
					}

					// Get key value
					buffer[index] = '\0';
					
					getKeyValueCallback(buffer, res);
					index = 0;
				}
			}

		} else  {
			buffer[index] = ch;
			index++;

			if (index >= BUFFER_SIZE-1) {
				buffer[BUFFER_SIZE-1] = '\0';
				res->print((const char*)buffer);  
				// sendChunk(res, (const char*)buffer);
#if HAP_WEBSERVER_TEMPLATE_PROCESSING_DEBUG
				LogE("curReadIndex: " + String(curReadIndex), true);
				Serial.print((const char*)buffer);
#endif
				index = 0;
			}
		}
		
	}

	if (index > 0) {
    	buffer[index] = '\0';
    	
		// sendChunk(res, (const char*)buffer);
		res->print((const char*)buffer);  
		res->print("");

#if HAP_WEBSERVER_TEMPLATE_PROCESSING_DEBUG		
		LogE("curReadIndex: " + String(curReadIndex), true);
		Serial.print((const char*)buffer);
#endif			
  	}

#if HAP_WEBSERVER_TEMPLATE_PROCESSING_CHUNKED
	if ((curReadIndex - 1) == (endIndex - startIndex)){
		// Send terminating chunk
		sendChunk(res, 0, 0);
	}
#endif	

	return true;
}


#if HAP_WEBSERVER_TEMPLATE_PROCESSING_CHUNKED
void HAPWebServerTemplateProcessor::sendChunk(HTTPResponse * res, const uint8_t* message, const size_t length){
	res->printf("%x%s", length, "\r\n");
	res->write(message, length);
	// res->print("\r\n");
}

void HAPWebServerTemplateProcessor::sendChunk(HTTPResponse * res, const String& message){
	sendChunk(res, message.c_str());	
}

void HAPWebServerTemplateProcessor::sendChunk(HTTPResponse * res, const char* message){	
	res->printf("%x%s", strlen(message), "\r\n");
	res->print(message);
	res->print("\r\n");
}
#endif