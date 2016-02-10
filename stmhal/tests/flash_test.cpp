/*
 * test_flash.c
 *
 *  Created on: 09.02.2016
 *      Author: badi
 */
#include <stdint.h>
#include <stdio.h>
#include "helper.h"
extern "C"
{
    #include STM32_HAL_H
    #include "flash.h"
}
typedef struct {
    uint32_t base_address;
    uint32_t sector_size;
    uint32_t sector_count;
} flash_organization_t;

#if defined(MCU_SERIES_F4)
static flash_organization_t flashDesc[]= {
        {((uint32_t)0x08000000), 16384,   4 },
        {((uint32_t)0x08010000), 0x10000, 1 },
        {((uint32_t)0x08020000), 0x20000, 3 },
#if defined(FLASH_SECTOR_8)
        {((uint32_t)0x08080000), 0x20000, 4 },
#endif
#if defined(FLASH_SECTOR_12)
        {((uint32_t)0x08100000), 16384,   4 },
        {((uint32_t)0x08110000), 0x10000, 1 },
        {((uint32_t)0x08120000), 0x20000, 7 }
#endif
};
#elif defined(MCU_SERIES_F7)
static flash_organization_t flashDesc[]= {
        {((uint32_t)0x08000000), 32768,   4 },
        {((uint32_t)0x08020000), 0x20000, 1 },
        {((uint32_t)0x08040000), 0x40000, 3 },
};
#elif defined(MCU_SERIES_L4)
static flash_organization_t flashDesc[]= {
        {((uint32_t)0x08000000), 2048,  512 }
};
#else
#error "UNKONWN Processor"
#endif
static uint32_t flashDesc_size = NUM_ELEMENTS(flashDesc);
//CppUTest includes should be after your and system includes
#include "CppUTest/TestHarness.h"

TEST_GROUP(FlashAddressMappingTestGroup)
{

};
/*
 * Test if addresses lower than the Flash address return the correct return value
 */
TEST(FlashAddressMappingTestGroup, CheckLowerFlashBoundary)
{
    const uint32_t DEFAULT_ADR  = 0xFFFFFFFFu;
    const uint32_t DEFAULT_SIZE = 0xFFFFFFF0u;
    uint32_t myAddr;
    uint32_t ueberhang  =30;
    const uint32_t flash_page = 0;

    for (myAddr=flashDesc[0].base_address-ueberhang;myAddr<flashDesc[0].base_address+ueberhang;myAddr++)
    {
        uint32_t start_addr = DEFAULT_ADR;
        uint32_t size = DEFAULT_SIZE;
        uint32_t sec_number = flash_get_sector_info(myAddr, &start_addr, &size);

        if (myAddr < flashDesc[0].base_address)
        {
            CHECK_EQUAL(flash_page, sec_number);
            CHECK_EQUAL(DEFAULT_ADR, start_addr);
            CHECK_EQUAL(DEFAULT_SIZE, size );
        }
        else
        {
            CHECK_EQUAL(flash_page, sec_number);
            CHECK_EQUAL(flashDesc[0].base_address, start_addr);
            CHECK_EQUAL(flashDesc[0].sector_size, size );
        }
    }
}
/*
 * Test if all addresses in flash are mapped to the
 * correct flash sector
 */
TEST(FlashAddressMappingTestGroup, CheckAdressSectorMapping)
{
    const uint32_t DEFAULT_ADR  = 0xFFFFFFFFu;
    const uint32_t DEFAULT_SIZE = 0xFFFFFFF0u;
    uint32_t myAddr;
    uint32_t flash_table_entry;
    uint32_t flash_page=0;

    for (flash_table_entry = 0; flash_table_entry<flashDesc_size; flash_table_entry++)
    {
        uint32_t idx;
        //printf("Flash tabel entry %d\n", flash_table_entry);
        for (idx = 0; idx<flashDesc[flash_table_entry].sector_count; idx++)
        {
            uint32_t firstSecAddr = flashDesc[flash_table_entry].base_address+idx*flashDesc[flash_table_entry].sector_size;
            uint32_t lastSecAddr = firstSecAddr+flashDesc[flash_table_entry].sector_size-1;

            //printf("  Flash idx %d\n", idx);
            for (myAddr=firstSecAddr; myAddr<=lastSecAddr;myAddr++)
            {
                uint32_t start_addr  =   DEFAULT_ADR;
                uint32_t size = DEFAULT_SIZE;
                uint32_t sec_number = flash_get_sector_info(myAddr, &start_addr, &size);

                //printf("    Addr 0x%08x in [0x%08x, 0x%08x]\n", myAddr, firstSecAddr, lastSecAddr);
                CHECK_EQUAL(flash_page, sec_number);
                CHECK_EQUAL(firstSecAddr, start_addr);
                CHECK_EQUAL(flashDesc[flash_table_entry].sector_size, size );
            }
            flash_page++;
        }
    }
}
/*
 * Test if addresses higher than the Flash address return the correct return value
 */
TEST(FlashAddressMappingTestGroup, CheckUpperFlashBoundary)
{
    const uint32_t DEFAULT_ADR  = 0xFFFFFFFFu;
    const uint32_t DEFAULT_SIZE = 0xFFFFFFF0u;
    uint32_t myAddr;
    uint32_t ueberhang  =32;
    uint32_t boundary = flashDesc[flashDesc_size-1].base_address+flashDesc[flashDesc_size-1].sector_size*flashDesc[flashDesc_size-1].sector_count;
    uint32_t sec_base_addr = flashDesc[flashDesc_size-1].base_address+flashDesc[flashDesc_size-1].sector_size*(flashDesc[flashDesc_size-1].sector_count-1);
    uint32_t flash_page = 0;

    for (uint32_t idx = 0; idx<flashDesc_size; idx++)
    {
        flash_page+=flashDesc[idx].sector_count;
    }
    flash_page--;

    for (myAddr=boundary-ueberhang;myAddr<boundary+ueberhang;myAddr++)
    {
        uint32_t start_addr = DEFAULT_ADR;
        uint32_t size = DEFAULT_SIZE;
        uint32_t sec_number = flash_get_sector_info(myAddr, &start_addr, &size);

        //printf("    Addr 0x%08x >= 0x%08x (0x%08x)\n", myAddr, boundary, sec_base_addr);

        if (myAddr >= boundary)
        {
            /*
             * start_addr and size are not modified.
             */
            CHECK_EQUAL(0, sec_number);
            CHECK_EQUAL(DEFAULT_ADR, start_addr);
            CHECK_EQUAL(DEFAULT_SIZE, size );
        }
        else
        {
            CHECK_EQUAL(flash_page, sec_number);
            CHECK_EQUAL(sec_base_addr, start_addr);
            CHECK_EQUAL(flashDesc[flashDesc_size-1].sector_size, size);
        }
    }
}
