#ifndef __RPTUN_BEST1502X_H__
#define __RPTUN_BEST1502X_H__

/**
 * @brief Initialize cpu to sensor hub RPTUN
 *
 * This function initializes RPTUN CPU2M55 and returns the result of the operation.
 *
 * @param image_addr The address of the sensor image, if called in sensor hub itself,
 *                   it should be set to 0;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_cpu2sens(uint32_t image_addr);

/**
 * @brief Initialize cpu to sensor hub RPTUN
 *
 * This function initializes RPTUN CPU2M55 and returns the result of the operation.
 *
 * @param file_name The file name of the sensor image;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_cpu2sens_fs(const char *file_name);

/**
 * @brief Handle cpu2sens tx interrupt
 *
 * This function is a callback function that handles the tx interrtup of cpu2sens.
 *
 * @param param A pointer to the data being transmitted
 * @param len The length of the data being transmitted
 */
void bes_cpu2sens_rx_handler(const void *param, uint32_t len);

/**
 * @brief Handle cpu2sens rx interrupt
 *
 * This function is a callback function that handles the rx interrupt of cpu2sens.
 *
 * @param param A pointer to the data being received
 * @param len The length of the data being received
 * @return 0 if the receiving data operation succeeds, otherwise a non-zero value is returned.
 */
unsigned int bes_cpu2sens_tx_handler(const void *param, uint32_t len);

#endif /* __RPTUN_BEST1502X_H__ */
