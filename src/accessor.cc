#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <unistd.h>

#include <string>

#include "spi.h"
#include "rfid.h"
#include "rc522.h"

char statusRfidReader;
uint16_t CType=0;
uint8_t serialNumber[10];
uint8_t serialNumberLength=0;
uint8_t noTagFoundCount=0;
char rfidChipSerialNumber[23];
char rfidChipSerialNumberRecentlyDetected[23];
char *p;
int loopCounter;

using namespace std;
using namespace v8;

void RunCallback(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate=Isolate::GetCurrent();
        HandleScope scope(isolate);

				rc522_log(LOG_LEVEL_DEBUG,"Enter RunCallback\n");

        if (args.Length() < 1) {
          isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Wrong number of arguments")));
            return;
         }

         //Check the argument types
         if (!args[0]->IsString()) {
           isolate->ThrowException(Exception::TypeError(
             String::NewFromUtf8(isolate, "Wrong arguments")));
             return;
         }

         String::Utf8Value param1(args[0]->ToString());
         std::string path = std::string(*param1);
         spi_dev_path = path.c_str();
				 rc522_log(LOG_LEVEL_DEBUG,"Exit RunCallback\n");
}

void EnableLog(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate=Isolate::GetCurrent();
        HandleScope scope(isolate);

				rc522_log(LOG_LEVEL_DEBUG,"Enter EnableLog\n");

        if (args.Length() < 1) {
          isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Wrong number of arguments")));
            return;
         }

         //return;

         //Check the argument types
         //if (!args[0]->IsNumber()) {
         //  isolate->ThrowException(Exception::TypeError(
        //     String::NewFromUtf8(isolate, "Wrong arguments")));
        //     return;
         //}

         double value = args[0]->NumberValue();
         log_enabled = static_cast<int>(value);

				 rc522_log(LOG_LEVEL_DEBUG,"Exit EnableLog\n");
}

void Read(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate=Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Function> callback = Local<Function>::Cast(args[0]);
    const unsigned argc = 1;

    rc522_log(LOG_LEVEL_DEBUG,"Enter read\n");

    if(spi_open() < 0){
    }
    InitRc522();

    for (;;) {
            statusRfidReader = find_tag(&CType);
            if (statusRfidReader == TAG_NOTAG) {

                    rc522_log(LOG_LEVEL_DEBUG,"TAG Not Found\n");
                    // The status that no tag is found is sometimes set even when a tag is within reach of the tag reader
                    // to prevent that the reset is performed the no tag event has to take place multiple times (ger: entrprellen)
                    if (noTagFoundCount > 2) {
                            // Sets the content of the array 'rfidChipSerialNumberRecentlyDetected' back to zero
                            memset(&rfidChipSerialNumberRecentlyDetected[0], 0, sizeof(rfidChipSerialNumberRecentlyDetected));
                            noTagFoundCount = 0;
                    }
                    else {
                            noTagFoundCount++;
                    }

                    usleep(200000);
                    continue;
            } else if (statusRfidReader != TAG_OK && statusRfidReader != TAG_COLLISION) {
                    rc522_log(LOG_LEVEL_DEBUG,"TAG NOK !COLLISION\n");
                    continue;
            }

            if (select_tag_sn(serialNumber,&serialNumberLength) != TAG_OK) {
                    rc522_log(LOG_LEVEL_DEBUG,"TAG NOK\n");
                    continue;
            }

            // Is a successful detected, the counter will be set to zero
            noTagFoundCount = 0;

            p=rfidChipSerialNumber;
            for (loopCounter = 0; loopCounter < serialNumberLength; loopCounter++) {
                    sprintf(p,"%02x", serialNumber[loopCounter]);
                    p+=2;
            }

            // Only when the serial number of the currently detected tag differs from the
            // recently detected tag the callback will be executed with the serial number
            if(strcmp(rfidChipSerialNumberRecentlyDetected, rfidChipSerialNumber) != 0)
            {
                    rc522_log(LOG_LEVEL_DEBUG,"TAG OK - Entering Callback\n");
                    Local<Value> argv[argc] = {
                            Local<Value>::New(isolate,String::NewFromUtf8(isolate,&rfidChipSerialNumber[0]))
                    };
                    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
                    rc522_log(LOG_LEVEL_DEBUG,"TAG OK - Exit Callback\n");
            }

            // Preserves the current detected serial number, so that it can be used
            // for future evaluations
            strcpy(rfidChipSerialNumberRecentlyDetected, rfidChipSerialNumber);

            *(p++)=0;
    }

    //bcm2835_spi_end();
    //bcm2835_close();
    spi_close();
}

void Init(Handle<Object> exports, Handle<Object> module) {
				rc522_log(LOG_LEVEL_DEBUG,"Enter Init\n");
        NODE_SET_METHOD(exports,"init",RunCallback);
        NODE_SET_METHOD(exports, "enableLog", EnableLog);
        NODE_SET_METHOD(exports, "read", Read);
        rc522_log(LOG_LEVEL_DEBUG,"Exit Init\n");
}

NODE_MODULE(rc522, Init)
