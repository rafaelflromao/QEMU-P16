/*
 * QEMU P16 loader helpers
 *
 * Copyright (c) 2025-2026 Rafael Romão
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qemu/osdep.h"
#include "qemu/datadir.h"
#include "hw/core/loader.h"
#include "elf.h"
#include "boot.h"
#include "qemu/error-report.h"

bool p16_load_firmware(P16CPU *cpu, MachineState *ms,
                       MemoryRegion *program_mr, const char *firmware)
{
    g_autofree char *filename = NULL;
    ssize_t bytes_loaded;
    uint64_t entry = 0;

    /*
     * Resolve the firmware file path.
     * QEMU will search the current working directory first, and then
     * fallback to the configured system BIOS/data directories.
     */
    filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, firmware);
    if (filename == NULL) {
        error_report("Unable to find %s", firmware);
        return false;
    }

    /*
     * Attempt 1: Load as an ELF executable.
     * This is the preferred method because ELF files contain execution metadata.
     * We strictly validate that the file is Little-Endian (ELFDATA2LSB) and
     * matches our specific P16 architecture ID (EM_P16).
     */
    bytes_loaded = load_elf(filename, NULL, NULL, NULL,
                               &entry, NULL, NULL,
                               NULL, ELFDATA2LSB, EM_P16, 0, 0);
                               
    if (bytes_loaded >= 0) {
        /*
         * ELF successfully loaded. The ELF header provides the exact entry point.
         * We override the CPU's default reset Program Counter with this address.
         */
        cpu->env.regs[PC_REG] = entry;
    } else {
        /*
         * Attempt 2: Fallback to Raw Binary (.bin).
         * If the file lacks ELF magic, we treat it as a raw memory dump and
         * inject it directly into the base of the target MemoryRegion.
         * Note: Raw binaries lack metadata, so the PC is NOT modified here; 
         * it relies entirely on the CPU's hardware reset vector (e.g., 0x0000).
         */
        bytes_loaded = load_image_mr(filename, program_mr);
    }

    if (bytes_loaded < 0) {
        error_report("Unable to load firmware image %s as ELF or raw binary",
                     firmware);
        return false;
    }

    return true;
}