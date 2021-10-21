#include "WSSIM800L.h"

// AT COMMAND checks if the chip is ready
const char CMD_AT[] PROGMEM = "AT";
// puts the chip into sleep mode
const char CMD_CSCLK2[] PROGMEM = "AT+CSCLK=2";
// HTTP related
const char CMD_SAPBR_GPRS[] PROGMEM = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"";
const char CMD_SAPBR_APN[] PROGMEM = "AT+SAPBR=3,1,\"APN\",\"";
const char CMD_SAPBR0[] PROGMEM = "AT+SAPBR=0,1"; 
const char CMD_SAPBR1[] PROGMEM = "AT+SAPBR=1,1";
const char CMD_SAPBR2[] PROGMEM = "AT+SAPBR=2,1";
const char CMD_HTTPINIT[] PROGMEM = "AT+HTTPINIT";
const char CMD_HTTPPARA_CID[] PROGMEM = "AT+HTTPPARA=\"CID\",1";
const char CMD_HTTPPARA_URL[] PROGMEM = "AT+HTTPPARA=\"URL\",\""; 
const char CMD_HTTPACTION0[] PROGMEM = "AT+HTTPACTION=0";
const char CMD_HTTPREAD[] PROGMEM = "AT+HTTPREAD=0,";
// SMS Related
const char CMD_CMGF[] PROGMEM = "AT+CMGF=1";
const char CMD_CMGS[] PROGMEM = "AT+CMGS=\"+";
const char CMD_CMGDA[] PROGMEM = "AT+CMGDA=\"DEL ALL\"";
// responses
const char RESP_OK[] PROGMEM = "OK";
const char RESP_ERROR[] PROGMEM = "ERROR";
const char RESP_GT[] PROGMEM = ">";
const char RESP_CMGS[] PROGMEM = "+CMGS:";
const char RESP_HTTPREAD[] PROGMEM = "+HTTPREAD:";


WSSIM800L::WSSIM800L(int baudrate) {
	/*	Constructor for the class, empty by design - all of 
	*	the setup happens in the begin() method so that
	*	the board can power up before the class initialises 
	*	pins otherwise issues happen
	*/ 
	simSerial = new AltSoftSerial;
	simSerial->begin(baudrate);
	timer(SHORT); // set the timer
  sleep();
	pinMode(SIM_RESET_PIN, OUTPUT);
}


bool WSSIM800L::reset(){
  // reset the SIM800L
	#if DEBUG
		Serial.println(F("WeatherStation:: Resetting SIM800L"));
	#endif

	// Reset the device
	digitalWrite(SIM_RESET_PIN, HIGH);
	delay(500);
	digitalWrite(SIM_RESET_PIN, LOW);
	delay(500);
	digitalWrite(SIM_RESET_PIN, HIGH);
	delay(20000);
  sleep();
	return true;
}



bool WSSIM800L::isReady() {	
	/*	The "AT" command is used to show the chip is ready for commands
	*/
	return doCommand_P(CMD_AT, "OK", NULL, 50);
}



void WSSIM800L::sleep() {	
	/*	The "AT" command is used to show the chip is ready for commands
	*/
	//doCommand_P(CMD_CSCLK2, NULL);
}




void WSSIM800L::wake() {	
	/*	when waking from sleep, the first issued command is ignored and the following must occur within 50ms 
  so it is easier to simply send the first command without any verification and concentrate on the second
	*/
  //simSerial->println(F("AT"));
	//doCommand_P(CMD_AT, NULL);
}



bool WSSIM800L::timer(TimerType type) {
	static unsigned long timeStamp;
	unsigned long currentTime = micros();

	switch (type) {
	case ELAPSED:
		if (currentTime - timeStamp >= this->maxTimeout){ return true; }
		break;
	case BEGIN:
	case RENEW:
		timeStamp = currentTime;
		break;
	case SHORT:
		maxTimeout =  1 * ONE_SECOND; // 1000 microseconds
		break;
	case LONGER:
		maxTimeout = 5 * ONE_SECOND; // 5000 microseconds
		break;
	}

	return false;
}




void WSSIM800L::clearBuffer() {																				// Clears the contents of the ioBuffer by filling it with zeroes.
	for (uint16_t i = 0; i < DEFAULT_BUFFER_SIZE; i++) { 
		buffer[i] = 0; 
	}
}


void WSSIM800L::readSerialToBuffer() {
	clearBuffer();
	timer(BEGIN);
	while (!simSerial->available()) {
		if (timer(ELAPSED)) {
			strcat_P(buffer, TIMEOUT);
			break;
		}
	}
	uint32_t buffCount = 0;
	while (!timer(ELAPSED) || awaitingResponse) {
		if (simSerial->available()) {
			awaitingResponse = false;
			timer(RENEW);
			if (buffCount <= DEFAULT_BUFFER_SIZE) {
				buffer[buffCount] = simSerial->read();
			} else {
				simSerial->read();
			}
			buffCount++;
		}
	}
}



bool WSSIM800L::checkResponse(const char* expected){
	readSerialToBuffer();
	char* comparison = strstr(buffer, expected);

	if (comparison != 0) { return true;	}
	return false;
}



bool WSSIM800L::doCommand_P(const char* command, const char* expected, const char* parameter, int8_t retries) {
	char commandBuffer[32];
  strcpy_P(commandBuffer, command);
  bool hasQuotes = false;

  if (parameter != NULL && command != CMD_HTTPREAD){
    hasQuotes = true;
  }

  return doCommand(commandBuffer, expected, parameter, hasQuotes, retries);
}





bool WSSIM800L::doCommand(const char* command, const char* expected, const char* parameter, bool hasQuotes, int8_t retries) {
	bool success = false;

	while (retries >= 0) {
    if(parameter == NULL){
      simSerial->println(command); // send the command to the chip
    } else {
      simSerial->print(command);
      simSerial->print(parameter);
      if(hasQuotes) {
        simSerial->print(F("\""));
      }
      simSerial->println();
    }

		
		if (expected == "+CMGS:") {
			simSerial->write(0x1a);
		}

		success = checkResponse(expected);  // check the response is as expected this also functions as a wait timer so

		#if DEBUG
			//log the command
			Serial.print(F("SIM800L Command: "));
      if(parameter == NULL){
        Serial.println(command); // send the command to the chip
      } else {
        Serial.print(command);
        Serial.print(parameter);
        if(hasQuotes) {
          Serial.print(F("\""));
        }
        Serial.println();
      }
			// log the response
			Serial.print(F("SIM800L Command Response: "));
			Serial.println(buffer);
		#endif
				

		if (success || expected == NULL) {
			return true;
		}

		retries --;	
	}
	
	return false;
}


void WSSIM800L::splitResponse(){
	char* buffptr;
  char current[20];
	buffptr = strtok(buffer, ",");
  strcpy(current, strtok(NULL, ","));
	responseStatus = atoi(current);
  strcpy(current, strtok(NULL, ","));
	responseLength = atoi(current);
}


bool WSSIM800L::sendSMS(const char* mobile, const char* message) {
  wake();

	#if DEBUG
		Serial.println(F("SIM800L:: Attempting to send SMS..."));
	#endif

	timer(SHORT);
	uint8_t resetCount = 0;

	// wait for the chip to respond that it's ready to process commands
	isReady();

	// set the chip to SMS mode
	if (!doCommand_P(CMD_CMGF, "OK")) return false;
	
	// set the mobile number to send the SMS to
	if (!doCommand_P(CMD_CMGS, ">", mobile)) return false;	
	
	// the remaining commands need longer to execute so switch to a longer timer
	timer(LONGER);

	//set the text content of the SMS and wait for verification
	if (!doCommand_P(message, "+CMGS:")) return false;
	
	//Clear the SMS from the SIM card so it won't get too full
	if (!doCommand_P(CMD_CMGDA, "OK")) return false;

	// set the timer back to short
	timer(SHORT);

  sleep();

	return true;
}






const char* WSSIM800L::httpGet(const char* apn, const char* url) {
	timer(SHORT);

	Serial.println(F("SIM800L:: Attempting to connect to website..."));

	// wait for the chip to respond that it's ready to process commands
	isReady();

	/* SAPBR is the mobile provider information, so we need to first set the content type to GPRS */
	if (!doCommand_P(CMD_SAPBR_GPRS, "OK")) return NULL;


	if (!doCommand_P(CMD_SAPBR_APN, "OK", apn)) return NULL;	


	// set the bearer profile to 1
	if (!doCommand_P(CMD_SAPBR1, NULL)) return NULL;	


	// initialise HTTP action
	if (!doCommand_P(CMD_HTTPINIT, NULL)) return NULL;	


	/* HTTPPARA is a set of parameters for a HTTP request so we must set several parameters before making that request */
	// set the CID parameter
	if (!doCommand_P(CMD_HTTPPARA_CID, "OK")) return NULL;	
	

	// set the URL parameter	
	if (!doCommand_P(CMD_HTTPPARA_URL, "OK", url)) return NULL;	


	// HTTPACTION defines whether the request is a Get(0) POST(1) or Head(2)
	if (!doCommand_P(CMD_HTTPACTION0, "OK")) return NULL;	
	
	// The response may take longer depending on network conditions so 
	// set a flag to stop the system from timing out and set the longer timer duration
	timer(LONGER);
	awaitingResponse = true;
	readSerialToBuffer();

	// the response contains the http status code and the length of the response
	// these are comma separated and so we need to split these into responseStatus and
	// responseLength variables for ease of use
	splitResponse();

	#if DEBUG
		Serial.print(F("SIM800L:: Responded with status code: "));
		Serial.println(responseStatus);
		Serial.println();
	#endif
	
	// if the HTTP status is 200 ("OK"), proceed with reading the response data
	if(responseStatus == 200){
    char charBuffer[4];
    itoa(responseLength, charBuffer, 10);
		if (!doCommand_P(CMD_HTTPREAD, "+HTTPREAD:", charBuffer)) return NULL;	

		// reading the relevant positions for the reply from the buffer into a reply char array
		int startPoint = floor(log10(abs(responseLength))) + 1 + HTTPREAD_COMMAND_LENGTH;
		char reply[responseLength];
		for(int i = 1; i <= responseLength; i++ ) {
			reply[i-1] = buffer[startPoint + i];
			if (i == responseLength) { reply[i] = '\0';}
		}
		

		#if DEBUG
			Serial.print(F("SIM800L Reply:: "));
			Serial.println(reply);
		#endif

    // close the GPRS connection and reset the timer to short
    doCommand_P(CMD_SAPBR0, NULL);
		timer(SHORT);

		// return the website content
		return strdup(&reply[0]);

	} else {
		#if DEBUG
			Serial.print(F("SIM800L:: "));
			Serial.print(F("HTTP Status code was:"));
			Serial.println(responseStatus);
		#endif
	}

  // close the GPRS connection and reset the timer to short
	doCommand_P(CMD_SAPBR0, NULL);
	timer(SHORT);
	return NULL;
}


