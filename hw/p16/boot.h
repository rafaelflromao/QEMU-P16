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

#ifndef HW_P16_BOOT_H
#define HW_P16_BOOT_H

#include "hw/core/boards.h"
#include "target/p16/cpu.h"

/**
 * p16_load_firmware:   load an image into a memory region
 *
 * @cpu:        A P16 CPU object
 * @ms:         A MachineState
 * @mr:         Memory Region to load into
 * @firmware:   Path to the firmware file (raw binary or ELF format)
 *
 * Load a firmware supplied by the machine or by the user  with the
 * '-bios' command line option, and put it in target memory.
 *
 * Returns: true on success, false on error.
 */
bool p16_load_firmware(P16CPU *cpu, MachineState *ms,
                       MemoryRegion *mr, const char *firmware);

#endif
