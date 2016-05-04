/*
 * helper.h
 *
 *  Created on: 10.02.2016
 *      Author: badi
 */

#ifndef STMHAL_TESTS_HELPER_H_
#define STMHAL_TESTS_HELPER_H_

#define NUM_ELEMENTS(a) sizeof((a))/sizeof((a[0]))


void write_le_64(uint64_t value, uint8_t *buf, uint16_t buf_size);
void write_le_32(uint32_t value, uint8_t *buf, uint16_t buf_size);
void write_le_16(uint16_t value, uint8_t *buf, uint16_t buf_size);
void write_be_64(uint64_t value, uint8_t *buf, uint16_t buf_size);
void write_be_32(uint32_t value, uint8_t *buf, uint16_t buf_size);
void write_be_16(uint16_t value, uint8_t *buf, uint16_t buf_size);

#endif /* STMHAL_TESTS_HELPER_H_ */
