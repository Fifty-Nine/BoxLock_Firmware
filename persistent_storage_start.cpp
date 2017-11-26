/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */
#include "driver_init.h"
#include "nv.h"
#include "string.h"
#include "nv_storage.h"

/**
 * \brief Initialize Persistent Storage
 */
void persistent_storage_init(void)
{
    nv_init(&FLASH_0);
    nv_storage_init();
}

