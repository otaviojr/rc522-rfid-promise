#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spi.h"

#define DEFAULT_SPI_SPEED 48000000L

//#define  SPIDEV            "/dev/spidev1.0"
#define  EXISTMODE         F_OK
#define  MODE              O_RDWR

const char *spi_dev_path;

static uint8_t mode = 0;
static uint8_t bits = 8;
static uint8_t lsb_setting = 0;

static int spi_fd = -1;

int log_enabled = 0;

void rc522_log(int log_level, const char* message){
		if(log_enabled == 1){
			FILE* fp;
			fp = fopen("/tmp/rc522.log","a+");
			if(fp != NULL){
				fputs (message,fp);
			  fclose (fp);
			}
		}
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

	sp=DEFAULT_SPI_SPEED;

	rc522_log(LOG_LEVEL_DEBUG,"SPIDEV: ");
	rc522_log(LOG_LEVEL_DEBUG,spi_dev_path);
	rc522_log(LOG_LEVEL_DEBUG,"\n");

	if(access(spi_dev_path,EXISTMODE) < 0) {
		rc522_log(LOG_LEVEL_ERROR,"SPIDEV not found\n");
		return -1 ;
	}
	if((spi_fd = open(spi_dev_path,MODE)) < 0){
		rc522_log(LOG_LEVEL_ERROR,"SPIDEV not found\n");
		return -1 ;
	}

	/*
	 * spi mode
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set spi mode\n");

	ret = ioctl(spi_fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get spi mode\n");

	/*
	 * bits per word
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set bits per word\n");

	ret = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get bits per word\n");

	/*
		* max speed hz
		*/
	ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &sp);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set max speed hz\n");

	ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &sp);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get max speed hz\n");

  /* MSB first */
  ret = ioctl(spi_fd, SPI_IOC_WR_LSB_FIRST, &lsb_setting);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't set msb first\n");

	ret = ioctl(spi_fd, SPI_IOC_RD_LSB_FIRST, &sp);
	if (ret == -1)
		rc522_log(LOG_LEVEL_ERROR,"can't get msb first\n");

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
