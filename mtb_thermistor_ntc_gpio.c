/**************************************************************************//**
 * \file mtb_thermistor_ntc_gpio.c
 *
 * Description: This file contains the functions for interacting with the
 *              thermistor.
 *
 *******************************************************************************
 * \copyright
 * Copyright 2018-2020 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#include "mtb_thermistor_ntc_gpio.h"
#include <math.h>


#if defined(__cplusplus)
extern "C"
{
#endif

/* Zero Kelvin in degree C */
#define ABSOLUTE_ZERO            (float)(-273.15)

cy_rslt_t mtb_thermistor_ntc_gpio_init(mtb_thermistor_ntc_gpio_t *obj, cyhal_adc_t *adc, cyhal_gpio_t gnd, cyhal_gpio_t vdd, cyhal_gpio_t out, mtb_thermistor_ntc_gpio_cfg_t *cfg)
{
    CY_ASSERT(adc != NULL);
    CY_ASSERT(obj != NULL);
    CY_ASSERT(cfg != NULL);
    memset(obj, 0, sizeof(mtb_thermistor_ntc_gpio_t));

    obj->cfg = cfg;

    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cyhal_gpio_init(vdd, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == result)
        obj->vdd = vdd;

    result = cyhal_gpio_init(gnd, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == result)
        obj->gnd = gnd;

    result = cyhal_adc_channel_init(&obj->channel, adc, out);
    if (CY_RSLT_SUCCESS == result)
        obj->out = out;

    if (CY_RSLT_SUCCESS != result)
        mtb_thermistor_ntc_gpio_free(obj);

    return result;
}

float mtb_thermistor_ntc_gpio_get_temp(mtb_thermistor_ntc_gpio_t *obj)
{
    /* Measure voltage drop on the reference resistor */
    cyhal_gpio_write(obj->vdd, 0u);
    cyhal_gpio_write(obj->gnd, 1u);
    uint16_t voltage_ref = cyhal_adc_read_u16(&obj->channel);

    /* Measure voltage drop on the thermistor */
    cyhal_gpio_write(obj->vdd, 1u);
    cyhal_gpio_write(obj->gnd, 0u);
    uint16_t voltage_therm = cyhal_adc_read_u16(&obj->channel);

    /* Disable power to thermistor circuit */
    cyhal_gpio_write(obj->vdd, 0u);

    /* Calculate thermistor resistance */
    float rThermistor = (obj->cfg->r_ref * (voltage_therm)) / ((float)(voltage_ref));

    /* Calculate thermistor temperature */
    float temperature = (obj->cfg->b_const / (logf(rThermistor / obj->cfg->r_infinity))) + ABSOLUTE_ZERO;

    /* Return the temperature value */
    return temperature;
}

void mtb_thermistor_ntc_gpio_free(mtb_thermistor_ntc_gpio_t *obj)
{
    if(obj->vdd != NC)
        cyhal_gpio_free(obj->vdd);
    if(obj->gnd != NC)
        cyhal_gpio_free(obj->gnd);
    if(obj->out != NC)
    {
        cyhal_adc_channel_free(&obj->channel);
        cyhal_gpio_free(obj->out);
    }
}

#if defined(__cplusplus)
}
#endif
