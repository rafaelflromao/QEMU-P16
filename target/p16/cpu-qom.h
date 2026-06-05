/*
 * QEMU P16 CPU QOM Header
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
#ifndef TARGET_P16_CPU_QOM_H
#define TARGET_P16_CPU_QOM_H

#include "hw/core/cpu.h"

#define TYPE_P16_CPU "p16-cpu"

OBJECT_DECLARE_CPU_TYPE(P16CPU, P16CPUClass, P16_CPU)

/**
 * For future CPU variants
 */
#define P16_CPU_TYPE_SUFFIX "-" TYPE_P16_CPU
#define P16_CPU_TYPE_NAME(name) (name P16_CPU_TYPE_SUFFIX)

#endif