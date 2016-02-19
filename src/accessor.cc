#include <node.h>
#include <v8.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "accessor.h"
#include "rfid.h"
#include "rc522.h"

#define DEFAULT_SPI_SPEED 5000L

uint8_t initRfidReader(void);

char statusRfidReader;
uint16_t CType=0;
uint8_t serialNumber[10];
uint8_t serialNumberLength=0;
uint8_t noTagFoundCount=0;
char rfidChipSerialNumber[23];
char rfidChipSerialNumberRecentlyDetected[23];
char *p;
int loopCounter;

#define  SPIDEV            "/dev/spidev1.0"
#define  EXISTMODE         F_OK
#define  MODE              O_RDWR

static uint8_t mode = 0;
static uint8_t bits = 8;
static uint8_t lsb_setting = 0;

static int spi_fd = -1 ;

using namespace v8;

void rc522_log(int log_level, const char* message){
#if LOG_ENABLED == 1
	FILE* fp;
	fp = fopen("/tmp/rc522.log","a+");
	if(fp != NULL){
		fputs (message,fp);
	  fclose (fp);
	}
#endif
}

/*
** Function : spi_open
** Note     : open spidev1.0 node for transfer
** return   : success return open fd else return -1
*/
int spi_open()
{
	uint16_t sp;
	int ret = 0;

	sp=(uint16_t)(250000L / DEFAULT_SPI_SPEED);
	sp=(uint16_t)2048;

	if(access(SPIDEV,EXISTMODE) < 0) return -1 ;
	if((spi_fd = open(SPIDEV,MODE)) < 0) return -1 ;

	/*
	 * spi mode
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set spi mode");

	ret = ioctl(spi_fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set bits per word");

	ret = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get bits per word");

	/*
		* max speed hz
		*/
	ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &sp);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set max speed hz");

	ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &sp);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get max speed hz");

  /* MSB first */
  ret = ioctl(spi_fd, SPI_IOC_WR_LSB_FIRST, &lsb_setting);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set msb first");

	ret = ioctl(spi_fd, SPI_IOC_RD_LSB_FIRST, &sp);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get msb first");

	return spi_fd;
}

/*
** Function : spi_close
** Note     : close spidev1.0 opened fd
** return   : success return 0 else return -1
*/
int spi_close()
{
	if(spi_fd == -1) return -1 ;
	close(spi_fd) ;
	return 0 ;
}


/*
** Function   : spi_read
** Note       : read message frome spidev
** return     : success return read size else return -1
*/

int spi_read(void* buf, int size)
{
	if((spi_fd == -1) || (buf == NULL) || (size <= 0)) return -1 ;
  int size_write = write(spi_fd,buf,1);
	if(size_write < 0) return -1 ;
	int size_read = read(spi_fd,buf,2) ;
	if(size_read < 0) return -1 ;
	return size_read ;
}

/*
** Function   :spi_write
** Note       :write message to spidev
** return     :success return write size else return -1
*/
int spi_write(void* buf ,int size)
{
	if((spi_fd == -1) || (NULL == buf) || (size <= 0)) return -1 ;
	int size_write = write(spi_fd,buf,size) ;
	if(size_write < 0) return -1 ;
	return size_write ;
}

void RunCallback(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate=Isolate::GetCurrent();
        HandleScope scope(isolate);

        Local<Function> callback = Local<Function>::Cast(args[0]);
        const unsigned argc = 1;

        InitRc522();

        for (;; ) {
                statusRfidReader = find_tag(&CType);
                if (statusRfidReader == TAG_NOTAG) {

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
                        continue;
                }

                if (select_tag_sn(serialNumber,&serialNumberLength) != TAG_OK) {
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
                        Local<Value> argv[argc] = {
                                Local<Value>::New(isolate,String::NewFromUtf8(isolate,&rfidChipSerialNumber[0]))
                        };
                        callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
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
        initRfidReader();
        NODE_SET_METHOD(module,"exports",RunCallback);
}

uint8_t initRfidReader(void) {
    return spi_open();
}

NODE_MODULE(rc522, Init)
