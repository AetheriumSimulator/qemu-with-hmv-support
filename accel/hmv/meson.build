message('Developing HMV support...')
if config_all_accel.has_key('CONFIG_HMV')
  message('CONFIG_HMV is in config_all_accel, adding files to specific_ss')
  specific_ss.add(files(
    'hmv-all.c',
    'hmv-cpus.c',
    'hmos-fixes.c',
  ))
else
  message('CONFIG_HMV is NOT in config_all_accel!')
endif
