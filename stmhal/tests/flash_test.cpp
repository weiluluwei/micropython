/*
 * test_flash.c
 *
 *  Created on: 09.02.2016
 *      Author: badi
 */
#include <stdint.h>
#include <stdio.h>
extern "C"
{
    #include "flash.h"
}

//CppUTest includes should be after your and system includes
#include "CppUTest/TestHarness.h"

TEST_GROUP(FlashTestGroup)
{

};

TEST(FlashTestGroup, GetFirstSector)
{
    uint32_t myAddr = 0x08000000+42;
    uint32_t start_addr;
    uint32_t size;
    uint32_t sec = flash_get_sector_info(myAddr, &start_addr, &size);
    printf("sector 0x%08x", sec);
}
