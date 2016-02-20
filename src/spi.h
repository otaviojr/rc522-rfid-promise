#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_INFO    2
#define LOG_LEVEL_DEBUG   3

extern const char *spi_dev_path;
extern int log_enabled;

#ifdef __cplusplus
extern "C" {
#endif

void rc522_log(int log_level, const char* message);
int spi_open();
int spi_close();
int spi_read(void* buf, int size);
int spi_write(void* buf ,int size);

#ifdef __cplusplus
}
#endif
