###core P0###
	if int(options.NumCores) == 1 or int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		l1_i_0_total_cache_ctrl_loops = sim_data.getint('FullRunStats', "l1_i_0_TotalCacheCtrlLoops")
		l1_i_0_total_accesses = sim_data.getint('FullRunStats', "l1_i_0_TotalAccesses")
		l1_i_0_total_hits = sim_data.getint('FullRunStats', "l1_i_0_TotalHits")
		l1_i_0_total_misses = sim_data.getint('FullRunStats', "l1_i_0_TotalMisses")
		l1_i_0_miss_rate = sim_data.getfloat('FullRunStats', "l1_i_0_MissRate")
		l1_i_0_total_reads = sim_data.getint('FullRunStats', "l1_i_0_TotalReads")
		l1_i_0_total_read_misses = sim_data.getint('FullRunStats', "l1_i_0_TotalReadMisses")
		l1_i_0_read_miss_rate = sim_data.getfloat('FullRunStats', "l1_i_0_ReadMissRate")
		l1_i_0_total_writes = 0
		l1_i_0_total_write_misses = 0
		l1_i_0_write_miss_rate = 0
		l1_i_0_total_get = 0
		l1_i_0_total_getx = 0
		l1_i_0_get_miss_rate = 0
		l1_i_0_getx_miss_rate = 0
		l1_i_0_total_upgrades = 0
		l1_i_0_upgrade_miss_rate = 0
		l1_i_0_total_write_backs = 0
		l1_i_0_cache_utilization = sim_data.getfloat('FullRunStats', "l1_i_0_CacheUtilization")

		l1_d_0_total_cache_ctrl_loops = sim_data.getint('FullRunStats', "TotalCacheCtrlLoops")
		l1_d_0_total_accesses = sim_data.getint('FullRunStats', "TotalAccesses")
		l1_d_0_total_hits = sim_data.getint('FullRunStats', "TotalHits")
		l1_d_0_total_misses = sim_data.getint('FullRunStats', "TotalMisses")
		l1_d_0_miss_rate = sim_data.getfloat('FullRunStats', "MissRate")
		l1_d_0_total_reads = sim_data.getint('FullRunStats', "TotalReads")
		l1_d_0_total_read_misses = sim_data.getint('FullRunStats', "TotalReadMisses")
		l1_d_0_read_miss_rate = sim_data.getfloat('FullRunStats', "ReadMissRate")
		l1_d_0_total_writes = sim_data.getint('FullRunStats', "TotalWrites")
		l1_d_0_total_write_misses = sim_data.getint('FullRunStats', "TotalWriteMisses")
		l1_d_0_write_miss_rate = sim_data.getfloat('FullRunStats', "WriteMissRate")
		l1_d_0_total_get = sim_data.getint('FullRunStats', "TotalGet")
		l1_d_0_total_getx = sim_data.getint('FullRunStats', "TotalGetx")
		l1_d_0_get_miss_rate = sim_data.getfloat('FullRunStats', "GetMissRate")
		l1_d_0_getx_miss_rate = sim_data.getfloat('FullRunStats', "GetxMissRate")
		l1_d_0_total_upgrades = sim_data.getint('FullRunStats', "TotalUpgrades")
		l1_d_0_upgrade_miss_rate = sim_data.getfloat('FullRunStats', "UpgradeMissRate")
		l1_d_0_total_write_backs = sim_data.getint('FullRunStats', "TotalWriteBacks")
		l1_d_0_cache_utilization = sim_data.getfloat('FullRunStats', "CacheUtilization")

		l2_0_total_cache_ctrl_loops = sim_data.getint('FullRunStats', "TotalCacheCtrlLoops")
		l2_0_total_accesses = sim_data.getint('FullRunStats', "TotalAccesses")
		l2_0_total_hits = sim_data.getint('FullRunStats', "TotalHits")
		l2_0_total_misses = sim_data.getint('FullRunStats', "TotalMisses")
		l2_0_miss_rate = sim_data.getfloat('FullRunStats', "MissRate")
		l2_0_total_reads = sim_data.getint('FullRunStats', "TotalReads")
		l2_0_total_read_misses = sim_data.getint('FullRunStats', "TotalReadMisses")
		l2_0_read_miss_rate = sim_data.getfloat('FullRunStats', "ReadMissRate")
		l2_0_total_writes = sim_data.getint('FullRunStats', "TotalWrites")
		l2_0_total_write_misses = sim_data.getint('FullRunStats', "TotalWriteMisses")
		l2_0_write_miss_rate = sim_data.getfloat('FullRunStats', "WriteMissRate")
		l2_0_total_get = sim_data.getint('FullRunStats', "TotalGet")
		l2_0_total_getx = sim_data.getint('FullRunStats', "TotalGetx")
		l2_0_get_miss_rate = sim_data.getfloat('FullRunStats', "GetMissRate")
		l2_0_getx_miss_rate = sim_data.getfloat('FullRunStats', "GetxMissRate")
		l2_0_total_upgrades = sim_data.getint('FullRunStats', "TotalUpgrades")
		l2_0_upgrade_miss_rate = sim_data.getfloat('FullRunStats', "UpgradeMissRate")
		l2_0_total_write_backs = sim_data.getint('FullRunStats', "TotalWriteBacks")
		l2_0_cache_utilization = sim_data.getfloat('FullRunStats', "CacheUtilization")

		l3_0_total_cache_ctrl_loops = sim_data.getint('FullRunStats', "TotalCacheCtrlLoops")
		l3_0_total_accesses = sim_data.getint('FullRunStats', "TotalAccesses")
		l3_0_total_hits = sim_data.getint('FullRunStats', "TotalHits")
		l3_0_total_misses = sim_data.getint('FullRunStats', "TotalMisses")
		l3_0_miss_rate = sim_data.getfloat('FullRunStats', "MissRate")
		l3_0_total_reads = sim_data.getint('FullRunStats', "TotalReads")
		l3_0_total_read_misses = sim_data.getint('FullRunStats', "TotalReadMisses")
		l3_0_read_miss_rate = sim_data.getfloat('FullRunStats', "ReadMissRate")
		l3_0_total_writes = sim_data.getint('FullRunStats', "TotalWrites")
		l3_0_total_write_misses = sim_data.getint('FullRunStats', "TotalWriteMisses")
		l3_0_write_miss_rate = sim_data.getfloat('FullRunStats', "WriteMissRate")
		l3_0_total_get = sim_data.getint('FullRunStats', "TotalGet")
		l3_0_total_getx = sim_data.getint('FullRunStats', "TotalGetx")
		l3_0_get_miss_rate = sim_data.getfloat('FullRunStats', "GetMissRate")
		l3_0_getx_miss_rate = sim_data.getfloat('FullRunStats', "GetxMissRate")
		l3_0_total_upgrades = sim_data.getint('FullRunStats', "TotalUpgrades")
		l3_0_upgrade_miss_rate = sim_data.getfloat('FullRunStats', "UpgradeMissRate")
		l3_0_total_write_backs = sim_data.getint('FullRunStats', "TotalWriteBacks")
		l3_0_cache_utilization = sim_data.getfloat('FullRunStats', "CacheUtilization")

		table_P0 = [
		["TotalCacheCtrlLoops", l1_i_0_total_cache_ctrl_loops, l1_d_0_total_cache_ctrl_loops, l2_0_total_cache_ctrl_loops, l3_0_total_cache_ctrl_loops],
		["TotalAccesses", l1_i_0_total_accesses, l1_d_0_total_accesses, l2_0_total_accesses, l3_0_total_accesses], 
		["TotalHits", l1_i_0_total_hits, l1_d_0_total_hits, l2_0_total_hits, l3_0_total_hits],
		["TotalMisses", l1_i_0_total_misses, l1_d_0_total_misses, l2_0_total_misses, l3_0_total_misses],
		["MissRate", l1_i_0_miss_rate, l1_d_0_miss_rate, l2_0_miss_rate, l3_0_miss_rate],
		["TotalReads", l1_i_0_total_reads, l1_d_0_total_reads, l2_0_total_reads, l3_0_total_reads],
		["TotalReadMisses", l1_i_0_total_read_misses, l1_d_0_total_read_misses, l2_0_total_read_misses, l3_0_total_read_misses],
		["ReadMissRate", l1_i_0_read_miss_rate, l1_d_0_read_miss_rate, l2_0_read_miss_rate, l3_0_read_miss_rate],
		["TotalWrites", l1_i_0_total_writes, l1_d_0_total_writes, l2_0_total_writes, l3_0_total_writes],
		["TotalWriteMisses", l1_i_0_total_write_misses, l1_d_0_total_write_misses, l2_0_total_write_misses, l3_0_total_write_misses],
		["WriteMissRate", l1_i_0_write_miss_rate, l1_d_0_write_miss_rate, l2_0_write_miss_rate, l3_0_write_miss_rate],
		["TotalGet", l1_i_0_total_get, l1_d_0_total_get, l2_0_total_get, l3_0_total_get],
		["TotalGetx", l1_i_0_total_getx, l1_d_0_total_getx, l2_0_total_getx, l3_0_total_getx],
		["GetMissRate", l1_i_0_get_miss_rate, l1_d_0_get_miss_rate, l2_0_get_miss_rate, l3_0_get_miss_rate],
		["GetxMissRate", l1_i_0_getx_miss_rate, l1_d_0_getx_miss_rate, l2_0_getx_miss_rate, l3_0_getx_miss_rate],
		["TotalUpgrades", l1_i_0_total_upgrades, l1_d_0_total_upgrades, l2_0_total_upgrades, l3_0_total_upgrades],
		["UpgradeMissRate", l1_i_0_upgrade_miss_rate, l1_d_0_upgrade_miss_rate, l2_0_upgrade_miss_rate, l3_0_upgrade_miss_rate],
		["TotalWriteBacks", l1_i_0_total_write_backs, l1_d_0_total_write_backs, l2_0_total_write_backs, l3_0_total_write_backs],
		["CacheUtilization", l1_i_0_cache_utilization, l1_d_0_cache_utilization, l2_0_cache_utilization, l3_0_cache_utilization]
		]


	###core P1###
	if int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		l1_i_1_total_cache_ctrl_loops = sim_data.getint("L1_I_Cache_1", "TotalCacheCtrlLoops")
		l1_i_1_total_accesses = sim_data.getint("L1_I_Cache_1", "TotalAccesses")
		l1_i_1_total_hits = sim_data.getint("L1_I_Cache_1", "TotalHits")
		l1_i_1_total_misses = sim_data.getint("L1_I_Cache_1", "TotalMisses")
		l1_i_1_miss_rate = sim_data.getfloat("L1_I_Cache_1", "MissRate")
		l1_i_1_total_reads = sim_data.getint("L1_I_Cache_1", "TotalReads")
		l1_i_1_total_read_misses = sim_data.getint("L1_I_Cache_1", "TotalReadMisses")
		l1_i_1_read_miss_rate = sim_data.getfloat("L1_I_Cache_1", "ReadMissRate")
		l1_i_1_total_writes = 0
		l1_i_1_total_write_misses = 0
		l1_i_1_write_miss_rate = 0
		l1_i_1_total_get = 0
		l1_i_1_total_getx = 0
		l1_i_1_get_miss_rate = 0
		l1_i_1_getx_miss_rate = 0
		l1_i_1_total_upgrades = 0
		l1_i_1_upgrade_miss_rate = 0
		l1_i_1_total_write_backs = 0
		l1_i_1_cache_utilization = sim_data.getfloat("L1_I_Cache_1", "CacheUtilization")


		l1_d_1_total_cache_ctrl_loops = sim_data.getint("L1_D_Cache_1", "TotalCacheCtrlLoops")
		l1_d_1_total_accesses = sim_data.getint("L1_D_Cache_1", "TotalAccesses")
		l1_d_1_total_hits = sim_data.getint("L1_D_Cache_1", "TotalHits")
		l1_d_1_total_misses = sim_data.getint("L1_D_Cache_1", "TotalMisses")
		l1_d_1_miss_rate = sim_data.getfloat("L1_D_Cache_1", "MissRate")
		l1_d_1_total_reads = sim_data.getint("L1_D_Cache_1", "TotalReads")
		l1_d_1_total_read_misses = sim_data.getint("L1_D_Cache_1", "TotalReadMisses")
		l1_d_1_read_miss_rate = sim_data.getfloat("L1_D_Cache_1", "ReadMissRate")
		l1_d_1_total_writes = sim_data.getint("L1_D_Cache_1", "TotalWrites")
		l1_d_1_total_write_misses = sim_data.getint("L1_D_Cache_1", "TotalWriteMisses")
		l1_d_1_write_miss_rate = sim_data.getfloat("L1_D_Cache_1", "WriteMissRate")
		l1_d_1_total_get = sim_data.getint("L1_D_Cache_1", "TotalGet")
		l1_d_1_total_getx = sim_data.getint("L1_D_Cache_1", "TotalGetx")
		l1_d_1_get_miss_rate = sim_data.getfloat("L1_D_Cache_1", "GetMissRate")
		l1_d_1_getx_miss_rate = sim_data.getfloat("L1_D_Cache_1", "GetxMissRate")
		l1_d_1_total_upgrades = sim_data.getint("L1_D_Cache_1", "TotalUpgrades")
		l1_d_1_upgrade_miss_rate = sim_data.getfloat("L1_D_Cache_1", "UpgradeMissRate")
		l1_d_1_total_write_backs = sim_data.getint("L1_D_Cache_1", "TotalWriteBacks")
		l1_d_1_cache_utilization = sim_data.getfloat("L1_D_Cache_1", "CacheUtilization")


		l2_1_total_cache_ctrl_loops = sim_data.getint("L2_Cache_1", "TotalCacheCtrlLoops")
		l2_1_total_accesses = sim_data.getint("L2_Cache_1", "TotalAccesses")
		l2_1_total_hits = sim_data.getint("L2_Cache_1", "TotalHits")
		l2_1_total_misses = sim_data.getint("L2_Cache_1", "TotalMisses")
		l2_1_miss_rate = sim_data.getfloat("L2_Cache_1", "MissRate")
		l2_1_total_reads = sim_data.getint("L2_Cache_1", "TotalReads")
		l2_1_total_read_misses = sim_data.getint("L2_Cache_1", "TotalReadMisses")
		l2_1_read_miss_rate = sim_data.getfloat("L2_Cache_1", "ReadMissRate")
		l2_1_total_writes = sim_data.getint("L2_Cache_1", "TotalWrites")
		l2_1_total_write_misses = sim_data.getint("L2_Cache_1", "TotalWriteMisses")
		l2_1_write_miss_rate = sim_data.getfloat("L2_Cache_1", "WriteMissRate")
		l2_1_total_get = sim_data.getint("L2_Cache_1", "TotalGet")
		l2_1_total_getx = sim_data.getint("L2_Cache_1", "TotalGetx")
		l2_1_get_miss_rate = sim_data.getfloat("L2_Cache_1", "GetMissRate")
		l2_1_getx_miss_rate = sim_data.getfloat("L2_Cache_1", "GetxMissRate")
		l2_1_total_upgrades = sim_data.getint("L2_Cache_1", "TotalUpgrades")
		l2_1_upgrade_miss_rate = sim_data.getfloat("L2_Cache_1", "UpgradeMissRate")
		l2_1_total_write_backs = sim_data.getint("L2_Cache_1", "TotalWriteBacks")
		l2_1_cache_utilization = sim_data.getfloat("L2_Cache_1", "CacheUtilization")

		l3_1_total_cache_ctrl_loops = sim_data.getint("L3_Cache_1", "TotalCacheCtrlLoops")
		l3_1_total_accesses = sim_data.getint("L3_Cache_1", "TotalAccesses")
		l3_1_total_hits = sim_data.getint("L3_Cache_1", "TotalHits")
		l3_1_total_misses = sim_data.getint("L3_Cache_1", "TotalMisses")
		l3_1_miss_rate = sim_data.getfloat("L3_Cache_1", "MissRate")
		l3_1_total_reads = sim_data.getint("L3_Cache_1", "TotalReads")
		l3_1_total_read_misses = sim_data.getint("L3_Cache_1", "TotalReadMisses")
		l3_1_read_miss_rate = sim_data.getfloat("L3_Cache_1", "ReadMissRate")
		l3_1_total_writes = sim_data.getint("L3_Cache_1", "TotalWrites")
		l3_1_total_write_misses = sim_data.getint("L3_Cache_1", "TotalWriteMisses")
		l3_1_write_miss_rate = sim_data.getfloat("L3_Cache_1", "WriteMissRate")
		l3_1_total_get = sim_data.getint("L3_Cache_1", "TotalGet")
		l3_1_total_getx = sim_data.getint("L3_Cache_1", "TotalGetx")
		l3_1_get_miss_rate = sim_data.getfloat("L3_Cache_1", "GetMissRate")
		l3_1_getx_miss_rate = sim_data.getfloat("L3_Cache_1", "GetxMissRate")
		l3_1_total_upgrades = sim_data.getint("L3_Cache_1", "TotalUpgrades")
		l3_1_upgrade_miss_rate = sim_data.getfloat("L3_Cache_1", "UpgradeMissRate")
		l3_1_total_write_backs = sim_data.getint("L3_Cache_1", "TotalWriteBacks")
		l3_1_cache_utilization = sim_data.getfloat("L3_Cache_1", "CacheUtilization")

		table_P1 = [
		["TotalCacheCtrlLoops", l1_i_1_total_cache_ctrl_loops, l1_d_1_total_cache_ctrl_loops, l2_1_total_cache_ctrl_loops, l3_1_total_cache_ctrl_loops],
		["TotalAccesses", l1_i_1_total_accesses, l1_d_1_total_accesses, l2_1_total_accesses, l3_1_total_accesses], 
		["TotalHits", l1_i_1_total_hits, l1_d_1_total_hits, l2_1_total_hits, l3_1_total_hits],
		["TotalMisses", l1_i_1_total_misses, l1_d_1_total_misses, l2_1_total_misses, l3_1_total_misses],
		["MissRate", l1_i_1_miss_rate, l1_d_1_miss_rate, l2_1_miss_rate, l3_1_miss_rate],
		["TotalReads", l1_i_1_total_reads, l1_d_1_total_reads, l2_1_total_reads, l3_1_total_reads],
		["TotalReadMisses", l1_i_1_total_read_misses, l1_d_1_total_read_misses, l2_1_total_read_misses, l3_1_total_read_misses],
		["ReadMissRate", l1_i_1_read_miss_rate, l1_d_1_read_miss_rate, l2_1_read_miss_rate, l3_1_read_miss_rate],
		["TotalWrites", l1_i_1_total_writes, l1_d_1_total_writes, l2_1_total_writes, l3_1_total_writes],
		["TotalWriteMisses", l1_i_1_total_write_misses, l1_d_1_total_write_misses, l2_1_total_write_misses, l3_1_total_write_misses],
		["WriteMissRate", l1_i_1_write_miss_rate, l1_d_1_write_miss_rate, l2_1_write_miss_rate, l3_1_write_miss_rate],
		["TotalGet", l1_i_1_total_get, l1_d_1_total_get, l2_1_total_get, l3_1_total_get],
		["TotalGetx", l1_i_1_total_getx, l1_d_1_total_getx, l2_1_total_getx, l3_1_total_getx],
		["GetMissRate", l1_i_1_get_miss_rate, l1_d_1_get_miss_rate, l2_1_get_miss_rate, l3_1_get_miss_rate],
		["GetxMissRate", l1_i_1_getx_miss_rate, l1_d_1_getx_miss_rate, l2_1_getx_miss_rate, l3_1_getx_miss_rate],
		["TotalUpgrades", l1_i_1_total_upgrades, l1_d_1_total_upgrades, l2_1_total_upgrades, l3_1_total_upgrades],
		["UpgradeMissRate", l1_i_1_upgrade_miss_rate, l1_d_1_upgrade_miss_rate, l2_1_upgrade_miss_rate, l3_1_upgrade_miss_rate],
		["TotalWriteBacks", l1_i_1_total_write_backs, l1_d_1_total_write_backs, l2_1_total_write_backs, l3_1_total_write_backs],
		["CacheUtilization", l1_i_1_cache_utilization, l1_d_1_cache_utilization, l2_1_cache_utilization, l3_1_cache_utilization]
		]

	###core P2###
	if int(options.NumCores) == 3 or int(options.NumCores) == 4:
		l1_i_2_total_cache_ctrl_loops = sim_data.getint("L1_I_Cache_2", "TotalCacheCtrlLoops")
		l1_i_2_total_accesses = sim_data.getint("L1_I_Cache_2", "TotalAccesses")
		l1_i_2_total_hits = sim_data.getint("L1_I_Cache_2", "TotalHits")
		l1_i_2_total_misses = sim_data.getint("L1_I_Cache_2", "TotalMisses")
		l1_i_2_miss_rate = sim_data.getfloat("L1_I_Cache_2", "MissRate")
		l1_i_2_total_reads = sim_data.getint("L1_I_Cache_2", "TotalReads")
		l1_i_2_total_read_misses = sim_data.getint("L1_I_Cache_2", "TotalReadMisses")
		l1_i_2_read_miss_rate = sim_data.getfloat("L1_I_Cache_2", "ReadMissRate")
		l1_i_2_total_writes = 0
		l1_i_2_total_write_misses = 0
		l1_i_2_write_miss_rate = 0
		l1_i_2_total_get = 0
		l1_i_2_total_getx = 0
		l1_i_2_get_miss_rate = 0
		l1_i_2_getx_miss_rate = 0
		l1_i_2_total_upgrades = 0
		l1_i_2_upgrade_miss_rate = 0
		l1_i_2_total_write_backs = 0
		l1_i_2_cache_utilization = sim_data.getfloat("L1_I_Cache_2", "CacheUtilization")

		l1_d_2_total_cache_ctrl_loops = sim_data.getint("L1_D_Cache_2", "TotalCacheCtrlLoops")
		l1_d_2_total_accesses = sim_data.getint("L1_D_Cache_2", "TotalAccesses")
		l1_d_2_total_hits = sim_data.getint("L1_D_Cache_2", "TotalHits")
		l1_d_2_total_misses = sim_data.getint("L1_D_Cache_2", "TotalMisses")
		l1_d_2_miss_rate = sim_data.getfloat("L1_D_Cache_2", "MissRate")
		l1_d_2_total_reads = sim_data.getint("L1_D_Cache_2", "TotalReads")
		l1_d_2_total_read_misses = sim_data.getint("L1_D_Cache_2", "TotalReadMisses")
		l1_d_2_read_miss_rate = sim_data.getfloat("L1_D_Cache_2", "ReadMissRate")
		l1_d_2_total_writes = sim_data.getint("L1_D_Cache_2", "TotalWrites")
		l1_d_2_total_write_misses = sim_data.getint("L1_D_Cache_2", "TotalWriteMisses")
		l1_d_2_write_miss_rate = sim_data.getfloat("L1_D_Cache_2", "WriteMissRate")
		l1_d_2_total_get = sim_data.getint("L1_D_Cache_2", "TotalGet")
		l1_d_2_total_getx = sim_data.getint("L1_D_Cache_2", "TotalGetx")
		l1_d_2_get_miss_rate = sim_data.getfloat("L1_D_Cache_2", "GetMissRate")
		l1_d_2_getx_miss_rate = sim_data.getfloat("L1_D_Cache_2", "GetxMissRate")
		l1_d_2_total_upgrades = sim_data.getint("L1_D_Cache_2", "TotalUpgrades")
		l1_d_2_upgrade_miss_rate = sim_data.getfloat("L1_D_Cache_2", "UpgradeMissRate")
		l1_d_2_total_write_backs = sim_data.getint("L1_D_Cache_2", "TotalWriteBacks")
		l1_d_2_cache_utilization = sim_data.getfloat("L1_D_Cache_2", "CacheUtilization")

		l2_2_total_cache_ctrl_loops = sim_data.getint("L2_Cache_2", "TotalCacheCtrlLoops")
		l2_2_total_accesses = sim_data.getint("L2_Cache_2", "TotalAccesses")
		l2_2_total_hits = sim_data.getint("L2_Cache_2", "TotalHits")
		l2_2_total_misses = sim_data.getint("L2_Cache_2", "TotalMisses")
		l2_2_miss_rate = sim_data.getfloat("L2_Cache_2", "MissRate")
		l2_2_total_reads = sim_data.getint("L2_Cache_2", "TotalReads")
		l2_2_total_read_misses = sim_data.getint("L2_Cache_2", "TotalReadMisses")
		l2_2_read_miss_rate = sim_data.getfloat("L2_Cache_2", "ReadMissRate")
		l2_2_total_writes = sim_data.getint("L2_Cache_2", "TotalWrites")
		l2_2_total_write_misses = sim_data.getint("L2_Cache_2", "TotalWriteMisses")
		l2_2_write_miss_rate = sim_data.getfloat("L2_Cache_2", "WriteMissRate")
		l2_2_total_get = sim_data.getint("L2_Cache_2", "TotalGet")
		l2_2_total_getx = sim_data.getint("L2_Cache_2", "TotalGetx")
		l2_2_get_miss_rate = sim_data.getfloat("L2_Cache_2", "GetMissRate")
		l2_2_getx_miss_rate = sim_data.getfloat("L2_Cache_2", "GetxMissRate")
		l2_2_total_upgrades = sim_data.getint("L2_Cache_2", "TotalUpgrades")
		l2_2_upgrade_miss_rate = sim_data.getfloat("L2_Cache_2", "UpgradeMissRate")
		l2_2_total_write_backs = sim_data.getint("L2_Cache_2", "TotalWriteBacks")
		l2_2_cache_utilization = sim_data.getfloat("L2_Cache_2", "CacheUtilization")

		l3_2_total_cache_ctrl_loops = sim_data.getint("L3_Cache_2", "TotalCacheCtrlLoops")
		l3_2_total_accesses = sim_data.getint("L3_Cache_2", "TotalAccesses")
		l3_2_total_hits = sim_data.getint("L3_Cache_2", "TotalHits")
		l3_2_total_misses = sim_data.getint("L3_Cache_2", "TotalMisses")
		l3_2_miss_rate = sim_data.getfloat("L3_Cache_2", "MissRate")
		l3_2_total_reads = sim_data.getint("L3_Cache_2", "TotalReads")
		l3_2_total_read_misses = sim_data.getint("L3_Cache_2", "TotalReadMisses")
		l3_2_read_miss_rate = sim_data.getfloat("L3_Cache_2", "ReadMissRate")
		l3_2_total_writes = sim_data.getint("L3_Cache_2", "TotalWrites")
		l3_2_total_write_misses = sim_data.getint("L3_Cache_2", "TotalWriteMisses")
		l3_2_write_miss_rate = sim_data.getfloat("L3_Cache_2", "WriteMissRate")
		l3_2_total_get = sim_data.getint("L3_Cache_2", "TotalGet")
		l3_2_total_getx = sim_data.getint("L3_Cache_2", "TotalGetx")
		l3_2_get_miss_rate = sim_data.getfloat("L3_Cache_2", "GetMissRate")
		l3_2_getx_miss_rate = sim_data.getfloat("L3_Cache_2", "GetxMissRate")
		l3_2_total_upgrades = sim_data.getint("L3_Cache_2", "TotalUpgrades")
		l3_2_upgrade_miss_rate = sim_data.getfloat("L3_Cache_2", "UpgradeMissRate")
		l3_2_total_write_backs = sim_data.getint("L3_Cache_2", "TotalWriteBacks")
		l3_2_cache_utilization = sim_data.getfloat("L3_Cache_2", "CacheUtilization")

		table_P2 = [
		["TotalCacheCtrlLoops", l1_i_2_total_cache_ctrl_loops, l1_d_2_total_cache_ctrl_loops, l2_2_total_cache_ctrl_loops, l3_2_total_cache_ctrl_loops],
		["TotalAccesses", l1_i_2_total_accesses, l1_d_2_total_accesses, l2_2_total_accesses, l3_2_total_accesses], 
		["TotalHits", l1_i_2_total_hits, l1_d_2_total_hits, l2_2_total_hits, l3_2_total_hits],
		["TotalMisses", l1_i_2_total_misses, l1_d_2_total_misses, l2_2_total_misses, l3_2_total_misses],
		["MissRate", l1_i_2_miss_rate, l1_d_2_miss_rate, l2_2_miss_rate, l3_2_miss_rate],
		["TotalReads", l1_i_2_total_reads, l1_d_2_total_reads, l2_2_total_reads, l3_2_total_reads],
		["TotalReadMisses", l1_i_2_total_read_misses, l1_d_2_total_read_misses, l2_2_total_read_misses, l3_2_total_read_misses],
		["ReadMissRate", l1_i_2_read_miss_rate, l1_d_2_read_miss_rate, l2_2_read_miss_rate, l3_2_read_miss_rate],
		["TotalWrites", l1_i_2_total_writes, l1_d_2_total_writes, l2_2_total_writes, l3_2_total_writes],
		["TotalWriteMisses", l1_i_2_total_write_misses, l1_d_2_total_write_misses, l2_2_total_write_misses, l3_2_total_write_misses],
		["WriteMissRate", l1_i_2_write_miss_rate, l1_d_2_write_miss_rate, l2_2_write_miss_rate, l3_2_write_miss_rate],
		["TotalGet", l1_i_2_total_get, l1_d_2_total_get, l2_2_total_get, l3_2_total_get],
		["TotalGetx", l1_i_2_total_getx, l1_d_2_total_getx, l2_2_total_getx, l3_2_total_getx],
		["GetMissRate", l1_i_2_get_miss_rate, l1_d_2_get_miss_rate, l2_2_get_miss_rate, l3_2_get_miss_rate],
		["GetxMissRate", l1_i_2_getx_miss_rate, l1_d_2_getx_miss_rate, l2_2_getx_miss_rate, l3_2_getx_miss_rate],
		["TotalUpgrades", l1_i_2_total_upgrades, l1_d_2_total_upgrades, l2_2_total_upgrades, l3_2_total_upgrades],
		["UpgradeMissRate", l1_i_2_upgrade_miss_rate, l1_d_2_upgrade_miss_rate, l2_2_upgrade_miss_rate, l3_2_upgrade_miss_rate],
		["TotalWriteBacks", l1_i_2_total_write_backs, l1_d_2_total_write_backs, l2_2_total_write_backs, l3_2_total_write_backs],
		["CacheUtilization", l1_i_2_cache_utilization, l1_d_2_cache_utilization, l2_2_cache_utilization, l3_2_cache_utilization]
		]

	###core P3###
	if int(options.NumCores) == 4:
		l1_i_3_total_cache_ctrl_loops = sim_data.getint("L1_I_Cache_3", "TotalCacheCtrlLoops")
		l1_i_3_total_accesses = sim_data.getint("L1_I_Cache_3", "TotalAccesses")
		l1_i_3_total_hits = sim_data.getint("L1_I_Cache_3", "TotalHits")
		l1_i_3_total_misses = sim_data.getint("L1_I_Cache_3", "TotalMisses")
		l1_i_3_miss_rate = sim_data.getfloat("L1_I_Cache_3", "MissRate")
		l1_i_3_total_reads = sim_data.getint("L1_I_Cache_3", "TotalReads")
		l1_i_3_total_read_misses = sim_data.getint("L1_I_Cache_3", "TotalReadMisses")
		l1_i_3_read_miss_rate = sim_data.getfloat("L1_I_Cache_3", "ReadMissRate")
		l1_i_3_total_writes = 0
		l1_i_3_total_write_misses = 0
		l1_i_3_write_miss_rate = 0
		l1_i_3_total_get = 0
		l1_i_3_total_getx = 0
		l1_i_3_get_miss_rate = 0
		l1_i_3_getx_miss_rate = 0
		l1_i_3_total_upgrades = 0
		l1_i_3_upgrade_miss_rate = 0
		l1_i_3_total_write_backs = 0
		l1_i_3_cache_utilization = sim_data.getfloat("L1_I_Cache_3", "CacheUtilization")

		l1_d_3_total_cache_ctrl_loops = sim_data.getint("L1_D_Cache_3", "TotalCacheCtrlLoops")
		l1_d_3_total_accesses = sim_data.getint("L1_D_Cache_3", "TotalAccesses")
		l1_d_3_total_hits = sim_data.getint("L1_D_Cache_3", "TotalHits")
		l1_d_3_total_misses = sim_data.getint("L1_D_Cache_3", "TotalMisses")
		l1_d_3_miss_rate = sim_data.getfloat("L1_D_Cache_3", "MissRate")
		l1_d_3_total_reads = sim_data.getint("L1_D_Cache_3", "TotalReads")
		l1_d_3_total_read_misses = sim_data.getint("L1_D_Cache_3", "TotalReadMisses")
		l1_d_3_read_miss_rate = sim_data.getfloat("L1_D_Cache_3", "ReadMissRate")
		l1_d_3_total_writes = sim_data.getint("L1_D_Cache_3", "TotalWrites")
		l1_d_3_total_write_misses = sim_data.getint("L1_D_Cache_3", "TotalWriteMisses")
		l1_d_3_write_miss_rate = sim_data.getfloat("L1_D_Cache_3", "WriteMissRate")
		l1_d_3_total_get = sim_data.getint("L1_D_Cache_3", "TotalGet")
		l1_d_3_total_getx = sim_data.getint("L1_D_Cache_3", "TotalGetx")
		l1_d_3_get_miss_rate = sim_data.getfloat("L1_D_Cache_3", "GetMissRate")
		l1_d_3_getx_miss_rate = sim_data.getfloat("L1_D_Cache_3", "GetxMissRate")
		l1_d_3_total_upgrades = sim_data.getint("L1_D_Cache_3", "TotalUpgrades")
		l1_d_3_upgrade_miss_rate = sim_data.getfloat("L1_D_Cache_3", "UpgradeMissRate")
		l1_d_3_total_write_backs = sim_data.getint("L1_D_Cache_3", "TotalWriteBacks")
		l1_d_3_cache_utilization = sim_data.getfloat("L1_D_Cache_3", "CacheUtilization")


		l2_3_total_cache_ctrl_loops = sim_data.getint("L2_Cache_3", "TotalCacheCtrlLoops")
		l2_3_total_accesses = sim_data.getint("L2_Cache_3", "TotalAccesses")
		l2_3_total_hits = sim_data.getint("L2_Cache_3", "TotalHits")
		l2_3_total_misses = sim_data.getint("L2_Cache_3", "TotalMisses")
		l2_3_miss_rate = sim_data.getfloat("L2_Cache_3", "MissRate")
		l2_3_total_reads = sim_data.getint("L2_Cache_3", "TotalReads")
		l2_3_total_read_misses = sim_data.getint("L2_Cache_3", "TotalReadMisses")
		l2_3_read_miss_rate = sim_data.getfloat("L2_Cache_3", "ReadMissRate")
		l2_3_total_writes = sim_data.getint("L2_Cache_3", "TotalWrites")
		l2_3_total_write_misses = sim_data.getint("L2_Cache_3", "TotalWriteMisses")
		l2_3_write_miss_rate = sim_data.getfloat("L2_Cache_3", "WriteMissRate")
		l2_3_total_get = sim_data.getint("L2_Cache_3", "TotalGet")
		l2_3_total_getx = sim_data.getint("L2_Cache_3", "TotalGetx")
		l2_3_get_miss_rate = sim_data.getfloat("L2_Cache_3", "GetMissRate")
		l2_3_getx_miss_rate = sim_data.getfloat("L2_Cache_3", "GetxMissRate")
		l2_3_total_upgrades = sim_data.getint("L2_Cache_3", "TotalUpgrades")
		l2_3_upgrade_miss_rate = sim_data.getfloat("L2_Cache_3", "UpgradeMissRate")
		l2_3_total_write_backs = sim_data.getint("L2_Cache_3", "TotalWriteBacks")
		l2_3_cache_utilization = sim_data.getfloat("L2_Cache_3", "CacheUtilization")


		l3_3_total_cache_ctrl_loops = sim_data.getint("L3_Cache_3", "TotalCacheCtrlLoops")
		l3_3_total_accesses = sim_data.getint("L3_Cache_3", "TotalAccesses")
		l3_3_total_hits = sim_data.getint("L3_Cache_3", "TotalHits")
		l3_3_total_misses = sim_data.getint("L3_Cache_3", "TotalMisses")
		l3_3_miss_rate = sim_data.getfloat("L3_Cache_3", "MissRate")
		l3_3_total_reads = sim_data.getint("L3_Cache_3", "TotalReads")
		l3_3_total_read_misses = sim_data.getint("L3_Cache_3", "TotalReadMisses")
		l3_3_read_miss_rate = sim_data.getfloat("L3_Cache_3", "ReadMissRate")
		l3_3_total_writes = sim_data.getint("L3_Cache_3", "TotalWrites")
		l3_3_total_write_misses = sim_data.getint("L3_Cache_3", "TotalWriteMisses")
		l3_3_write_miss_rate = sim_data.getfloat("L3_Cache_3", "WriteMissRate")
		l3_3_total_get = sim_data.getint("L3_Cache_3", "TotalGet")
		l3_3_total_getx = sim_data.getint("L3_Cache_3", "TotalGetx")
		l3_3_get_miss_rate = sim_data.getfloat("L3_Cache_3", "GetMissRate")
		l3_3_getx_miss_rate = sim_data.getfloat("L3_Cache_3", "GetxMissRate")
		l3_3_total_upgrades = sim_data.getint("L3_Cache_3", "TotalUpgrades")
		l3_3_upgrade_miss_rate = sim_data.getfloat("L3_Cache_3", "UpgradeMissRate")
		l3_3_total_write_backs = sim_data.getint("L3_Cache_3", "TotalWriteBacks")
		l3_3_cache_utilization = sim_data.getfloat("L3_Cache_3", "CacheUtilization")

		table_P3 = [
		["TotalCacheCtrlLoops", l1_i_3_total_cache_ctrl_loops, l1_d_3_total_cache_ctrl_loops, l2_3_total_cache_ctrl_loops, l3_3_total_cache_ctrl_loops],
		["TotalAccesses", l1_i_3_total_accesses, l1_d_3_total_accesses, l2_3_total_accesses, l3_3_total_accesses], 
		["TotalHits", l1_i_3_total_hits, l1_d_3_total_hits, l2_3_total_hits, l3_3_total_hits],
		["TotalMisses", l1_i_3_total_misses, l1_d_3_total_misses, l2_3_total_misses, l3_3_total_misses],
		["MissRate", l1_i_3_miss_rate, l1_d_3_miss_rate, l2_3_miss_rate, l3_3_miss_rate],
		["TotalReads", l1_i_3_total_reads, l1_d_3_total_reads, l2_3_total_reads, l3_3_total_reads],
		["TotalReadMisses", l1_i_3_total_read_misses, l1_d_3_total_read_misses, l2_3_total_read_misses, l3_3_total_read_misses],
		["ReadMissRate", l1_i_3_read_miss_rate, l1_d_3_read_miss_rate, l2_3_read_miss_rate, l3_3_read_miss_rate],
		["TotalWrites", l1_i_3_total_writes, l1_d_3_total_writes, l2_3_total_writes, l3_3_total_writes],
		["TotalWriteMisses", l1_i_3_total_write_misses, l1_d_3_total_write_misses, l2_3_total_write_misses, l3_3_total_write_misses],
		["WriteMissRate", l1_i_3_write_miss_rate, l1_d_3_write_miss_rate, l2_3_write_miss_rate, l3_3_write_miss_rate],
		["TotalGet", l1_i_3_total_get, l1_d_3_total_get, l2_3_total_get, l3_3_total_get],
		["TotalGetx", l1_i_3_total_getx, l1_d_3_total_getx, l2_3_total_getx, l3_3_total_getx],
		["GetMissRate", l1_i_3_get_miss_rate, l1_d_3_get_miss_rate, l2_3_get_miss_rate, l3_3_get_miss_rate],
		["GetxMissRate", l1_i_3_getx_miss_rate, l1_d_3_getx_miss_rate, l2_3_getx_miss_rate, l3_3_getx_miss_rate],
		["TotalUpgrades", l1_i_3_total_upgrades, l1_d_3_total_upgrades, l2_3_total_upgrades, l3_3_total_upgrades],
		["UpgradeMissRate", l1_i_3_upgrade_miss_rate, l1_d_3_upgrade_miss_rate, l2_3_upgrade_miss_rate, l3_3_upgrade_miss_rate],
		["TotalWriteBacks", l1_i_3_total_write_backs, l1_d_3_total_write_backs, l2_3_total_write_backs, l3_3_total_write_backs],
		["CacheUtilization", l1_i_3_cache_utilization, l1_d_3_cache_utilization, l2_3_cache_utilization, l3_3_cache_utilization]
		]

table_L3_cache = [
		["TotalCacheCtrlLoops", l3_0_total_cache_ctrl_loops, l3_1_total_cache_ctrl_loops, l3_2_total_cache_ctrl_loops, l3_3_total_cache_ctrl_loops, l3_total_cache_ctrl_loops],
		["TotalAccesses", l3_0_total_accesses, l3_1_total_accesses, l3_2_total_accesses, l3_3_total_accesses, l3_total_accesses], 
		["TotalHits", l3_0_total_hits, l3_1_total_hits, l3_2_total_hits, l3_3_total_hits, l3_total_hits],
		["TotalMisses", l3_0_total_misses, l3_1_total_misses, l3_2_total_misses, l3_3_total_misses, l3_total_misses],
		["MissRate", l3_0_miss_rate, l3_1_miss_rate, l3_2_miss_rate, l3_3_miss_rate, l3_miss_rate],
		["TotalReads", l3_0_total_reads, l3_1_total_reads, l3_2_total_reads, l3_3_total_reads, l3_total_reads],
		["TotalReadMisses", l3_0_total_read_misses, l3_1_total_read_misses, l3_2_total_read_misses, l3_3_total_read_misses, l3_total_read_misses],
		["ReadMissRate", l3_0_read_miss_rate, l3_1_read_miss_rate, l3_2_read_miss_rate, l3_3_read_miss_rate, l3_read_miss_rate],
		["TotalWrites", l3_0_total_writes, l3_1_total_writes, l3_2_total_writes, l3_3_total_writes, l3_total_writes],
		["TotalWriteMisses", l3_0_total_write_misses, l3_1_total_write_misses, l3_2_total_write_misses, l3_3_total_write_misses, l3_total_write_misses],
		["WriteMissRate", l3_0_write_miss_rate, l3_1_write_miss_rate, l3_2_write_miss_rate, l3_3_write_miss_rate, l3_write_miss_rate],
		["TotalGet",  l3_0_total_get,  l3_1_total_get,  l3_2_total_get, l3_3_total_get, l3_total_get],
		["TotalGetx", l3_0_total_getx, l3_1_total_getx, l3_2_total_getx, l3_3_total_getx, l3_total_getx],
		["GetMissRate", l3_0_get_miss_rate, l3_1_get_miss_rate, l3_2_get_miss_rate, l3_3_get_miss_rate, l3_get_miss_rate],
		["GetxMissRate", l3_0_getx_miss_rate, l3_1_getx_miss_rate, l3_2_getx_miss_rate, l3_3_getx_miss_rate, l3_getx_miss_rate],
		["TotalUpgrades", l3_0_total_upgrades, l3_1_total_upgrades, l3_2_total_upgrades, l3_3_total_upgrades, l3_total_upgrades],
		["UpgradeMissRate", l3_0_upgrade_miss_rate, l3_1_upgrade_miss_rate, l3_2_upgrade_miss_rate, l3_3_upgrade_miss_rate, l3_total_write_backs],
		["TotalWriteBacks", l3_0_total_write_backs, l3_1_total_write_backs, l3_2_total_write_backs, l3_3_total_write_backs, l3_total_write_backs],
		["CacheUtilization", l3_0_cache_utilization, l3_1_cache_utilization, l3_2_cache_utilization, l3_3_cache_utilization, l3_cache_utilization]
		]

#First_access_lat = sa_data.getint('FullRunStats', 'MemSystem_FirstAccessLat(Fetch)')
	#Total_fetches = sa_data.getint('FullRunStats', 'MemSystem_TotalFetches')
	#Fetches_L1 = sa_data.getint('FullRunStats', 'MemSystem_FetchesL1')
	#Fetches_L2 = sa_data.getint('FullRunStats', 'MemSystem_FetchesL2')
	#Fetches_L3 = sa_data.getint('FullRunStats', 'MemSystem_FetchesL3')
	#Fetches_memory = sa_data.getint('FullRunStats', 'MemSystem_FetchesMemory')
	#Total_loads = sa_data.getint('FullRunStats', 'MemSystem_TotalLoads')
	#Loads_L1 = sa_data.getint('FullRunStats', 'MemSystem_LoadsL1')
	#Loads_L2 = sa_data.getint('FullRunStats', 'MemSystem_LoadsL2')
	#Loads_L3 = sa_data.getint('FullRunStats', 'MemSystem_LoadsL3')
	#Loads_memory = sa_data.getint('FullRunStats', 'MemSystem_LoadsMemory')
	#Loads_getfwd = sa_data.getint('FullRunStats', 'MemSystem_LoadsGetFwd')
	#Total_store = sa_data.getint('FullRunStats', 'MemSystem_TotalStore')
	#Stores_L1 = sa_data.getint('FullRunStats', 'MemSystem_StoresL1')
	#Stores_L2 = sa_data.getint('FullRunStats', 'MemSystem_StoresL2')
	#Stores_L3 = sa_data.getint('FullRunStats', 'MemSystem_StoresL3')
	#Stores_memory = sa_data.getint('FullRunStats', 'MemSystem_StoresMemory')
	#Stores_getxfwd = sa_data.getint('FullRunStats', 'MemSystem_StoresGetxFwd')
	#Stores_upgrade = sa_data.getint('FullRunStats', 'MemSystem_StoresUpgrade')

###core P0###
	if int(options.NumCores) == 1 or int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		core_0_NumSyscalls = cpu_data.getint('FullRunStats', 'core_0_NumSyscalls')
		core_0_ROBStalls = cpu_data.getint('FullRunStats', 'core_0_ROBStalls')
		core_0_ROBStallLoad = cpu_data.getint('FullRunStats', 'core_0_ROBStallLoad')
		core_0_ROBStallStore = cpu_data.getint('FullRunStats', 'core_0_ROBStallStore')
		core_0_ROBStallOther = cpu_data.getint('FullRunStats', 'core_0_ROBStallOther')
		core_0_FirstFetchCycle = cpu_data.getint('FullRunStats', 'core_0_FirstFetchCycle')
		core_0_LastCommitCycle = cpu_data.getint('FullRunStats', 'core_0_FirstFetchCycle')
		core_0_FetchStall = cpu_data.getint('FullRunStats', 'core_0_FetchStall')
		core_0_RunTime = cpu_data.getint('FullRunStats', 'core_0_RunTime')
		core_0_IdleTime = cpu_data.getint('FullRunStats', 'core_0_IdleTime')
		core_0_SystemTime = cpu_data.getint('FullRunStats', 'core_0_SystemTime')
		core_0_StallTime = cpu_data.getint('FullRunStats', 'core_0_StallTime')
		core_0_BusyTime = cpu_data.getint('FullRunStats', 'core_0_BusyTime')
		core_0_IdlePct = cpu_data.getfloat('FullRunStats', 'core_0_IdlePct')
		core_0_RunPct = cpu_data.getfloat('FullRunStats', 'core_0_RunPct')
		core_0_SystemPct = cpu_data.getfloat('FullRunStats', 'core_0_SystemPct')
		core_0_StallPct = cpu_data.getfloat('FullRunStats', 'core_0_StallPct')
		core_0_BusyPct = cpu_data.getfloat('FullRunStats', 'core_0_BusyPct')
		core_0_StallfetchPct = cpu_data.getfloat('FullRunStats', 'core_0_StallfetchPct')
		core_0_StallLoadPct = cpu_data.getfloat('FullRunStats', 'core_0_StallLoadPct')
		core_0_StallStorePct = cpu_data.getfloat('FullRunStats', 'core_0_StallStorePct')
		core_0_StallOtherPct = cpu_data.getfloat('FullRunStats', 'core_0_StallOtherPct')

	###core P1###
	if int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		core_1_NumSyscalls = cpu_data.getint('FullRunStats', 'core_1_NumSyscalls')
		core_1_ROBStalls = cpu_data.getint('FullRunStats', 'core_1_ROBStalls')
		core_1_ROBStallLoad = cpu_data.getint('FullRunStats', 'core_1_ROBStallLoad')
		core_1_ROBStallStore = cpu_data.getint('FullRunStats', 'core_1_ROBStallStore')
		core_1_ROBStallOther = cpu_data.getint('FullRunStats', 'core_1_ROBStallOther')
		core_1_FirstFetchCycle = cpu_data.getint('FullRunStats', 'core_1_FirstFetchCycle')
		core_1_LastCommitCycle = cpu_data.getint('FullRunStats', 'core_1_FirstFetchCycle')
		core_1_FetchStall = cpu_data.getint('FullRunStats', 'core_1_FetchStall')
		core_1_RunTime = cpu_data.getint('FullRunStats', 'core_1_RunTime')
		core_1_IdleTime = cpu_data.getint('FullRunStats', 'core_1_IdleTime')
		core_1_SystemTime = cpu_data.getint('FullRunStats', 'core_1_SystemTime')
		core_1_StallTime = cpu_data.getint('FullRunStats', 'core_1_StallTime')
		core_1_BusyTime = cpu_data.getint('FullRunStats', 'core_1_BusyTime')
		core_1_IdlePct = cpu_data.getfloat('FullRunStats', 'core_1_IdlePct')
		core_1_RunPct = cpu_data.getfloat('FullRunStats', 'core_1_RunPct')
		core_1_SystemPct = cpu_data.getfloat('FullRunStats', 'core_1_SystemPct')
		core_1_StallPct = cpu_data.getfloat('FullRunStats', 'core_1_StallPct')
		core_1_BusyPct = cpu_data.getfloat('FullRunStats', 'core_1_BusyPct')
		core_1_StallfetchPct = cpu_data.getfloat('FullRunStats', 'core_1_StallfetchPct')
		core_1_StallLoadPct = cpu_data.getfloat('FullRunStats', 'core_1_StallLoadPct')
		core_1_StallStorePct = cpu_data.getfloat('FullRunStats', 'core_1_StallStorePct')
		core_1_StallOtherPct = cpu_data.getfloat('FullRunStats', 'core_1_StallOtherPct')

	###core P2###
	if int(options.NumCores) == 3 or int(options.NumCores) == 4:
		core_2_NumSyscalls = cpu_data.getint('FullRunStats', 'core_2_NumSyscalls')
		core_2_ROBStalls = cpu_data.getint('FullRunStats', 'core_2_ROBStalls')
		core_2_ROBStallLoad = cpu_data.getint('FullRunStats', 'core_2_ROBStallLoad')
		core_2_ROBStallStore = cpu_data.getint('FullRunStats', 'core_2_ROBStallStore')
		core_2_ROBStallOther = cpu_data.getint('FullRunStats', 'core_2_ROBStallOther')
		core_2_FirstFetchCycle = cpu_data.getint('FullRunStats', 'core_2_FirstFetchCycle')
		core_2_LastCommitCycle = cpu_data.getint('FullRunStats', 'core_2_FirstFetchCycle')
		core_2_FetchStall = cpu_data.getint('FullRunStats', 'core_2_FetchStall')
		core_2_RunTime = cpu_data.getint('FullRunStats', 'core_2_RunTime')
		core_2_IdleTime = cpu_data.getint('FullRunStats', 'core_2_IdleTime')
		core_2_SystemTime = cpu_data.getint('FullRunStats', 'core_2_SystemTime')
		core_2_StallTime = cpu_data.getint('FullRunStats', 'core_2_StallTime')
		core_2_BusyTime = cpu_data.getint('FullRunStats', 'core_2_BusyTime')
		core_2_IdlePct = cpu_data.getfloat('FullRunStats', 'core_2_IdlePct')
		core_2_RunPct = cpu_data.getfloat('FullRunStats', 'core_2_RunPct')
		core_2_SystemPct = cpu_data.getfloat('FullRunStats', 'core_2_SystemPct')
		core_2_StallPct = cpu_data.getfloat('FullRunStats', 'core_2_StallPct')
		core_2_BusyPct = cpu_data.getfloat('FullRunStats', 'core_2_BusyPct')
		core_2_StallfetchPct = cpu_data.getfloat('FullRunStats', 'core_2_StallfetchPct')
		core_2_StallLoadPct = cpu_data.getfloat('FullRunStats', 'core_2_StallLoadPct')
		core_2_StallStorePct = cpu_data.getfloat('FullRunStats', 'core_2_StallStorePct')
		core_2_StallOtherPct = cpu_data.getfloat('FullRunStats', 'core_2_StallOtherPct')

	###core P3###
	if int(options.NumCores) == 4:
		core_3_NumSyscalls = cpu_data.getint('FullRunStats', 'core_3_NumSyscalls')
		core_3_ROBStalls = cpu_data.getint('FullRunStats', 'core_3_ROBStalls')
		core_3_ROBStallLoad = cpu_data.getint('FullRunStats', 'core_3_ROBStallLoad')
		core_3_ROBStallStore = cpu_data.getint('FullRunStats', 'core_3_ROBStallStore')
		core_3_ROBStallOther = cpu_data.getint('FullRunStats', 'core_3_ROBStallOther')
		core_3_FirstFetchCycle = cpu_data.getint('FullRunStats', 'core_3_FirstFetchCycle')
		core_3_LastCommitCycle = cpu_data.getint('FullRunStats', 'core_3_FirstFetchCycle')
		core_3_FetchStall = cpu_data.getint('FullRunStats', 'core_3_FetchStall')
		core_3_RunTime = cpu_data.getint('FullRunStats', 'core_3_RunTime')
		core_3_IdleTime = cpu_data.getint('FullRunStats', 'core_3_IdleTime')
		core_3_SystemTime = cpu_data.getint('FullRunStats', 'core_3_SystemTime')
		core_3_StallTime = cpu_data.getint('FullRunStats', 'core_3_StallTime')
		core_3_BusyTime = cpu_data.getint('FullRunStats', 'core_3_BusyTime')
		core_3_IdlePct = cpu_data.getfloat('FullRunStats', 'core_3_IdlePct')
		core_3_RunPct = cpu_data.getfloat('FullRunStats', 'core_3_RunPct')
		core_3_SystemPct = cpu_data.getfloat('FullRunStats', 'core_3_SystemPct')
		core_3_StallPct = cpu_data.getfloat('FullRunStats', 'core_3_StallPct')
		core_3_BusyPct = cpu_data.getfloat('FullRunStats', 'core_3_BusyPct')
		core_3_StallfetchPct = cpu_data.getfloat('FullRunStats', 'core_3_StallfetchPct')
		core_3_StallLoadPct = cpu_data.getfloat('FullRunStats', 'core_3_StallLoadPct')
		core_3_StallStorePct = cpu_data.getfloat('FullRunStats', 'core_3_StallStorePct')
		core_3_StallOtherPct = cpu_data.getfloat('FullRunStats', 'core_3_StallOtherPct')


	table_P0 = [
		["NumSyscalls", core_0_NumSyscalls],
		["ROBStalls", core_0_ROBStalls], 
		["ROBStallLoad", core_0_ROBStallLoad],
		["ROBStallStore", core_0_ROBStallStore],
		["ROBStallOther", core_0_ROBStallOther],
		["FirstFetchCycle", core_0_FirstFetchCycle],
		["LastCommitCycle", core_0_LastCommitCycle],
		["FetchStall", core_0_FetchStall],
		["RunTime", core_0_RunTime],
		["IdleTime", core_0_IdleTime],
		["SystemTime(SysCalls)", core_0_SystemTime],
		["StallTime", core_0_StallTime],
		["BusyTime", core_0_BusyTime],
		["IdlePct", core_0_IdlePct],
		["RunPct", core_0_RunPct],
		["SystemPct", core_0_SystemPct],
		["StallPct", core_0_StallPct],
		["BusyPct", core_0_BusyPct],
		["StallfetchPct", core_0_StallfetchPct],
		["StallLoadPct", core_0_StallLoadPct],
		["StallStorePct", core_0_StallStorePct],
		["StallOtherPct", core_0_StallOtherPct]
		]
