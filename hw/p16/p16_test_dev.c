/*
 * P16 Test Device
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
#include "qapi/error.h"
#include "hw/core/sysbus.h"
#include "hw/core/qdev-properties.h"
#include "system/runstate.h"

#define TYPE_P16_TEST "p16-test"
OBJECT_DECLARE_SIMPLE_TYPE(P16TestState, P16_TEST)

/**
 * P16TestState:
 * @parent_obj: Base SysBus device.
 * @memory_region: The QEMU memory region mapped to the guest.
 */
struct P16TestState {
    SysBusDevice parent_obj;
    MemoryRegion memory_region;
};

static void p16_test_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                                unsigned size)
{
    if (val == 0xAA) {
        /* PASSED Test: Ask QEMU to cleanly exit with code 0 */
        qemu_system_shutdown_request(SHUTDOWN_CAUSE_GUEST_SHUTDOWN);
    } else if (val == 0xDE) {
        /* FAILED Test: Abort execution immediately with code 1 */
        exit(1);
    }
}

static uint64_t p16_test_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    /* The test device is write-only. Reading returns 0. */
    return 0;
}

static const MemoryRegionOps p16_test_mmio_ops = {
    .read = p16_test_mmio_read,
    .write = p16_test_mmio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 2,
    },
};

/**
 * p16_test_realize:
 *
 * Initializes the 2-byte I/O memory region used as the "Magic MMIO"
 * test finisher, and exposes it via the SysBus interface.
 */
static void p16_test_realize(DeviceState *dev, Error **errp)
{
    P16TestState *s = P16_TEST(dev);

    memory_region_init_io(&s->memory_region, OBJECT(s), &p16_test_mmio_ops,
                          s, "p16.test_device", 2);

    /* Connect the Memory to the device's SysBus interface */
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->memory_region);
}

static void p16_test_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = p16_test_realize;
    dc->desc = "Device for P16 bare-metal testing";
}

static const TypeInfo p16_test_info = {
    .name          = TYPE_P16_TEST,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(P16TestState),
    .class_init    = p16_test_class_init,
};

static void p16_test_register_types(void) {
    type_register_static(&p16_test_info);
}

type_init(p16_test_register_types)