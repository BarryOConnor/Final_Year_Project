#ifndef WSSIM800L_h
#define WSSIM800L_h

#ifndef DEBUG
#define DEBUG true
#endif

#include <AltSoftSerial.h>

#ifndef SIM_RESET_PIN
#define SIM_RESET_PIN 3
#endif

//timer
#define ONE_SECOND 1000000
#define SHORT_SECONDS 1
#define LONG_SECONDS 5
#define DEFAULT_BUFFER_SIZE 200
#define MAX_RETRIES 100
#define NUM_RESPONSES 3
#define TIMEOUT "TIMED OUT"
#define HTTPREAD_COMMAND_LENGTH 14

enum TimerType {BEGIN, ELAPSED, RENEW, SHORT, LONGER};


class WSSIM800L {
private:
    char buffer[DEFAULT_BUFFER_SIZE];
    unsigned long maxTimeout;
    AltSoftSerial* simSerial;
    bool awaitingResponse = false;
    uint8_t responseStatus = 0;
    uint8_t responseLength = 0;

    bool timer(TimerType type);
    bool doCommand_P(const char* command, const char* expected = NULL, const char* parameter = NULL, int8_t retries = 0);
    bool doCommand(const char* command, const char* expected = NULL, const char* parameter = NULL, bool hasQuotes = false, int8_t retries = 0);
    void splitResponse();  
    void sleep();
    void wake();   

public:
    WSSIM800L(int baudrate);
    
    bool reset();
    bool isReady();  
    void readSerialToBuffer();
    void clearBuffer();    
    bool checkResponse(const char* expected);
    bool sendSMS(const char* mobile, const char* message);
    const char* httpGet(const char* apn, const char* url);

};
#endif
