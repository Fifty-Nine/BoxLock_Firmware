/**
 * \file
 *
 * \brief Non-volatile level implementation.
 *
 * Copyright (C) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <nv.h>
#include <hal_flash.h>
#include <utils_assert.h>
#include <utils.h>
#include <nv_storage_config.h>
#include <string.h>

static struct flash_descriptor *flash;

#define NV_MEMORY_END (CONF_STORAGE_MEMORY_START + CONF_SECTOR_SIZE * SECTOR_AMOUNT) - 1

/**
 * \brief Initialize non-volatile storage
 */
void nv_init(void *descr)
{
	flash = (struct flash_descriptor *)descr;

	/* Storage must start at row start */
	ASSERT((CONF_STORAGE_MEMORY_START % (flash_get_page_size(flash) * 4)) == 0);
	/* Sector must be of integer number of rows */
	ASSERT((CONF_SECTOR_SIZE % ((flash_get_page_size(flash) * 4)) == 0));
}

/**
 * \brief Write data to non-volatile storage
 */
void nv_write(const uint8_t sector, const uint16_t offset, const uint8_t *const data, const uint16_t size)
{
	uint32_t address = CONF_STORAGE_MEMORY_START + sector * CONF_SECTOR_SIZE + offset;

	/* Write access must start and end within the storage and must not write
	   more than a sector. */
	ASSERT((address <= NV_MEMORY_END) && ((address + size) <= (NV_MEMORY_END + 1))
	       && ((offset + size) <= CONF_SECTOR_SIZE));

	flash_append(flash, address, (uint8_t *)data, size);
}

/**
 * \brief Read data from non-volatile storage
 */
void nv_read(const uint8_t sector, const uint16_t offset, uint8_t *const data, const uint16_t size)
{
	uint32_t address = CONF_STORAGE_MEMORY_START + sector * CONF_SECTOR_SIZE + offset;

	/* Read access must start and end within the storage and must not read more
	   than a sector. */
	ASSERT((address <= NV_MEMORY_END) && ((address + size) <= (NV_MEMORY_END + 1))
	       && ((offset + size) <= CONF_SECTOR_SIZE));

	flash_read(flash, address, data, size);
}

/**
 * \brief Erase a sector
 */
void nv_erase_sector(uint8_t sector)
{
	uint32_t address = CONF_STORAGE_MEMORY_START + sector * CONF_SECTOR_SIZE;

	ASSERT(address <= NV_MEMORY_END);

	flash_erase(flash, address, CONF_SECTOR_SIZE / flash_get_page_size(flash));
}

/**
 * \brief Compare given data and data stored in non-volatile storage
 */
bool nv_is_equal(const uint8_t sector, const uint16_t offset, const uint8_t *const data, const uint16_t size)
{
	uint8_t  tmp[64];
	uint16_t pos     = 0;
	uint32_t address = CONF_STORAGE_MEMORY_START + sector * CONF_SECTOR_SIZE + offset;

	/* Read access must start and end within the storage and must not read more
	   than a sector. There must be at least one byte to compare to. */
	ASSERT((address <= NV_MEMORY_END) && ((address + size) <= (NV_MEMORY_END + 1))
	       && ((offset + size) <= CONF_SECTOR_SIZE)
	       && size);

	while (pos != size) {
		uint8_t cur_size = min(64, size - pos);

		flash_read(flash, address + pos, tmp, cur_size);
		if (memcmp(data + pos, tmp, cur_size)) {
			return false;
		}

		pos += cur_size;
	}

	return true;
}

/**
 * \brief Check if given area is empty
 */
bool nv_is_empty(const uint8_t sector, const uint16_t offset, const uint16_t size)
{
	uint8_t  tmp[64];
	uint16_t pos     = 0;
	uint32_t address = CONF_STORAGE_MEMORY_START + sector * CONF_SECTOR_SIZE + offset;

	/* Read access must start and end within the storage and must not read more
	   than a sector. There must be at least one byte to compare to. */
	ASSERT((address <= NV_MEMORY_END) && ((address + size) <= (NV_MEMORY_END + 1))
	       && ((offset + size) <= CONF_SECTOR_SIZE)
	       && size);

	memset(tmp, 0xFF, 64);

	while (pos != size) {
		uint8_t cur_size = min(64, size - pos);
		uint8_t i        = 0;

		flash_read(flash, address + pos, tmp, cur_size);
		for (; i < 64; i++) {
			if (tmp[i] != 0xFF) {
				return false;
			}
		}

		pos += cur_size;
	}

	return true;
}

/**
 * \brief Align current point of write to actual based on type of previous write
 *        access
 */
uint16_t nv_align_next_access_address(const uint16_t address, const enum nv_alignment_type type)
{
	switch (type) {
	case SECTOR_HEADER_ALIGNMENT:
		return address;

	case BLOCK_WITH_DATA_ALIGNMENT:
		return (address + 127) & 0xFF80;

	default:
		ASSERT(false);
		return address;
	}
}
