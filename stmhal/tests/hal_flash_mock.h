/*
 * hal_flash_mock.h
 *
 *  Created on: 21.03.2016
 *      Author: badi
 */

#ifndef STMHAL_TESTS_HAL_FLASH_MOCK_H_
#define STMHAL_TESTS_HAL_FLASH_MOCK_H_

uint32_t hal_flash_mock_buffersize(void);
uint8_t hal_flash_mock_get8(uint32_t byteAddr);
uint16_t hal_flash_mock_get16(uint32_t byteAddr);
uint64_t hal_flash_mock_get64(uint32_t byteAddr);
void hal_flash_mock_erase(uint8_t val);


#endif /* STMHAL_TESTS_HAL_FLASH_MOCK_H_ */
