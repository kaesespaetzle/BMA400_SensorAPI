/**
 * Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
 *
 * BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bma400.h"
#include "coines.h"

/*! Read write length varies based on user requirement */
#define READ_WRITE_LENGTH  UINT8_C(46)

/*! BMA400 shuttle board ID */
#define BMA400_SHUTTLE_ID  UINT16_C(0x1A1)

/* Variable to store the device address */
static uint8_t dev_addr;

/*!
 * @brief I2C read function map to COINES platform
 */
BMA400_INTF_RET_TYPE bma400_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;

    return coines_read_i2c(COINES_I2C_BUS_0,dev_addr, reg_addr, reg_data, (uint16_t)len);
}

/*!
 * @brief I2C write function map to COINES platform
 */
BMA400_INTF_RET_TYPE bma400_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;

    return coines_write_i2c(COINES_I2C_BUS_0,dev_addr, reg_addr, (uint8_t *)reg_data, (uint16_t)len);
}

/*!
 * @brief SPI read function map to COINES platform
 */
BMA400_INTF_RET_TYPE bma400_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;

    return coines_read_spi(COINES_SPI_BUS_0,dev_addr, reg_addr, reg_data, (uint16_t)len);
}

/*!
 * @brief SPI write function map to COINES platform
 */
BMA400_INTF_RET_TYPE bma400_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;

     return coines_write_spi(COINES_SPI_BUS_0,dev_addr, reg_addr, (uint8_t *)reg_data, (uint16_t)len);
}

/*!
 * @brief Delay function map to COINES platform
 */
void bma400_delay_us(uint32_t period, void *intf_ptr)
{
    coines_delay_usec(period);
}

void bma400_check_rslt(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BMA400_OK:

            /* Do nothing */
            break;
        case BMA400_E_NULL_PTR:
            printf("API : %s Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BMA400_E_COM_FAIL:
            printf("API : %s Error [%d] : Communication failure\r\n", api_name, rslt);
            break;
        case BMA400_E_INVALID_CONFIG:
            printf("API : %s Error [%d] : Invalid configuration\r\n", api_name, rslt);
            break;
        case BMA400_E_DEV_NOT_FOUND:
            printf("API : %s Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        default:
            printf("API : %s Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}

int8_t bma400_interface_init(struct bma400_dev *bma400, uint8_t intf)
{
    int8_t rslt = BMA400_OK;
    struct coines_board_info board_info;

    if (bma400 != NULL)
    {
        int16_t result = coines_open_comm_intf(COINES_COMM_INTF_USB,NULL);

        if (result < COINES_SUCCESS)
        {
            printf(
                "\n Unable to connect with Application Board ! \n" " 1. Check if the board is connected and powered on. \n" " 2. Check if Application Board USB driver is installed. \n"
                " 3. Check if board is in use by another application. (Insufficient permissions to access USB) \n");
            exit(result);
        }

        rslt = coines_get_board_info(&board_info);

#if defined(PC)
        setbuf(stdout, NULL);
#endif

        if (rslt == COINES_SUCCESS)
        {
            if ((board_info.shuttle_id != BMA400_SHUTTLE_ID))
            {
                printf("! Warning invalid sensor shuttle \n ," "This application will not support this sensor \n");
                exit(COINES_E_FAILURE);
            }
        }

        coines_set_shuttleboard_vdd_vddio_config(0, 0);
        coines_delay_msec(100);

        /* Bus configuration : I2C */
        if (intf == BMA400_I2C_INTF)
        {
            printf("I2C Interface \n");

            dev_addr = BMA400_I2C_ADDRESS_SDO_LOW;
            bma400->read = bma400_i2c_read;
            bma400->write = bma400_i2c_write;
            bma400->intf = BMA400_I2C_INTF;
            coines_config_i2c_bus(COINES_I2C_BUS_0, COINES_I2C_STANDARD_MODE);
        }
        /* Bus configuration : SPI */
        else if (intf == BMA400_SPI_INTF)
        {
            printf("SPI Interface \n");

            dev_addr = COINES_SHUTTLE_PIN_7;
            bma400->read = bma400_spi_read;
            bma400->write = bma400_spi_write;
            bma400->intf = BMA400_SPI_INTF;
            coines_config_spi_bus(COINES_SPI_BUS_0, COINES_SPI_SPEED_7_5_MHZ, COINES_SPI_MODE0);
        }

        bma400->intf_ptr = &dev_addr;
        bma400->delay_us = bma400_delay_us;
        bma400->read_write_len = READ_WRITE_LENGTH;

        coines_delay_msec(100);

        coines_set_shuttleboard_vdd_vddio_config(3300, 3300);

        coines_delay_msec(200);
    }
    else
    {
        rslt = BMA400_E_NULL_PTR;
    }

    return rslt;
}

void bma400_coines_deinit(void)
{
    fflush(stdout);

    coines_set_shuttleboard_vdd_vddio_config(0, 0);
    coines_delay_msec(100);

    /* Coines interface reset */
    coines_soft_reset();
    coines_delay_msec(100);

    coines_close_comm_intf(COINES_COMM_INTF_USB,NULL);
}
