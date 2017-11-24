/**
 * \file
 *
 * \brief Non-volatile level declaration.
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

#ifndef _NV_INCLUDED
#define _NV_INCLUDED

#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The amount of sectors in the storage
 */
#ifndef SECTOR_AMOUNT
#define SECTOR_AMOUNT 2
#endif

enum nv_alignment_type { SECTOR_HEADER_ALIGNMENT, BLOCK_WITH_DATA_ALIGNMENT };

/**
 * \brief Initialize non-volatile storage
 *
 * \param[in] descr Descriptor of a driver or middleware used by NV-storage
 */
void nv_init(void *descr);

/**
 * \brief Write data to non-volatile storage
 *
 * \param[in] sector Sector of the storage to write to
 * \param[in] offset The offset inside the sector to start writing to
 * \param[in] data Data to write
 * \param[in] size The size of the data to write
 */
void nv_write(const uint8_t sector, const uint16_t offset, const uint8_t *const data, const uint16_t size);

/**
 * \brief Read data from non-volatile storage
 *
 * \param[in] sector Sector of the storage to read from
 * \param[in] offset The offset inside the sector to start reading from
 * \param[in] data Data buffer to read data to
 * \param[in] size The size of the data buffer
 */
void nv_read(const uint8_t sector, const uint16_t offset, uint8_t *const data, const uint16_t size);

/**
 * \brief Erase a sector
 *
 * \param[in] sector Sector to erase
 */
void nv_erase_sector(uint8_t sector);

/**
 * \brief Compare given data and data stored in non-volatile storage
 *
 * \param[in] sector Sector of the storage with data to compare to
 * \param[in] offset The offset inside the sector to start comparing from
 * \param[in] data Data to compare to
 * \param[in] size The size of the data buffer
 *
 * \return True if content of non-volatile storage and data buffer matches,
 *	otherwise false
 */
bool nv_is_equal(const uint8_t sector, const uint16_t offset, const uint8_t *const data, const uint16_t size);

/**
 * \brief Check if given area is empty
 *
 * \param[in] sector Sector of the storage to perform the check in
 * \param[in] offset The offset inside the sector to start checking from
 * \param[in] size The size of the data to check
 *
 * \return True if content of non-volatile storage is empty, otherwise false
 */
bool nv_is_empty(const uint8_t sector, const uint16_t offset, const uint16_t size);

/**
 * \brief Align current point of write to actual based on type of previous write
 *        access
 *
 * \param[in] address The address to align
 * \param[in] type The type of access
 *
 * \return Aligned point-of-write (offset from storage base address)
 */
uint16_t nv_align_next_access_address(const uint16_t address, const enum nv_alignment_type type);

#ifdef __cplusplus
}
#endif

#endif /* _NV_INCLUDED */
