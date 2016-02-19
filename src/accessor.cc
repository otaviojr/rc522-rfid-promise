#include <node.h>
#include <v8.h>
#include <unistd.h>

#include "spi.h"
#include "rfid.h"
#include "rc522.h"

uint8_t initRfidReader(const char*);

char statusRfidReader;
uint16_t CType=0;
uint8_t serialNumber[10];
uint8_t serialNumberLength=0;
uint8_t noTagFoundCount=0;
char rfidChipSerialNumber[23];
char rfidChipSerialNumberRecentlyDetected[23];
char *p;
int loopCounter;

static char* spi_dev_path;

using namespace v8;

void RunCallback(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate=Isolate::GetCurrent();
        HandleScope scope(isolate);

				rc522_log(LOG_LEVEL_DEBUG,"Enter RunCallback\n");

        if (args.Length() < 2) {
          isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Wrong number of arguments")));
            return;
         }

         Local<Function> callback = Local<Function>::Cast(args[0]);
         const unsigned argc = 1;

        //Check the argument types
        if (!args[1]->IsString()) {
          isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Wrong arguments")));
            return;
        }

        v8::String::Utf8Value spi_path(args[1]->ToString());
        spi_dev_path = *spi_path;
        initRfidReader(spi_dev_path);

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
				rc522_log(LOG_LEVEL_DEBUG,"Exit RunCallback\n");
}

void Init(Handle<Object> exports, Handle<Object> module) {
				rc522_log(LOG_LEVEL_DEBUG,"Enter Init\n");
        NODE_SET_METHOD(module,"exports",RunCallback);
				rc522_log(LOG_LEVEL_DEBUG,"Exit Init\n");
}

uint8_t initRfidReader(const char* spi_dev) {
    return spi_open(spi_dev);
}

NODE_MODULE(rc522, Init)
