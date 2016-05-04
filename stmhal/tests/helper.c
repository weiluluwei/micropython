/*
 * helper.c
 *
 *  Created on: 16.03.2016
 *      Author: badi
 */
#include <stdint.h>
#include <stdio.h>
#include "helper.h"

void write_le_64(uint64_t value, uint8_t *buf, uint16_t buf_size)
{

    if (buf_size<sizeof(value))
    {
        return;
    }

    for (uint16_t i=0;i<sizeof(value);i++)
    {
        buf[i] = (value>>(i*8));
        //printf("0x%016lx %d 0x%02x\n", value, i, buf[i]);
    }

    return;
}

void write_le_32(uint32_t value, uint8_t *buf, uint16_t buf_size)
{

    if (buf_size<sizeof(value))
    {
        return;
    }

    for (uint16_t i=0;i<sizeof(value);i++)
    {
        buf[i] = (value>>(i*8));
        //printf("0x%08lx %d 0x%02x\n", value, i, buf[i]);
    }

    return;
}

void write_le_16(uint16_t value, uint8_t *buf, uint16_t buf_size)
{

    if (buf_size<sizeof(value))
    {
        return;
    }

    for (uint16_t i=0;i<sizeof(value);i++)
    {
        buf[i] = (value>>(i*8));
        //printf("0x%08lx %d 0x%02x\n", value, i, buf[i]);
    }

    return;
}

void write_be_64(uint64_t value, uint8_t *buf, uint16_t buf_size)
{

    if (buf_size<sizeof(value))
    {
        return;
    }

    for (uint16_t i=0;i<sizeof(value);i++)
    {
        buf[i] = (value>>((sizeof(value)-i-1)*8));
        //printf("0x%08lx %d 0x%02x\n", value, i, buf[i]);
    }


    return;
}

void write_be_32(uint32_t value, uint8_t *buf, uint16_t buf_size)
{

    if (buf_size<sizeof(value))
    {
        return;
    }

    for (uint16_t i=0;i<sizeof(value);i++)
    {
        buf[i] = (value>>((sizeof(value)-i-1)*8));
        //printf("0x%08lx %d 0x%02x\n", value, i, buf[i]);
    }


    return;
}

void write_be_16(uint16_t value, uint8_t *buf, uint16_t buf_size)
{

    if (buf_size<sizeof(value))
    {
        return;
    }

    for (uint16_t i=0;i<sizeof(value);i++)
    {
        buf[i] = (value>>((sizeof(value)-i-1)*8));
        //printf("0x%08lx %d 0x%02x\n", value, i, buf[i]);
    }


    return;
}

