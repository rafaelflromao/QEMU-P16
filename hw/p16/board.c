/*
 * QEMU P16 Machine implementation
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
#include "qemu/error-report.h"
#include "qom/object.h"
#include "system/address-spaces.h"
#include "system/device_tree.h"
#include "target/p16/cpu.h"
#include "hw/core/qdev-properties.h"
#include "hw/core/sysbus.h"
#include "boot.h"
#include "qemu/units.h"

/* --- Machine State Definition --- */

/**
 * P16MachineState:
 * @parent_obj: Base QEMU machine state.
 * @cpu: Pointer to the single P16 processor instance on this board.
 */
struct P16MachineState {
    MachineState parent_obj;
    P16CPU *cpu;
};

typedef struct P16MachineState P16MachineState;

#define TYPE_P16_MACHINE MACHINE_TYPE_NAME("p16-generic")
#define TYPE_P16_TEST_MACHINE MACHINE_TYPE_NAME("p16-testboard")
DECLARE_INSTANCE_CHECKER(P16MachineState, P16_MACHINE, TYPE_P16_MACHINE)


/* --- Initialization & Hardware Wiring --- */

/**
 * p16_machine_init:
 * @machine: The generic machine state provided by QEMU's VL (main loop).
 *
 * Assembles the virtual hardware. This board uses a modern Data-Driven
 * design, meaning it relies heavily on a Device Tree Blob (DTB) to map
 * memory and peripherals, rather than hardcoding addresses in C.
 */
static void p16_machine_base_init(MachineState *machine) {
    P16MachineState *s = P16_MACHINE(machine);
    MemoryRegion *sysmem = get_system_memory();
    MemoryRegion *ram = g_new(MemoryRegion, 1);

    /*
     * 1. Instantiate the CPU
     * Creates and realizes the processor based on the user's -cpu flag
     * (or the board's default). error_fatal ensures QEMU aborts if this fails.
     */
    s->cpu = P16_CPU(object_new(machine->cpu_type));
    qdev_realize(DEVICE(s->cpu), NULL, &error_fatal);

    /*
     * 2. Create RAM and add it to SysMem bus
     */
    if (machine->ram_size) {
        uint64_t ram_size = machine->ram_size;
        memory_region_init_ram(ram, NULL, "p16.ram", ram_size, &error_fatal);
        memory_region_add_subregion(sysmem, 0x0000, ram);
    }

    /*
     * 3. Inject the Firmware/Program
     * Loads the compiled P16 binary into the target's memory space so the
     * CPU has instructions to fetch when it comes out of reset.
     */
    if (machine->firmware) {
        if (!p16_load_firmware(s->cpu, machine, sysmem, machine->firmware)) {
            error_report("Critical error: Failed to load firmware file '%s'",
                         machine->firmware);
            exit(1);
        }
    }
}

static void p16_machine_test_init(MachineState *machine) {
    p16_machine_base_init(machine);

    DeviceState *test_dev = qdev_new("p16-test");
    sysbus_realize_and_unref(SYS_BUS_DEVICE(test_dev), &error_fatal);

    sysbus_mmio_map(SYS_BUS_DEVICE(test_dev), 0, 0xFF00);
}


/* --- QOM Class Setup --- */

/**
 * p16_machine_class_init:
 *
 * Defines the capabilities and restrictions of this specific board.
 */
static void p16_machine_class_init(ObjectClass *oc, const void *data) {
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Generic P16 Board";
    mc->init = p16_machine_base_init;
    mc->default_cpu_type = TYPE_P16_CPU;

    /* Hardware restrictions for this microcontroller board */
    mc->max_cpus = 1;
    mc->no_floppy = 1;
    mc->no_cdrom = 1;
    mc->no_parallel = 1;

    /* Default 32 KiB Memory Size */
    mc->default_ram_size = 32 * KiB;

}

/**
 * p16_testboard_machine_class_init:
 *
 * Defines the capabilities and restrictions of this specific board.
 */
static void p16_testboard_machine_class_init(ObjectClass *oc, const void *data) {
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "P16 Testboard";
    mc->init = p16_machine_test_init;
}

static const TypeInfo p16_machine_types[] = {
    {
        .name = TYPE_P16_MACHINE,
        .parent = TYPE_MACHINE,
        .instance_size = sizeof(P16MachineState),
        .class_init = p16_machine_class_init,
    },
    {
        .name = TYPE_P16_TEST_MACHINE,
        .parent = TYPE_P16_MACHINE,
        .class_init = p16_testboard_machine_class_init,
    },
};

DEFINE_TYPES(p16_machine_types)
