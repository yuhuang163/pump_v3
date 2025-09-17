#ifndef __RPTUN_BEST1600_H__
#define __RPTUN_BEST1600_H__

/*bth to m55 channnel */

/**
 * @brief Initialize BTH2M55 RPTUN
 *
 * This function initializes RPTUN BTH2M55 and returns the result of the operation.
 *
 * @param image_addr The address of the DSP M55 image, if called in dsp m55 itself,
 *                   it should be set to 0;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_bth2m55(uint32_t image_addr);

/**
 * @brief Handle BTH2M55 tx interrupt
 *
 * This function is a callback function that handles the tx interrtup of BTH2M55.
 *
 * @param param A pointer to the data being transmitted
 * @param len The length of the data being transmitted
 */
void bes_bth2m55_tx_handler(const void *param, uint32_t len);

/**
 * @brief Handle BTH2M55 rx interrupt
 *
 * This function is a callback function that handles the rx interrupt of BTH2M55.
 *
 * @param param A pointer to the data being received
 * @param len The length of the data being received
 * @return 0 if the receiving data operation succeeds, otherwise a non-zero value is returned.
 */
unsigned int bes_bth2m55_rx_handler(const void *param, uint32_t len);

/*bth to sensor hub channnel */

/**
 * @brief Initialize sensor hub bin address
 *
 * This function set RPTUN 2SENS load bin address.
 *
 * @param image_addr The address of the sensor image, if called in sensor hub itself,
 *                   it should be set to 0;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
void rptun_set_bth2sens_image_addr(uint32_t image_addr);
/**
 * @brief Initialize bth to sensor hub RPTUN
 *
 * This function initializes RPTUN 2SENS and returns the result of the operation.
 *
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_bth2sens(void);
/**
 * @brief set bth to sensor hub RPTUN file name
 *
 * This function initializes RPTUN 2SENS and returns the result of the operation.
 *
 * @param file_name The file name of the sensor image;
 */
void rptun_set_bth2sens_file_name(const char *file_name);
/**
 * @brief Initialize bth to sensor hub RPTUN
 *
 * This function initializes RPTUN 2SENS and returns the result of the operation.
 *
 * @param void;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_bth2sens_fs(void);

/**
 * @brief Handle bth2sens tx interrupt
 *
 * This function is a callback function that handles the tx interrtup of bth2sens.
 *
 * @param param A pointer to the data being transmitted
 * @param len The length of the data being transmitted
 */
void bes_bth2sens_rx_handler(const void *param, uint32_t len);

/**
 * @brief Handle bth2sens rx interrupt
 *
 * This function is a callback function that handles the rx interrupt of bth2sens.
 *
 * @param param A pointer to the data being received
 * @param len The length of the data being received
 * @return 0 if the receiving data operation succeeds, otherwise a non-zero value is returned.
 */
unsigned int bes_bth2sens_tx_handler(const void *param, uint32_t len);

/*bth to hifi4 hub channnel */

/**
 * @brief Initialize bth to hifi4 RPTUN
 *
 * This function initializes RPTUN BTH2HIFI and returns the result of the operation.
 *
 * @param image_addr The address of the sensor image, if called in hifi4 itself,
 *                   it should be set to 0;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_bth2hifi4(uint32_t image_addr);

/**
 * @brief Handle bth2hifi4 tx interrupt
 *
 * This function is a callback function that handles the tx interrtup of bth2sens.
 *
 * @param param A pointer to the data being transmitted
 * @param len The length of the data being transmitted
 */
unsigned int bes_bth2hifi4_rx_handler(const void *param, uint32_t len);

/**
 * @brief Handle bth2hifi4 rx interrupt
 *
 * This function is a callback function that handles the rx interrupt of bth2hifi4.
 *
 * @param param A pointer to the data being received
 * @param len The length of the data being received
 * @return 0 if the receiving data operation succeeds, otherwise a non-zero value is returned.
 */
unsigned int bes_bth2hifi4_tx_handler(const void *param, uint32_t len);

#endif /* __RPTUN_BEST1600_H__ */
