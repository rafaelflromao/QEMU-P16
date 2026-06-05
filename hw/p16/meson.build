p16_ss = ss.source_set()
p16_ss.add(files('boot.c'))
p16_ss.add(when: 'CONFIG_P16_BOARD', if_true: files('board.c'))
hw_arch += {'p16': p16_ss}