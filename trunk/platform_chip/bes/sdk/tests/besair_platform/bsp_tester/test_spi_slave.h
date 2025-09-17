#ifndef __TEST_SPI_SLAVE_H__
#define __TEST_SPI_SLAVE_H__

#define DUMP_CRASH_END  0x01

void xtc_spi_slave_write(uint8_t *buf, uint32_t len);
void xtc_spi_slave_dump_crash(uint8_t *buf, uint32_t len, uint8_t status);

#endif