gen = [
    decodetree.process('insn.decode', extra_args: [ '--decode', 'decode_insn',
                                                    '--insnwidth', '16' ])
]
p16_ss = ss.source_set()
p16_system_ss = ss.source_set()

p16_ss.add(gen)
p16_ss.add(files(
               'translate.c',
               'helper.c',
               'cpu.c',
               'gdbstub.c',
               'disas.c'))

p16_system_ss.add(files('machine.c'))

target_arch += {'p16': p16_ss}
target_system_arch += {'p16': p16_system_ss}