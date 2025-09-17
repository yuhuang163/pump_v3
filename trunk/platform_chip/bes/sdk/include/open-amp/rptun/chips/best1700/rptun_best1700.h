#ifndef __RPTUN_BEST1700_H__
#define __RPTUN_BEST1700_H__

/*bth to hifi channnel */

/**
 * @brief Initialize BTH2HIFI RPTUN
 *
 * This function initializes RPTUN BTH2HIFI and returns the result of the operation.
 *
 * @param image_addr The address of the DSP M55 image, if called in dsp m55 itself,
 *                   it should be set to 0;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_bth2hifi4(uint32_t image_addr);

/**
 * @brief Handle BTH2HIFI tx interrupt
 *
 * This function is a callback function that handles the tx interrtup of BTH2HIFI.
 *
 * @param param A pointer to the data being transmitted
 * @param len The length of the data being transmitted
 */
void bes_bth2hifi4_tx_handler(const void *param, uint32_t len);

/**
 * @brief Handle BTH2HIFI rx interrupt
 *
 * This function is a callback function that handles the rx interrupt of BTH2HIFI.
 *
 * @param param A pointer to the data being received
 * @param len The length of the data being received
 * @return 0 if the receiving data operation succeeds, otherwise a non-zero value is returned.
 */
unsigned int bes_bth2hifi4_rx_handler(const void *param, uint32_t len);

/*m55c0 to dspc0 channnel */

/**
 * @brief Initialize M55C0_2_DSPC0 RPTUN
 *
 * This function initializes RPTUN M55C0_2_DSPC0 and returns the result of the operation.
 *
 * @param image_addr The address of the DSP M55 image, if called in dsp m55 itself,
 *                   it should be set to 0;
 * @return 0 if the initialization succeeds, otherwise an error code is returned.
 */
int rptun_init_m55c0_2_dspc0(uint32_t image_addr);
#endif /* __RPTUN_BEST1600_H__ */
