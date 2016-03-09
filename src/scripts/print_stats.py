#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser

cache_combined = 1

def print_cache_stats(options):
	sim_data = ConfigParser.RawConfigParser()
	sim_data.read(options.InFileName)
	
	###core P0###
	if int(options.NumCores) == 1 or int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		l1_i_0_total_cache_ctrl_loops = sim_data.getint("L1_I_Cache_0", "TotalCacheCtrlLoops")
		l1_i_0_total_accesses = sim_data.getint("L1_I_Cache_0", "TotalAccesses")
		l1_i_0_total_hits = sim_data.getint("L1_I_Cache_0", "TotalHits")
		l1_i_0_total_misses = sim_data.getint("L1_I_Cache_0", "TotalMisses")
		l1_i_0_miss_rate = sim_data.getfloat("L1_I_Cache_0", "MissRate")
		l1_i_0_total_reads = sim_data.getint("L1_I_Cache_0", "TotalReads")
		l1_i_0_total_read_misses = sim_data.getint("L1_I_Cache_0", "TotalReadMisses")
		l1_i_0_read_miss_rate = sim_data.getfloat("L1_I_Cache_0", "ReadMissRate")
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
		l1_i_0_cache_utilization = sim_data.getfloat("L1_I_Cache_0", "CacheUtilization")

		l1_d_0_total_cache_ctrl_loops = sim_data.getint("L1_D_Cache_0", "TotalCacheCtrlLoops")
		l1_d_0_total_accesses = sim_data.getint("L1_D_Cache_0", "TotalAccesses")
		l1_d_0_total_hits = sim_data.getint("L1_D_Cache_0", "TotalHits")
		l1_d_0_total_misses = sim_data.getint("L1_D_Cache_0", "TotalMisses")
		l1_d_0_miss_rate = sim_data.getfloat("L1_D_Cache_0", "MissRate")
		l1_d_0_total_reads = sim_data.getint("L1_D_Cache_0", "TotalReads")
		l1_d_0_total_read_misses = sim_data.getint("L1_D_Cache_0", "TotalReadMisses")
		l1_d_0_read_miss_rate = sim_data.getfloat("L1_D_Cache_0", "ReadMissRate")
		l1_d_0_total_writes = sim_data.getint("L1_D_Cache_0", "TotalWrites")
		l1_d_0_total_write_misses = sim_data.getint("L1_D_Cache_0", "TotalWriteMisses")
		l1_d_0_write_miss_rate = sim_data.getfloat("L1_D_Cache_0", "WriteMissRate")
		l1_d_0_total_get = sim_data.getint("L1_D_Cache_0", "TotalGet")
		l1_d_0_total_getx = sim_data.getint("L1_D_Cache_0", "TotalGetx")
		l1_d_0_get_miss_rate = sim_data.getfloat("L1_D_Cache_0", "GetMissRate")
		l1_d_0_getx_miss_rate = sim_data.getfloat("L1_D_Cache_0", "GetxMissRate")
		l1_d_0_total_upgrades = sim_data.getint("L1_D_Cache_0", "TotalUpgrades")
		l1_d_0_upgrade_miss_rate = sim_data.getfloat("L1_D_Cache_0", "UpgradeMissRate")
		l1_d_0_total_write_backs = sim_data.getint("L1_D_Cache_0", "TotalWriteBacks")
		l1_d_0_cache_utilization = sim_data.getfloat("L1_D_Cache_0", "CacheUtilization")

		l2_0_total_cache_ctrl_loops = sim_data.getint("L2_Cache_0", "TotalCacheCtrlLoops")
		l2_0_total_accesses = sim_data.getint("L2_Cache_0", "TotalAccesses")
		l2_0_total_hits = sim_data.getint("L2_Cache_0", "TotalHits")
		l2_0_total_misses = sim_data.getint("L2_Cache_0", "TotalMisses")
		l2_0_miss_rate = sim_data.getfloat("L2_Cache_0", "MissRate")
		l2_0_total_reads = sim_data.getint("L2_Cache_0", "TotalReads")
		l2_0_total_read_misses = sim_data.getint("L2_Cache_0", "TotalReadMisses")
		l2_0_read_miss_rate = sim_data.getfloat("L2_Cache_0", "ReadMissRate")
		l2_0_total_writes = sim_data.getint("L2_Cache_0", "TotalWrites")
		l2_0_total_write_misses = sim_data.getint("L2_Cache_0", "TotalWriteMisses")
		l2_0_write_miss_rate = sim_data.getfloat("L2_Cache_0", "WriteMissRate")
		l2_0_total_get = sim_data.getint("L2_Cache_0", "TotalGet")
		l2_0_total_getx = sim_data.getint("L2_Cache_0", "TotalGetx")
		l2_0_get_miss_rate = sim_data.getfloat("L2_Cache_0", "GetMissRate")
		l2_0_getx_miss_rate = sim_data.getfloat("L2_Cache_0", "GetxMissRate")
		l2_0_total_upgrades = sim_data.getint("L2_Cache_0", "TotalUpgrades")
		l2_0_upgrade_miss_rate = sim_data.getfloat("L2_Cache_0", "UpgradeMissRate")
		l2_0_total_write_backs = sim_data.getint("L2_Cache_0", "TotalWriteBacks")
		l2_0_cache_utilization = sim_data.getfloat("L2_Cache_0", "CacheUtilization")

		l3_0_total_cache_ctrl_loops = sim_data.getint("L3_Cache_0", "TotalCacheCtrlLoops")
		l3_0_total_accesses = sim_data.getint("L3_Cache_0", "TotalAccesses")
		l3_0_total_hits = sim_data.getint("L3_Cache_0", "TotalHits")
		l3_0_total_misses = sim_data.getint("L3_Cache_0", "TotalMisses")
		l3_0_miss_rate = sim_data.getfloat("L3_Cache_0", "MissRate")
		l3_0_total_reads = sim_data.getint("L3_Cache_0", "TotalReads")
		l3_0_total_read_misses = sim_data.getint("L3_Cache_0", "TotalReadMisses")
		l3_0_read_miss_rate = sim_data.getfloat("L3_Cache_0", "ReadMissRate")
		l3_0_total_writes = sim_data.getint("L3_Cache_0", "TotalWrites")
		l3_0_total_write_misses = sim_data.getint("L3_Cache_0", "TotalWriteMisses")
		l3_0_write_miss_rate = sim_data.getfloat("L3_Cache_0", "WriteMissRate")
		l3_0_total_get = sim_data.getint("L3_Cache_0", "TotalGet")
		l3_0_total_getx = sim_data.getint("L3_Cache_0", "TotalGetx")
		l3_0_get_miss_rate = sim_data.getfloat("L3_Cache_0", "GetMissRate")
		l3_0_getx_miss_rate = sim_data.getfloat("L3_Cache_0", "GetxMissRate")
		l3_0_total_upgrades = sim_data.getint("L3_Cache_0", "TotalUpgrades")
		l3_0_upgrade_miss_rate = sim_data.getfloat("L3_Cache_0", "UpgradeMissRate")
		l3_0_total_write_backs = sim_data.getint("L3_Cache_0", "TotalWriteBacks")
		l3_0_cache_utilization = sim_data.getfloat("L3_Cache_0", "CacheUtilization")

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


	###combined caches###
	if int(options.NumCores) == 4:
		l1_i_total_cache_ctrl_loops = l1_i_0_total_cache_ctrl_loops + l1_i_1_total_cache_ctrl_loops + l1_i_2_total_cache_ctrl_loops + l1_i_3_total_cache_ctrl_loops
		l1_i_total_accesses = l1_i_0_total_accesses + l1_i_1_total_accesses + l1_i_2_total_accesses + l1_i_3_total_accesses
		l1_i_total_hits = l1_i_0_total_hits + l1_i_1_total_hits + l1_i_2_total_hits + l1_i_3_total_hits
		l1_i_total_misses = l1_i_0_total_misses + l1_i_1_total_misses + l1_i_2_total_misses + l1_i_3_total_misses
		l1_i_miss_rate = l1_i_0_miss_rate + l1_i_1_miss_rate + l1_i_2_miss_rate + l1_i_3_miss_rate
		l1_i_total_reads = l1_i_0_total_reads + l1_i_1_total_reads + l1_i_2_total_reads + l1_i_3_total_reads
		l1_i_total_read_misses = l1_i_0_total_read_misses + l1_i_1_total_read_misses + l1_i_2_total_read_misses + l1_i_3_total_read_misses
		l1_i_read_miss_rate = l1_i_0_read_miss_rate + l1_i_1_read_miss_rate + l1_i_2_read_miss_rate + l1_i_3_read_miss_rate
		l1_i_total_writes = l1_i_0_total_writes + l1_i_1_total_writes + l1_i_2_total_writes + l1_i_3_total_writes
		l1_i_total_write_misses = l1_i_0_total_write_misses + l1_i_1_total_write_misses + l1_i_2_total_write_misses + l1_i_3_total_write_misses
		l1_i_write_miss_rate = l1_i_0_write_miss_rate + l1_i_1_write_miss_rate + l1_i_2_write_miss_rate + l1_i_3_write_miss_rate
		l1_i_total_get = l1_i_0_total_get +  l1_i_1_total_get +  l1_i_2_total_get + l1_i_3_total_get
		l1_i_total_getx = l1_i_0_total_getx + l1_i_1_total_getx + l1_i_2_total_getx + l1_i_3_total_getx
		l1_i_get_miss_rate = l1_i_0_get_miss_rate + l1_i_1_get_miss_rate + l1_i_2_get_miss_rate + l1_i_3_get_miss_rate
		l1_i_getx_miss_rate = l1_i_0_getx_miss_rate + l1_i_1_getx_miss_rate + l1_i_2_getx_miss_rate + l1_i_3_getx_miss_rate
		l1_i_total_upgrades = l1_i_0_total_upgrades + l1_i_1_total_upgrades + l1_i_2_total_upgrades + l1_i_3_total_upgrades
		l1_i_upgrade_miss_rate = l1_i_0_upgrade_miss_rate + l1_i_1_upgrade_miss_rate + l1_i_2_upgrade_miss_rate + l1_i_3_upgrade_miss_rate
		l1_i_total_write_backs = l1_i_0_total_write_backs + l1_i_1_total_write_backs + l1_i_2_total_write_backs + l1_i_3_total_write_backs
		l1_i_cache_utilization = l1_i_0_cache_utilization + l1_i_1_cache_utilization + l1_i_2_cache_utilization + l1_i_3_cache_utilization

		l1_d_total_cache_ctrl_loops = l1_d_0_total_cache_ctrl_loops + l1_d_1_total_cache_ctrl_loops + l1_d_2_total_cache_ctrl_loops + l1_d_3_total_cache_ctrl_loops
		l1_d_total_accesses = l1_d_0_total_accesses + l1_d_1_total_accesses + l1_d_2_total_accesses + l1_d_3_total_accesses
		l1_d_total_hits = l1_d_0_total_hits + l1_d_1_total_hits + l1_d_2_total_hits + l1_d_3_total_hits
		l1_d_total_misses = l1_d_0_total_misses + l1_d_1_total_misses + l1_d_2_total_misses + l1_d_3_total_misses
		l1_d_miss_rate = l1_d_0_miss_rate + l1_d_1_miss_rate + l1_d_2_miss_rate + l1_d_3_miss_rate
		l1_d_total_reads = l1_d_0_total_reads + l1_d_1_total_reads + l1_d_2_total_reads + l1_d_3_total_reads
		l1_d_total_read_misses = l1_d_0_total_read_misses + l1_d_1_total_read_misses + l1_d_2_total_read_misses + l1_d_3_total_read_misses
		l1_d_read_miss_rate = l1_d_0_read_miss_rate + l1_d_1_read_miss_rate + l1_d_2_read_miss_rate + l1_d_3_read_miss_rate
		l1_d_total_writes = l1_d_0_total_writes + l1_d_1_total_writes + l1_d_2_total_writes + l1_d_3_total_writes
		l1_d_total_write_misses = l1_d_0_total_write_misses + l1_d_1_total_write_misses + l1_d_2_total_write_misses + l1_d_3_total_write_misses
		l1_d_write_miss_rate = l1_d_0_write_miss_rate + l1_d_1_write_miss_rate + l1_d_2_write_miss_rate + l1_d_3_write_miss_rate
		l1_d_total_get = l1_d_0_total_get +  l1_d_1_total_get +  l1_d_2_total_get + l1_d_3_total_get
		l1_d_total_getx = l1_d_0_total_getx + l1_d_1_total_getx + l1_d_2_total_getx + l1_d_3_total_getx
		l1_d_get_miss_rate = l1_d_0_get_miss_rate + l1_d_1_get_miss_rate + l1_d_2_get_miss_rate + l1_d_3_get_miss_rate
		l1_d_getx_miss_rate = l1_d_0_getx_miss_rate + l1_d_1_getx_miss_rate + l1_d_2_getx_miss_rate + l1_d_3_getx_miss_rate
		l1_d_total_upgrades = l1_d_0_total_upgrades + l1_d_1_total_upgrades + l1_d_2_total_upgrades + l1_d_3_total_upgrades
		l1_d_upgrade_miss_rate = l1_d_0_upgrade_miss_rate + l1_d_1_upgrade_miss_rate + l1_d_2_upgrade_miss_rate + l1_d_3_upgrade_miss_rate
		l1_d_total_write_backs = l1_d_0_total_write_backs + l1_d_1_total_write_backs + l1_d_2_total_write_backs + l1_d_3_total_write_backs
		l1_d_cache_utilization = l1_d_0_cache_utilization + l1_d_1_cache_utilization + l1_d_2_cache_utilization + l1_d_3_cache_utilization

		l2_total_cache_ctrl_loops = l2_0_total_cache_ctrl_loops + l2_1_total_cache_ctrl_loops + l2_2_total_cache_ctrl_loops + l2_3_total_cache_ctrl_loops
		l2_total_accesses = l2_0_total_accesses + l2_1_total_accesses + l2_2_total_accesses + l2_3_total_accesses
		l2_total_hits = l2_0_total_hits + l2_1_total_hits + l2_2_total_hits + l2_3_total_hits
		l2_total_misses = l2_0_total_misses + l2_1_total_misses + l2_2_total_misses + l2_3_total_misses
		l2_miss_rate = l2_0_miss_rate + l2_1_miss_rate + l2_2_miss_rate + l2_3_miss_rate
		l2_total_reads = l2_0_total_reads + l2_1_total_reads + l2_2_total_reads + l2_3_total_reads
		l2_total_read_misses = l2_0_total_read_misses + l2_1_total_read_misses + l2_2_total_read_misses + l2_3_total_read_misses
		l2_read_miss_rate = l2_0_read_miss_rate + l2_1_read_miss_rate + l2_2_read_miss_rate + l2_3_read_miss_rate
		l2_total_writes = l2_0_total_writes + l2_1_total_writes + l2_2_total_writes + l2_3_total_writes
		l2_total_write_misses = l2_0_total_write_misses + l2_1_total_write_misses + l2_2_total_write_misses + l2_3_total_write_misses
		l2_write_miss_rate = l2_0_write_miss_rate + l2_1_write_miss_rate + l2_2_write_miss_rate + l2_3_write_miss_rate
		l2_total_get = l2_0_total_get +  l2_1_total_get +  l2_2_total_get + l2_3_total_get
		l2_total_getx = l2_0_total_getx + l2_1_total_getx + l2_2_total_getx + l2_3_total_getx
		l2_get_miss_rate = l2_0_get_miss_rate + l2_1_get_miss_rate + l2_2_get_miss_rate + l2_3_get_miss_rate
		l2_getx_miss_rate = l2_0_getx_miss_rate + l2_1_getx_miss_rate + l2_2_getx_miss_rate + l2_3_getx_miss_rate
		l2_total_upgrades = l2_0_total_upgrades + l2_1_total_upgrades + l2_2_total_upgrades + l2_3_total_upgrades
		l2_upgrade_miss_rate = l2_0_upgrade_miss_rate + l2_1_upgrade_miss_rate + l2_2_upgrade_miss_rate + l2_3_upgrade_miss_rate
		l2_total_write_backs = l2_0_total_write_backs + l2_1_total_write_backs + l2_2_total_write_backs + l2_3_total_write_backs
		l2_cache_utilization = l2_0_cache_utilization + l2_1_cache_utilization + l2_2_cache_utilization + l2_3_cache_utilization

		l3_total_cache_ctrl_loops = l3_0_total_cache_ctrl_loops + l3_1_total_cache_ctrl_loops + l3_2_total_cache_ctrl_loops + l3_3_total_cache_ctrl_loops
		l3_total_accesses = l3_0_total_accesses + l3_1_total_accesses + l3_2_total_accesses + l3_3_total_accesses
		l3_total_hits = l3_0_total_hits + l3_1_total_hits + l3_2_total_hits + l3_3_total_hits
		l3_total_misses = l3_0_total_misses + l3_1_total_misses + l3_2_total_misses + l3_3_total_misses
		l3_miss_rate = l3_0_miss_rate + l3_1_miss_rate + l3_2_miss_rate + l3_3_miss_rate
		l3_total_reads = l3_0_total_reads + l3_1_total_reads + l3_2_total_reads + l3_3_total_reads
		l3_total_read_misses = l3_0_total_read_misses + l3_1_total_read_misses + l3_2_total_read_misses + l3_3_total_read_misses
		l3_read_miss_rate = l3_0_read_miss_rate + l3_1_read_miss_rate + l3_2_read_miss_rate + l3_3_read_miss_rate
		l3_total_writes = l3_0_total_writes + l3_1_total_writes + l3_2_total_writes + l3_3_total_writes
		l3_total_write_misses = l3_0_total_write_misses + l3_1_total_write_misses + l3_2_total_write_misses + l3_3_total_write_misses
		l3_write_miss_rate = l3_0_write_miss_rate + l3_1_write_miss_rate + l3_2_write_miss_rate + l3_3_write_miss_rate
		l3_total_get = l3_0_total_get +  l3_1_total_get +  l3_2_total_get + l3_3_total_get
		l3_total_getx = l3_0_total_getx + l3_1_total_getx + l3_2_total_getx + l3_3_total_getx
		l3_get_miss_rate = l3_0_get_miss_rate + l3_1_get_miss_rate + l3_2_get_miss_rate + l3_3_get_miss_rate
		l3_getx_miss_rate = l3_0_getx_miss_rate + l3_1_getx_miss_rate + l3_2_getx_miss_rate + l3_3_getx_miss_rate
		l3_total_upgrades = l3_0_total_upgrades + l3_1_total_upgrades + l3_2_total_upgrades + l3_3_total_upgrades
		l3_upgrade_miss_rate = l3_0_upgrade_miss_rate + l3_1_upgrade_miss_rate + l3_2_upgrade_miss_rate + l3_3_upgrade_miss_rate
		l3_total_write_backs = l3_0_total_write_backs + l3_1_total_write_backs + l3_2_total_write_backs + l3_3_total_write_backs
		l3_cache_utilization = l3_0_cache_utilization + l3_1_cache_utilization + l3_2_cache_utilization + l3_3_cache_utilization

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

		table_cache_combined = [
		["TotalCacheCtrlLoops", l1_i_total_cache_ctrl_loops, l1_d_total_cache_ctrl_loops, l2_total_cache_ctrl_loops, l3_total_cache_ctrl_loops],
		["TotalAccesses", l1_i_total_accesses, l1_d_total_accesses, l2_total_accesses, l3_total_accesses], 
		["TotalHits", l1_i_total_hits, l1_d_total_hits, l2_total_hits, l3_total_hits],
		["TotalMisses", l1_i_total_misses, l1_d_total_misses, l2_total_misses, l3_total_misses],
		["MissRate", l1_i_miss_rate, l1_d_miss_rate, l2_miss_rate, l3_miss_rate],
		["TotalReads", l1_i_total_reads, l1_d_total_reads, l2_total_reads, l3_total_reads],
		["TotalReadMisses", l1_i_total_read_misses, l1_d_total_read_misses, l2_total_read_misses, l3_total_read_misses],
		["ReadMissRate", l1_i_read_miss_rate, l1_d_read_miss_rate, l2_read_miss_rate, l3_read_miss_rate],
		["TotalWrites", l1_i_total_writes, l1_d_total_writes, l2_total_writes, l3_total_writes],
		["TotalWriteMisses", l1_i_total_write_misses, l1_d_total_write_misses, l2_total_write_misses, l3_total_write_misses],
		["WriteMissRate", l1_i_write_miss_rate, l1_d_write_miss_rate, l2_write_miss_rate, l3_write_miss_rate],
		["TotalGet", l1_i_total_get, l1_d_total_get, l2_total_get, l3_total_get],
		["TotalGetx", l1_i_total_getx, l1_d_total_getx, l2_total_getx, l3_total_getx],
		["GetMissRate", l1_i_get_miss_rate, l1_d_get_miss_rate, l2_get_miss_rate, l3_get_miss_rate],
		["GetxMissRate", l1_i_getx_miss_rate, l1_d_getx_miss_rate, l2_getx_miss_rate, l3_getx_miss_rate],
		["TotalUpgrades", l1_i_total_upgrades, l1_d_total_upgrades, l2_total_upgrades, l3_total_upgrades],
		["UpgradeMissRate", l1_i_upgrade_miss_rate, l1_d_upgrade_miss_rate, l2_upgrade_miss_rate, l3_upgrade_miss_rate],
		["TotalWriteBacks", l1_i_total_write_backs, l1_d_total_write_backs, l2_total_write_backs, l3_total_write_backs],
		["CacheUtilization", l1_i_cache_utilization, l1_d_cache_utilization, l2_cache_utilization, l3_cache_utilization]
		]

	f = open(options.OutFileName, 'a')

	f.write("//Cache Stats//////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')

	###print stats for 1 core (P0)###
	if int(options.NumCores) == 1:
	
		max_title_length = len('Cache stats P0')
		current_title_length = 0

		for tup in table_P0:
			for item in tup[0:1]:
				current_title_length = len(tup[0])
				if max_title_length < current_title_length:
					max_title_length = current_title_length

		#get the largest data element length
		max_element_length = 0
		current_element_length = 0

		for tup in table_P0:
			for item in tup[1:5]:
				current_element_length = len(str(item))
				if max_element_length < current_element_length:
					max_element_length = current_element_length

		max_title_length += 2
		max_element_length += 2
		#print "max title {} max data {}".format(max_title_length, max_element_length)

		title_bar = '-' * (max_title_length - 1)
		data_bar = '-' * (max_element_length - 1)

		#print the title and bars
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format("Cache stats P0",'I$', 'D$', 'L2$', 'L3$', title_width=max_title_length, data_width=max_element_length) + '\n')
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

		#print the table's data
		for tup in table_P0:
			f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')

		f.write('\n')

	###print stats combined 4 cores###
	if int(options.NumCores) == 4:
		#get the largest title length	
		max_title_length = len('Cache stats combined')
		current_title_length = 0

		for tup in table_cache_combined:
			for item in tup[0:1]:
				current_title_length = len(tup[0])
				if max_title_length < current_title_length:
					max_title_length = current_title_length

		#get the largest data element length
		max_element_length = 0
		current_element_length = 0

		for tup in table_cache_combined:
			for item in tup[1:5]:
				current_element_length = len(str(item))
				if max_element_length < current_element_length:
					max_element_length = current_element_length

		max_title_length += 2
		max_element_length += 2
		#print "max title {} max data {}".format(max_title_length, max_element_length)
	
		title_bar = '-' * (max_title_length - 1)
		data_bar = '-' * (max_element_length - 1)

		#print the title and bars
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format("Cache stats combined",'I$', 'D$', 'L2$', 'L3$', title_width=max_title_length, data_width=max_element_length) + '\n')
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

		#print the table's data
		for tup in table_cache_combined:
			f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')

		f.write('\n')
		
	f.close()

	return

def print_switch_stats(options):
	switch_data = ConfigParser.ConfigParser()
	switch_data.read(options.InFileName)

	###print stats core 1 switch###
	if int(options.NumCores) == 1 or int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		s_0_total_ctrl_loops = switch_data.getint('Switch_0', 'NumberSwitchCtrlLoops')
		s_0_occupance = switch_data.getfloat('Switch_0', 'SwitchOccupance')
		s_0_total_links_formed = switch_data.getint('Switch_0', 'NumberLinks')
		s_0_max_links_formed = switch_data.getint('Switch_0', 'MaxNumberLinks')
		s_0_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_0', 'AveNumberLinksPerCtrlLoop')
		s_0_north_io_transfers = switch_data.getint('Switch_0', 'NorthIOTransfers')
		s_0_north_io_cycles = switch_data.getint('Switch_0', 'NorthIOCycles')
		s_0_north_io_bytes_transfered = switch_data.getfloat('Switch_0', 'NorthIOBytesTransfered')
		s_0_north_rxqueue_max_depth = switch_data.getint('Switch_0', 'NorthRxQueueMaxDepth')
		s_0_north_rxqueue_ave_depth = switch_data.getfloat('Switch_0', 'NorthRxQueueAveDepth')
		s_0_north_txqueue_max_depth = switch_data.getint('Switch_0', 'NorthTxQueueMaxDepth')
		s_0_north_txqueue_ave_depth = switch_data.getfloat('Switch_0', 'NorthTxQueueAveDepth')
		s_0_east_io_transfers = switch_data.getint('Switch_0', 'EastIOTransfers')
		s_0_east_io_cycles = switch_data.getint('Switch_0', 'EastIOCycles')
		s_0_east_io_bytes_transfered = switch_data.getint('Switch_0', 'EastIOBytesTransfered')
		s_0_east_rxqueue_max_depth = switch_data.getint('Switch_0', 'EastRxQueueMaxDepth')
		s_0_east_rxqueue_ave_depth = switch_data.getfloat('Switch_0', 'EastRxQueueAveDepth')
		s_0_east_txqueue_max_depth = switch_data.getint('Switch_0', 'EastTxQueueMaxDepth')
		s_0_east_txqueue_ave_depth = switch_data.getfloat('Switch_0', 'EastTxQueueAveDepth')
		s_0_south_io_transfers = switch_data.getint('Switch_0', 'SouthIOTransfers')
		s_0_south_io_cycles = switch_data.getint('Switch_0', 'SouthIOCycles')
		s_0_south_io_bytes_transfered = switch_data.getint('Switch_0', 'SouthIOBytesTransfered')
		s_0_south_rxqueue_max_depth = switch_data.getint('Switch_0', 'SouthRxQueueMaxDepth')
		s_0_south_rxqueue_ave_depth = switch_data.getfloat('Switch_0', 'SouthRxQueueAveDepth')
		s_0_south_txqueue_max_depth = switch_data.getint('Switch_0', 'SouthTxQueueMaxDepth')
		s_0_south_txqueue_ave_depth = switch_data.getfloat('Switch_0', 'SouthTxQueueAveDepth')
		s_0_west_io_transfers = switch_data.getint('Switch_0', 'WestIOTransfers')
		s_0_west_io_cycles = switch_data.getint('Switch_0', 'WestIOCycles')
		s_0_west_io_bytes_transfered = switch_data.getint('Switch_0', 'WestIOBytesTransfered')
		s_0_west_rxqueue_max_depth = switch_data.getint('Switch_0', 'WestRxQueueMaxDepth')
		s_0_west_rxqueue_ave_depth = switch_data.getfloat('Switch_0', 'WestRxQueueAveDepth')
		s_0_west_txqueue_max_depth = switch_data.getint('Switch_0', 'WestTxQueueMaxDepth')
		s_0_west_txqueue_ave_depth = switch_data.getfloat('Switch_0', 'WestTxQueueAveDepth')

	if int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		s_1_total_ctrl_loops = switch_data.getint('Switch_1', 'NumberSwitchCtrlLoops')
		s_1_occupance = switch_data.getfloat('Switch_1', 'SwitchOccupance')
		s_1_total_links_formed = switch_data.getint('Switch_1', 'NumberLinks')
		s_1_max_links_formed = switch_data.getint('Switch_1', 'MaxNumberLinks')
		s_1_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_1', 'AveNumberLinksPerCtrlLoop')
		s_1_north_io_transfers = switch_data.getint('Switch_1', 'NorthIOTransfers')
		s_1_north_io_cycles = switch_data.getint('Switch_1', 'NorthIOCycles')
		s_1_north_io_bytes_transfered = switch_data.getint('Switch_1', 'NorthIOBytesTransfered')
		s_1_north_rxqueue_max_depth = switch_data.getint('Switch_1', 'NorthRxQueueMaxDepth')
		s_1_north_rxqueue_ave_depth = switch_data.getfloat('Switch_1', 'NorthRxQueueAveDepth')
		s_1_north_txqueue_max_depth = switch_data.getint('Switch_1', 'NorthTxQueueMaxDepth')
		s_1_north_txqueue_ave_depth = switch_data.getfloat('Switch_1', 'NorthTxQueueAveDepth')
		s_1_east_io_transfers = switch_data.getint('Switch_1', 'EastIOTransfers')
		s_1_east_io_cycles = switch_data.getint('Switch_1', 'EastIOCycles')
		s_1_east_io_bytes_transfered = switch_data.getint('Switch_1', 'EastIOBytesTransfered')
		s_1_east_rxqueue_max_depth = switch_data.getint('Switch_1', 'EastRxQueueMaxDepth')
		s_1_east_rxqueue_ave_depth = switch_data.getfloat('Switch_1', 'EastRxQueueAveDepth')
		s_1_east_txqueue_max_depth = switch_data.getint('Switch_1', 'EastTxQueueMaxDepth')
		s_1_east_txqueue_ave_depth = switch_data.getfloat('Switch_1', 'EastTxQueueAveDepth')
		s_1_south_io_transfers = switch_data.getint('Switch_1', 'SouthIOTransfers')
		s_1_south_io_cycles = switch_data.getint('Switch_1', 'SouthIOCycles')
		s_1_south_io_bytes_transfered = switch_data.getint('Switch_1', 'SouthIOBytesTransfered')
		s_1_south_rxqueue_max_depth = switch_data.getint('Switch_1', 'SouthRxQueueMaxDepth')
		s_1_south_rxqueue_ave_depth = switch_data.getfloat('Switch_1', 'SouthRxQueueAveDepth')
		s_1_south_txqueue_max_depth = switch_data.getint('Switch_1', 'SouthTxQueueMaxDepth')
		s_1_south_txqueue_ave_depth = switch_data.getfloat('Switch_1', 'SouthTxQueueAveDepth')
		s_1_west_io_transfers = switch_data.getint('Switch_1', 'WestIOTransfers')
		s_1_west_io_cycles = switch_data.getint('Switch_1', 'WestIOCycles')
		s_1_west_io_bytes_transfered = switch_data.getint('Switch_1', 'WestIOBytesTransfered')
		s_1_west_rxqueue_max_depth = switch_data.getint('Switch_1', 'WestRxQueueMaxDepth')
		s_1_west_rxqueue_ave_depth = switch_data.getfloat('Switch_1', 'WestRxQueueAveDepth')
		s_1_west_txqueue_max_depth = switch_data.getint('Switch_1', 'WestTxQueueMaxDepth')
		s_1_west_txqueue_ave_depth = switch_data.getfloat('Switch_1', 'WestTxQueueAveDepth')

	if int(options.NumCores) == 3 or int(options.NumCores) == 4:
		s_2_total_ctrl_loops = switch_data.getint('Switch_2', 'NumberSwitchCtrlLoops')
		s_2_occupance = switch_data.getfloat('Switch_2', 'SwitchOccupance')
		s_2_total_links_formed = switch_data.getint('Switch_2', 'NumberLinks')
		s_2_max_links_formed = switch_data.getint('Switch_2', 'MaxNumberLinks')
		s_2_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_2', 'AveNumberLinksPerCtrlLoop')
		s_2_north_io_transfers = switch_data.getint('Switch_2', 'NorthIOTransfers')
		s_2_north_io_cycles = switch_data.getint('Switch_2', 'NorthIOCycles')
		s_2_north_io_bytes_transfered = switch_data.getint('Switch_2', 'NorthIOBytesTransfered')
		s_2_north_rxqueue_max_depth = switch_data.getint('Switch_2', 'NorthRxQueueMaxDepth')
		s_2_north_rxqueue_ave_depth = switch_data.getfloat('Switch_2', 'NorthRxQueueAveDepth')
		s_2_north_txqueue_max_depth = switch_data.getint('Switch_2', 'NorthTxQueueMaxDepth')
		s_2_north_txqueue_ave_depth = switch_data.getfloat('Switch_2', 'NorthTxQueueAveDepth')
		s_2_east_io_transfers = switch_data.getint('Switch_2', 'EastIOTransfers')
		s_2_east_io_cycles = switch_data.getint('Switch_2', 'EastIOCycles')
		s_2_east_io_bytes_transfered = switch_data.getint('Switch_2', 'EastIOBytesTransfered')
		s_2_east_rxqueue_max_depth = switch_data.getint('Switch_2', 'EastRxQueueMaxDepth')
		s_2_east_rxqueue_ave_depth = switch_data.getfloat('Switch_2', 'EastRxQueueAveDepth')
		s_2_east_txqueue_max_depth = switch_data.getint('Switch_2', 'EastTxQueueMaxDepth')
		s_2_east_txqueue_ave_depth = switch_data.getfloat('Switch_2', 'EastTxQueueAveDepth')
		s_2_south_io_transfers = switch_data.getint('Switch_2', 'SouthIOTransfers')
		s_2_south_io_cycles = switch_data.getint('Switch_2', 'SouthIOCycles')
		s_2_south_io_bytes_transfered = switch_data.getint('Switch_2', 'SouthIOBytesTransfered')
		s_2_south_rxqueue_max_depth = switch_data.getint('Switch_2', 'SouthRxQueueMaxDepth')
		s_2_south_rxqueue_ave_depth = switch_data.getfloat('Switch_2', 'SouthRxQueueAveDepth')
		s_2_south_txqueue_max_depth = switch_data.getint('Switch_2', 'SouthTxQueueMaxDepth')
		s_2_south_txqueue_ave_depth = switch_data.getfloat('Switch_2', 'SouthTxQueueAveDepth')
		s_2_west_io_transfers = switch_data.getint('Switch_2', 'WestIOTransfers')
		s_2_west_io_cycles = switch_data.getint('Switch_2', 'WestIOCycles')
		s_2_west_io_bytes_transfered = switch_data.getint('Switch_2', 'WestIOBytesTransfered')
		s_2_west_rxqueue_max_depth = switch_data.getint('Switch_2', 'WestRxQueueMaxDepth')
		s_2_west_rxqueue_ave_depth = switch_data.getfloat('Switch_2', 'WestRxQueueAveDepth')
		s_2_west_txqueue_max_depth = switch_data.getint('Switch_2', 'WestTxQueueMaxDepth')
		s_2_west_txqueue_ave_depth = switch_data.getfloat('Switch_2', 'WestTxQueueAveDepth')

	if int(options.NumCores) == 4:
		s_3_total_ctrl_loops = switch_data.getint('Switch_3', 'NumberSwitchCtrlLoops')
		s_3_occupance = switch_data.getfloat('Switch_3', 'SwitchOccupance')
		s_3_total_links_formed = switch_data.getint('Switch_3', 'NumberLinks')
		s_3_max_links_formed = switch_data.getint('Switch_3', 'MaxNumberLinks')
		s_3_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_3', 'AveNumberLinksPerCtrlLoop')
		s_3_north_io_transfers = switch_data.getint('Switch_3', 'NorthIOTransfers')
		s_3_north_io_cycles = switch_data.getint('Switch_3', 'NorthIOCycles')
		s_3_north_io_bytes_transfered = switch_data.getint('Switch_3', 'NorthIOBytesTransfered')
		s_3_north_rxqueue_max_depth = switch_data.getint('Switch_3', 'NorthRxQueueMaxDepth')
		s_3_north_rxqueue_ave_depth = switch_data.getfloat('Switch_3', 'NorthRxQueueAveDepth')
		s_3_north_txqueue_max_depth = switch_data.getint('Switch_3', 'NorthTxQueueMaxDepth')
		s_3_north_txqueue_ave_depth = switch_data.getfloat('Switch_3', 'NorthTxQueueAveDepth')
		s_3_east_io_transfers = switch_data.getint('Switch_3', 'EastIOTransfers')
		s_3_east_io_cycles = switch_data.getint('Switch_3', 'EastIOCycles')
		s_3_east_io_bytes_transfered = switch_data.getint('Switch_3', 'EastIOBytesTransfered')
		s_3_east_io_bytes_transfered = switch_data.getint('Switch_3', 'EastIOBytesTransfered')
		s_3_east_rxqueue_max_depth = switch_data.getint('Switch_3', 'EastRxQueueMaxDepth')
		s_3_east_rxqueue_ave_depth = switch_data.getfloat('Switch_3', 'EastRxQueueAveDepth')
		s_3_east_txqueue_max_depth = switch_data.getint('Switch_3', 'EastTxQueueMaxDepth')
		s_3_east_txqueue_ave_depth = switch_data.getfloat('Switch_3', 'EastTxQueueAveDepth')
		s_3_south_io_transfers = switch_data.getint('Switch_3', 'SouthIOTransfers')
		s_3_south_io_cycles = switch_data.getint('Switch_3', 'SouthIOCycles')
		s_3_south_io_bytes_transfered = switch_data.getint('Switch_3', 'SouthIOBytesTransfered')
		s_3_south_rxqueue_max_depth = switch_data.getint('Switch_3', 'SouthRxQueueMaxDepth')
		s_3_south_rxqueue_ave_depth = switch_data.getfloat('Switch_3', 'SouthRxQueueAveDepth')
		s_3_south_txqueue_max_depth = switch_data.getint('Switch_3', 'SouthTxQueueMaxDepth')
		s_3_south_txqueue_ave_depth = switch_data.getfloat('Switch_3', 'SouthTxQueueAveDepth')
		s_3_west_io_transfers = switch_data.getint('Switch_3', 'WestIOTransfers')
		s_3_west_io_cycles = switch_data.getint('Switch_3', 'WestIOCycles')
		s_3_west_io_bytes_transfered = switch_data.getint('Switch_3', 'WestIOBytesTransfered')
		s_3_west_rxqueue_max_depth = switch_data.getint('Switch_3', 'WestRxQueueMaxDepth')
		s_3_west_rxqueue_ave_depth = switch_data.getfloat('Switch_3', 'WestRxQueueAveDepth')
		s_3_west_txqueue_max_depth = switch_data.getint('Switch_3', 'WestTxQueueMaxDepth')
		s_3_west_txqueue_ave_depth = switch_data.getfloat('Switch_3', 'WestTxQueueAveDepth')


	s_sa_total_ctrl_loops = switch_data.getint('Switch_SA', 'NumberSwitchCtrlLoops')
	s_sa_occupance = switch_data.getfloat('Switch_SA', 'SwitchOccupance')
	s_sa_total_links_formed = switch_data.getint('Switch_SA', 'NumberLinks')
	s_sa_max_links_formed = switch_data.getint('Switch_SA', 'MaxNumberLinks')
	s_sa_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_SA', 'AveNumberLinksPerCtrlLoop')
	s_sa_north_io_transfers = switch_data.getint('Switch_SA', 'NorthIOTransfers')
	s_sa_north_io_cycles = switch_data.getint('Switch_SA', 'NorthIOCycles')
	s_sa_north_io_bytes_transfered = switch_data.getint('Switch_SA', 'NorthIOBytesTransfered')
	s_sa_north_rxqueue_max_depth = switch_data.getint('Switch_SA', 'NorthRxQueueMaxDepth')
	s_sa_north_rxqueue_ave_depth = switch_data.getfloat('Switch_SA', 'NorthRxQueueAveDepth')
	s_sa_north_txqueue_max_depth = switch_data.getint('Switch_SA', 'NorthTxQueueMaxDepth')
	s_sa_north_txqueue_ave_depth = switch_data.getfloat('Switch_SA', 'NorthTxQueueAveDepth')
	s_sa_east_io_transfers = switch_data.getint('Switch_SA', 'EastIOTransfers')
	s_sa_east_io_cycles = switch_data.getint('Switch_SA', 'EastIOCycles')
	s_sa_east_io_bytes_transfered = switch_data.getint('Switch_SA', 'EastIOBytesTransfered')
	s_sa_east_io_bytes_transfered = switch_data.getint('Switch_SA', 'EastIOBytesTransfered')
	s_sa_east_io_bytes_transfered = switch_data.getint('Switch_SA', 'EastIOBytesTransfered')
	s_sa_east_rxqueue_max_depth = switch_data.getint('Switch_SA', 'EastRxQueueMaxDepth')
	s_sa_east_rxqueue_ave_depth = switch_data.getfloat('Switch_SA', 'EastRxQueueAveDepth')
	s_sa_east_txqueue_max_depth = switch_data.getint('Switch_SA', 'EastTxQueueMaxDepth')
	s_sa_east_txqueue_ave_depth = switch_data.getfloat('Switch_SA', 'EastTxQueueAveDepth')
	s_sa_south_io_transfers = switch_data.getint('Switch_SA', 'SouthIOTransfers')
	s_sa_south_io_cycles = switch_data.getint('Switch_SA', 'SouthIOCycles')
	s_sa_south_io_bytes_transfered = switch_data.getint('Switch_SA', 'SouthIOBytesTransfered')
	s_sa_south_io_bytes_transfered = switch_data.getint('Switch_SA', 'SouthIOBytesTransfered')
	s_sa_south_rxqueue_max_depth = switch_data.getint('Switch_SA', 'SouthRxQueueMaxDepth')
	s_sa_south_rxqueue_ave_depth = switch_data.getfloat('Switch_SA', 'SouthRxQueueAveDepth')
	s_sa_south_txqueue_max_depth = switch_data.getint('Switch_SA', 'SouthTxQueueMaxDepth')
	s_sa_south_txqueue_ave_depth = switch_data.getfloat('Switch_SA', 'SouthTxQueueAveDepth')
	s_sa_west_io_transfers = switch_data.getint('Switch_SA', 'WestIOTransfers')
	s_sa_west_io_cycles = switch_data.getint('Switch_SA', 'WestIOCycles')
	s_sa_west_io_bytes_transfered = switch_data.getint('Switch_SA', 'WestIOBytesTransfered')
	s_sa_west_io_bytes_transfered = switch_data.getint('Switch_SA', 'WestIOBytesTransfered')
	s_sa_west_rxqueue_max_depth = switch_data.getint('Switch_SA', 'WestRxQueueMaxDepth')
	s_sa_west_rxqueue_ave_depth = switch_data.getfloat('Switch_SA', 'WestRxQueueAveDepth')
	s_sa_west_txqueue_max_depth = switch_data.getint('Switch_SA', 'WestTxQueueMaxDepth')
	s_sa_west_txqueue_ave_depth = switch_data.getfloat('Switch_SA', 'WestTxQueueAveDepth')

	if int(options.NumCores) == 1:
		table_switch_data_p0 = [
		["TotalSwitchCtrlLoops", s_0_total_ctrl_loops, s_sa_total_ctrl_loops],
		["SwitchOccupancy", s_0_occupance, s_sa_occupance],
		["TotalLinksFormed", s_0_total_links_formed, s_sa_total_links_formed],
		["MaxLinksFormed", s_0_max_links_formed, s_sa_max_links_formed],
		["TotalAveNumberLinksPerCtrlLoop", s_0_ave_links_formed_per_ctrl_loop, s_sa_ave_links_formed_per_ctrl_loop],
		["NorthIOTransfers", s_0_north_io_transfers, s_sa_north_io_transfers ],
		["NorthIOCycles", s_0_north_io_cycles, s_sa_north_io_cycles],
		["NorthIOBytesTransfered", s_0_north_io_bytes_transfered, s_sa_north_io_bytes_transfered],
		["NorthRxQueueMaxDepth", s_0_north_rxqueue_max_depth, s_sa_north_rxqueue_max_depth],
		["NorthRxQueueAveDepth", s_0_north_rxqueue_ave_depth, s_sa_north_rxqueue_ave_depth],
		["NorthTxQueueMaxDepth", s_0_north_txqueue_max_depth, s_sa_north_txqueue_max_depth],
		["NorthTxQueueAveDepth", s_0_north_txqueue_ave_depth, s_sa_north_txqueue_ave_depth],
		["EastIOTransfers", s_0_east_io_transfers, s_sa_east_io_transfers],
		["EastIOCycles", s_0_east_io_cycles, s_sa_east_io_cycles],
		["EastIOBytesTransfered", s_0_east_io_bytes_transfered, s_sa_east_io_bytes_transfered],
		["EastRxQueueMaxDepth", s_0_east_rxqueue_max_depth, s_sa_east_rxqueue_max_depth],
		["EastRxQueueAveDepth", s_0_east_rxqueue_ave_depth, s_sa_east_rxqueue_ave_depth],
		["EastTxQueueMaxDepth", s_0_east_txqueue_max_depth, s_sa_east_txqueue_max_depth],
		["EastTxQueueAveDepth", s_0_east_txqueue_ave_depth, s_sa_east_txqueue_ave_depth],
		["SouthIOTransfers", s_0_south_io_transfers, s_sa_south_io_transfers],
		["SouthIOCycles", s_0_south_io_cycles, s_sa_south_io_cycles],
		["SouthIOBytesTransfered", s_0_south_io_bytes_transfered, s_sa_south_io_bytes_transfered],
		["SouthRxQueueMaxDepth", s_0_south_rxqueue_max_depth, s_sa_south_rxqueue_max_depth],
		["SouthRxQueueAveDepth", s_0_south_rxqueue_ave_depth, s_sa_south_rxqueue_ave_depth],
		["SouthTxQueueMaxDepth", s_0_south_txqueue_max_depth, s_sa_south_txqueue_max_depth],
		["SouthTxQueueAveDepth", s_0_south_txqueue_ave_depth, s_sa_south_txqueue_ave_depth],
		["WestIOTransfers", s_0_west_io_transfers, s_sa_west_io_transfers],
		["WestIOCycles", s_0_west_io_cycles, s_sa_west_io_cycles],
		["WestIOBytesTransfered", s_0_west_io_bytes_transfered, s_sa_west_io_bytes_transfered],
		["WestRxQueueMaxDepth", s_0_west_rxqueue_max_depth, s_sa_west_rxqueue_max_depth],
		["WestRxQueueAveDepth", s_0_west_rxqueue_ave_depth, s_sa_west_rxqueue_ave_depth],
		["WestTxQueueMaxDepth", s_0_west_txqueue_max_depth, s_sa_west_txqueue_max_depth],
		["WestTxQueueAveDepth", s_0_west_txqueue_ave_depth, s_sa_west_txqueue_ave_depth],
		]

	if int(options.NumCores) == 4:
		table_switch_data_p4 = [
		["TotalSwitchCtrlLoops", s_0_total_ctrl_loops, s_1_total_ctrl_loops, s_2_total_ctrl_loops, s_3_total_ctrl_loops, s_sa_total_ctrl_loops],
		["SwitchOccupancy", s_0_occupance, s_1_occupance, s_2_occupance, s_3_occupance, s_sa_occupance],
		["TotalLinksFormed", s_0_total_links_formed, s_1_total_links_formed, s_2_total_links_formed, s_3_total_links_formed, s_sa_total_ctrl_loops],
		["MaxLinksFormed", s_0_max_links_formed, s_1_max_links_formed, s_2_max_links_formed, s_3_max_links_formed, s_sa_max_links_formed],
		["TotalAveNumberLinksPerCtrlLoop", s_0_ave_links_formed_per_ctrl_loop, s_1_ave_links_formed_per_ctrl_loop, s_2_ave_links_formed_per_ctrl_loop, s_3_ave_links_formed_per_ctrl_loop, s_sa_ave_links_formed_per_ctrl_loop],
		["NorthIOTransfers", s_0_north_io_transfers, s_1_north_io_transfers, s_2_north_io_transfers, s_3_north_io_transfers, s_sa_north_io_transfers ],
		["NorthIOCycles", s_0_north_io_cycles, s_1_north_io_cycles, s_2_north_io_cycles, s_3_north_io_cycles, s_sa_north_io_cycles],
		["NorthIOBytesTransfered", s_0_north_io_bytes_transfered, s_1_north_io_bytes_transfered, s_2_north_io_bytes_transfered, s_3_north_io_bytes_transfered, s_sa_north_io_bytes_transfered],
		["NorthRxQueueMaxDepth", s_0_north_rxqueue_max_depth, s_1_north_rxqueue_max_depth, s_2_north_rxqueue_max_depth, s_3_north_rxqueue_max_depth, s_sa_north_rxqueue_max_depth],
		["NorthRxQueueAveDepth", s_0_north_rxqueue_ave_depth, s_1_north_rxqueue_ave_depth, s_2_north_rxqueue_ave_depth, s_3_north_rxqueue_ave_depth, s_sa_north_rxqueue_ave_depth],
		["NorthTxQueueMaxDepth", s_0_north_txqueue_max_depth, s_1_north_txqueue_max_depth, s_2_north_txqueue_max_depth, s_3_north_txqueue_max_depth, s_sa_north_txqueue_max_depth],
		["NorthTxQueueAveDepth", s_0_north_txqueue_ave_depth, s_1_north_txqueue_ave_depth, s_2_north_txqueue_ave_depth, s_3_north_txqueue_ave_depth, s_sa_north_txqueue_ave_depth],
		["EastIOTransfers", s_0_east_io_transfers, s_1_east_io_transfers, s_2_east_io_transfers, s_3_east_io_transfers, s_sa_east_io_transfers],
		["EastIOCycles", s_0_east_io_cycles, s_1_east_io_cycles, s_2_east_io_cycles, s_3_east_io_cycles, s_sa_east_io_cycles],
		["EastIOBytesTransfered", s_0_east_io_bytes_transfered, s_1_east_io_bytes_transfered, s_2_east_io_bytes_transfered, s_3_east_io_bytes_transfered, s_sa_east_io_bytes_transfered],
		["EastRxQueueMaxDepth", s_0_east_rxqueue_max_depth, s_1_east_rxqueue_max_depth, s_2_east_rxqueue_max_depth, s_3_east_rxqueue_max_depth, s_sa_east_rxqueue_max_depth],
		["EastRxQueueAveDepth", s_0_east_rxqueue_ave_depth, s_1_east_rxqueue_ave_depth, s_2_east_rxqueue_ave_depth, s_3_east_rxqueue_ave_depth, s_sa_east_rxqueue_ave_depth],
		["EastTxQueueMaxDepth", s_0_east_txqueue_max_depth, s_1_east_txqueue_max_depth, s_2_east_txqueue_max_depth, s_3_east_txqueue_max_depth, s_sa_east_txqueue_max_depth],
		["EastTxQueueAveDepth", s_0_east_txqueue_ave_depth, s_1_east_txqueue_ave_depth, s_2_east_txqueue_ave_depth, s_3_east_txqueue_ave_depth, s_sa_east_txqueue_ave_depth],
		["SouthIOTransfers", s_0_south_io_transfers, s_1_south_io_transfers, s_2_south_io_transfers, s_3_south_io_transfers, s_sa_south_io_transfers],
		["SouthIOCycles", s_0_south_io_cycles, s_1_south_io_cycles, s_2_south_io_cycles, s_3_south_io_cycles, s_sa_south_io_cycles],
		["SouthIOBytesTransfered", s_0_south_io_bytes_transfered, s_1_south_io_bytes_transfered, s_2_south_io_bytes_transfered, s_3_south_io_bytes_transfered, s_sa_south_io_bytes_transfered],
		["SouthRxQueueMaxDepth", s_0_south_rxqueue_max_depth, s_1_south_rxqueue_max_depth, s_2_south_rxqueue_max_depth, s_3_south_rxqueue_max_depth, s_sa_south_rxqueue_max_depth],
		["SouthRxQueueAveDepth", s_0_south_rxqueue_ave_depth, s_1_south_rxqueue_ave_depth, s_2_south_rxqueue_ave_depth, s_3_south_rxqueue_ave_depth, s_sa_south_rxqueue_ave_depth],
		["SouthTxQueueMaxDepth", s_0_south_txqueue_max_depth, s_1_south_txqueue_max_depth, s_2_south_txqueue_max_depth, s_3_south_txqueue_max_depth, s_sa_south_txqueue_max_depth],
		["SouthTxQueueAveDepth", s_0_south_txqueue_ave_depth, s_1_south_txqueue_ave_depth, s_2_south_txqueue_ave_depth, s_3_south_txqueue_ave_depth, s_sa_south_txqueue_ave_depth],
		["WestIOTransfers", s_0_west_io_transfers, s_1_west_io_transfers, s_2_west_io_transfers, s_3_west_io_transfers, s_sa_west_io_transfers],
		["WestIOCycles", s_0_west_io_cycles, s_1_west_io_cycles, s_2_west_io_cycles, s_3_west_io_cycles, s_sa_west_io_cycles],
		["WestIOBytesTransfered", s_0_west_io_bytes_transfered, s_1_west_io_bytes_transfered, s_2_west_io_bytes_transfered, s_3_west_io_bytes_transfered, s_sa_west_io_bytes_transfered],
		["WestRxQueueMaxDepth", s_0_west_rxqueue_max_depth, s_1_west_rxqueue_max_depth, s_2_west_rxqueue_max_depth, s_3_west_rxqueue_max_depth, s_sa_west_rxqueue_max_depth],
		["WestRxQueueAveDepth", s_0_west_rxqueue_ave_depth, s_1_west_rxqueue_ave_depth, s_2_west_rxqueue_ave_depth, s_3_west_rxqueue_ave_depth, s_sa_west_rxqueue_ave_depth],
		["WestTxQueueMaxDepth", s_0_west_txqueue_max_depth, s_1_west_txqueue_max_depth, s_2_west_txqueue_max_depth, s_3_west_txqueue_max_depth, s_sa_west_txqueue_max_depth],
		["WestTxQueueAveDepth", s_0_west_txqueue_ave_depth, s_1_west_txqueue_ave_depth, s_2_west_txqueue_ave_depth, s_3_west_txqueue_ave_depth, s_sa_west_txqueue_ave_depth],
		]

	f = open(options.OutFileName, 'a')
	f.write('//Switch Stats/////////////////////////////////////////////////' +'\n')
	f.write('///////////////////////////////////////////////////////////////'  + '\n\n')	
	
	#get the largest title length	
	max_title_length = len('Stat Switches')
	current_title_length = 0

	if int(options.NumCores) == 1:
		for tup in table_switch_data_p0:
			for item in tup[0:1]:
				current_title_length = len(tup[0])
				if max_title_length < current_title_length:
					max_title_length = current_title_length

		#get the largest data element length
		max_element_length = 0
		current_element_length = 0

		for tup in table_switch_data_p0:
			for item in tup[1:5]:
				current_element_length = len(str(item))
				if max_element_length < current_element_length:
					max_element_length = current_element_length

		max_title_length += 2
		max_element_length += 2
		#print "max title {} max data {}".format(max_title_length, max_element_length)
	
		title_bar = '-' * (max_title_length - 1)
		data_bar = '-' * (max_element_length - 1)

		#print the title and bars
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}".format("Stat Switch",'S0', 'S1', title_width=max_title_length, data_width=max_element_length) + '\n')
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

		#print the table's data
		for tup in table_switch_data_p0:
			f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], title_width=max_title_length, data_width=max_element_length) + '\n')

		f.write('\n')

	if int(options.NumCores) == 4:
		for tup in table_switch_data_p4:
			for item in tup[0:1]:
				current_title_length = len(tup[0])
				if max_title_length < current_title_length:
					max_title_length = current_title_length

		#get the largest data element length
		max_element_length = 0
		current_element_length = 0

		for tup in table_switch_data_p4:
			for item in tup[1:5]:
				current_element_length = len(str(item))
				if max_element_length < current_element_length:
					max_element_length = current_element_length

		max_title_length += 2
		max_element_length += 2
		#print "max title {} max data {}".format(max_title_length, max_element_length)
	
		title_bar = '-' * (max_title_length - 1)
		data_bar = '-' * (max_element_length - 1)

		#print the title and bars
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format("Stat Switch",'S0', 'S1', 'S2', 'S3', 'S4', title_width=max_title_length, data_width=max_element_length) + '\n')
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, data_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

		#print the table's data
		for tup in table_switch_data_p4:
			f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], tup[5], title_width=max_title_length, data_width=max_element_length) + '\n')

		f.write('\n')
	

	f.close
	return



def print_samc_stats(options):

	sa_data = ConfigParser.ConfigParser()
	sa_data.read(options.InFileName)

	sa_total_ctrl_loops = sa_data.getint('SystemAgent', 'TotalCtrlLoops')
	sa_total_mc_loads = sa_data.getint('SystemAgent', 'MCLoads')
	sa_total_mc_stores = sa_data.getint('SystemAgent', 'MCStores')
	sa_total_mc_returns = sa_data.getint('SystemAgent', 'MCReturns')
	sa_total_north_io_busy_cycles = sa_data.getint('SystemAgent', 'NorthIOBusyCycles')
	sa_max_north_rx_queue_depth = sa_data.getint('SystemAgent', 'MaxNorthRxQueueDepth')
	sa_ave_north_rx_queue_depth = sa_data.getfloat('SystemAgent', 'AveNorthRxQueueDepth')
	sa_max_north_tx_queue_depth = sa_data.getint('SystemAgent', 'MaxNorthTxQueueDepth')
	sa_ave_north_tx_queue_depth = sa_data.getfloat('SystemAgent', 'AveNorthTxQueueDepth')
	sa_total_south_io_busy_cycles = sa_data.getint('SystemAgent', 'SouthIOBusyCycles')
	sa_max_soutth_rx_queue_depth = sa_data.getint('SystemAgent', 'MaxSouthRxQueueDepth')
	sa_ave_south_rx_queue_depth = sa_data.getfloat('SystemAgent', 'AveSouthRxQueueDepth')
	sa_max_south_tx_queue_depth = sa_data.getint('SystemAgent', 'MaxSouthTxQueueDepth')
	sa_ave_south_tx_queue_depth = sa_data.getfloat('SystemAgent', 'AveSouthTxQueueDepth')

	mc_total_ctrl_loops = sa_data.getint('MemCtrl', 'MemCtrlBusyCycles')
	mc_total_reads = sa_data.getint('MemCtrl', 'TotalReads')
	mc_total_writes = sa_data.getint('MemCtrl', 'TotalWrites')
	mc_io_busy_cycles = sa_data.getint('MemCtrl', 'IOBusyCycles')
	mc_max_north_rx_queue_depth = sa_data.getint('MemCtrl', 'RxMax')
	mc_max_north_tx_queue_depth = sa_data.getint('MemCtrl', 'TxMax')
	mc_dram_busy_cycles = sa_data.getint('MemCtrl', 'DramBusyCycles')
	mc_dram_ave_read_lat = sa_data.getfloat('MemCtrl', 'AveDramReadLat')
	mc_dram_ave_write_lat = sa_data.getfloat('MemCtrl', 'AveDramWriteLat')
	mc_dram_ave_total_lat_cyc = sa_data.getfloat('MemCtrl', 'AveDramTotalLat(cycles)')
	mc_dram_ave_total_lat_ns = sa_data.getfloat('MemCtrl', 'AveDramTotalLat(nanoseconds)')
	mc_dram_read_min_lat = sa_data.getint('MemCtrl', 'ReadMinLat')
	mc_dram_read_max_lat = sa_data.getint('MemCtrl', 'ReadMaxLat')
	mc_dram_write_min_lat = sa_data.getint('MemCtrl', 'WriteMinLat')
	mc_dram_write_max_lat = sa_data.getint('MemCtrl', 'WriteMaxLat')
	mc_dram_max_queue_depth = sa_data.getint('MemCtrl', 'DramMaxQueueDepth')
	mc_dram_ave_queue_depth = sa_data.getfloat('MemCtrl', 'DramAveQueueDepth')
	mc_dram_total_bytes_read = sa_data.getint('MemCtrl', 'ByteRead')
	mc_dram_total_bytes_written = sa_data.getint('MemCtrl', 'BytesWrote')

	#bridge some of the stats so we can put evetyhing in one table...
	sa_0 = 0;
	mc_0 = 0;

	table_sa_data = [
	["TotalCtrlLoops", sa_total_ctrl_loops, mc_total_ctrl_loops],
	["TotalLoads", sa_total_mc_loads, mc_total_reads],
	["TotalStores",sa_total_mc_stores, mc_total_writes],
	["TotalReturns", sa_total_mc_returns, mc_0],
	["NorthIOCycles", sa_total_north_io_busy_cycles, mc_io_busy_cycles],
	["NorthMaxRxQueueDepth", sa_max_north_rx_queue_depth, mc_max_north_rx_queue_depth],
	["NorthAveRxQueueDepth", sa_ave_north_rx_queue_depth, mc_0],
	["NorthMaxTxQueueDepth", sa_max_north_tx_queue_depth, mc_max_north_tx_queue_depth],
	["NorthAveTxQueueDepth", sa_ave_north_tx_queue_depth, mc_0],
	["SouthIOCycles", sa_total_south_io_busy_cycles, mc_0],
	["SouthMaxRxQueueDepth", sa_max_soutth_rx_queue_depth, mc_0],
	["SouthAveRxQueueDepth", sa_ave_south_rx_queue_depth, mc_0],
	["SouthMaxTxQueueDepth", sa_max_south_tx_queue_depth, mc_0],
	["SouthAveTxQueueDepth", sa_ave_south_tx_queue_depth, mc_0],
	["DramBusyCycles", sa_0, mc_dram_busy_cycles],
	["DramAveReadLat", sa_0, mc_dram_ave_read_lat],
	["DramAveWriteLat", sa_0, mc_dram_ave_write_lat],
	["DramAveTotalLat(cyc)", sa_0, mc_dram_ave_total_lat_cyc],
	["DramAveTotalLat(ns)", sa_0, mc_dram_ave_total_lat_ns],
	["DramReadMinLat", sa_0, mc_dram_read_min_lat],
	["DramReadMaxLat", sa_0, mc_dram_read_max_lat],
	["DramWriteMinLat", sa_0, mc_dram_write_min_lat],
	["DramWriteMaxLat", sa_0, mc_dram_write_max_lat],
	["DramMaxQueueDepth", sa_0, mc_dram_max_queue_depth],
	["DramAveQueueDepth", sa_0, mc_dram_ave_queue_depth],
	["DramTotalBytesRead", sa_0, mc_dram_total_bytes_read],
	["DramTotalBytesWritten", sa_0, mc_dram_total_bytes_written],
	]

	f = open(options.OutFileName, 'a')

	f.write("//SA-MC Stats//////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')
		
	#get the largest title length	
	max_title_length = len('SA-MC Stats')
	current_title_length = 0

	for tup in table_sa_data:
		for item in tup[0:1]:
			current_title_length = len(tup[0])
			if max_title_length < current_title_length:
				max_title_length = current_title_length

	#get the largest data element length
	max_element_length = 0
	current_element_length = 0

	for tup in table_sa_data:
		for item in tup[1:2]:
			current_element_length = len(str(item))
			if max_element_length < current_element_length:
				max_element_length = current_element_length

	max_title_length += 2
	max_element_length += 2
	#print "max title {} max data {}".format(max_title_length, max_element_length)
	
	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}".format("Stat SA/MC",'SA', 'MC', title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in table_sa_data:
		f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write('\n')
	f.close
	return


def print_mem_system_stats(options):

	sa_data = ConfigParser.ConfigParser()
	sa_data.read(options.InFileName)
	
	First_access_lat = sa_data.getint('MemSystem', 'FirstAccessLat(Fetch)')
	Total_fetches = sa_data.getint('MemSystem', 'TotalFetches')
	Fetches_L1 = sa_data.getint('MemSystem', 'FetchesL1')
	Fetches_L2 = sa_data.getint('MemSystem', 'FetchesL2')
	Fetches_L3 = sa_data.getint('MemSystem', 'FetchesL3')
	Fetches_memory = sa_data.getint('MemSystem', 'FetchesMemory')
	Total_loads = sa_data.getint('MemSystem', 'TotalLoads')
	Loads_L1 = sa_data.getint('MemSystem', 'LoadsL1')
	Loads_L2 = sa_data.getint('MemSystem', 'LoadsL2')
	Loads_L3 = sa_data.getint('MemSystem', 'LoadsL3')
	Loads_memory = sa_data.getint('MemSystem', 'LoadsMemory')
	Loads_getfwd = sa_data.getint('MemSystem', 'LoadsGetFwd')
	Total_store = sa_data.getint('MemSystem', 'TotalStore')
	Stores_L1 = sa_data.getint('MemSystem', 'StoresL1')
	Stores_L2 = sa_data.getint('MemSystem', 'StoresL2')
	Stores_L3 = sa_data.getint('MemSystem', 'StoresL3')
	Stores_memory = sa_data.getint('MemSystem', 'StoresMemory')
	Stores_getxfwd = sa_data.getint('MemSystem', 'StoresGetxFwd')
	Stores_upgrade = sa_data.getint('MemSystem', 'StoresUpgrade')
		
	table_mem_system_data = [
	["FirstAccessLat(Fetch)", First_access_lat],
	["TotalFetches", Total_fetches],
	["FetchesL1", Fetches_L1],
	["FetchesL2", Fetches_L2],
	["FetchesL3", Fetches_L3],
	["FetchesMemory", Fetches_memory],
	["TotalLoads", Total_loads],
	["LoadsL1", Loads_L1],
	["LoadsL2", Loads_L2],
	["LoadsL3", Loads_L3],
	["LoadsMemory", Loads_memory],
	["LoadsGetFwd", Loads_getfwd],
	["TotalStore", Total_store],
	["StoresL1", Stores_L1],
	["StoresL2", Stores_L2],
	["StoresL3", Stores_L3],
	["StoresMemory", Stores_memory],
	["StoresGetxFwd", Stores_getxfwd],
	["StoresUpgrade", Stores_upgrade],
	]

	f = open(options.OutFileName, 'a')

	f.write("//Mem-System Stats/////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')

	#get the largest title length	
	max_title_length = len('Stat Mem System')
	current_title_length = 0

	for tup in table_mem_system_data:
		for item in tup[0:1]:
			current_title_length = len(tup[0])
			if max_title_length < current_title_length:
				max_title_length = current_title_length

	#get the largest data element length
	max_element_length = 0
	current_element_length = 0

	for tup in table_mem_system_data:
		for item in tup[1:2]:
			current_element_length = len(str(item))
			if max_element_length < current_element_length:
				max_element_length = current_element_length

	max_title_length += 2
	max_element_length += 2
	#print "max title {} max data {}".format(max_title_length, max_element_length)
	
	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("Stat Mem System",'Stats', title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in table_mem_system_data:
		f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write('\n')
	f.close

	return

def print_general_stats(options):

	general_data = ConfigParser.ConfigParser()
	general_data.read(options.InFileName)
		
	bench_args = general_data.get('General', 'Benchmark')
	day_time = general_data.get('General', 'Day&Time')
	total_cycles = general_data.getint('General', 'TotalCycles')
	run_time_cpu = general_data.getfloat('General', 'SimulationRunTimeSeconds(cpu)')
	run_time_wall = general_data.getfloat('General', 'SimulationRunTimeSeconds(wall)')
	simulated_cycles_per_sec = general_data.getfloat('General', 'SimulatedCyclesPerSec')
	
	table_general_data = [
	["BenchmarkArgs", bench_args],
	["DateTime", day_time],
	["TotalCycles", total_cycles],
	["CPURunTime(sec)", run_time_cpu],
	["WallRunTime(sec)", run_time_wall],
	["SimulatedCyclesPerSec", simulated_cycles_per_sec],
	]
	
	f = open(options.OutFileName, 'w')

	f.write("//General Stats////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')

	#get the largest title length	
	max_title_length = len('General Stats')
	current_title_length = 0

	for tup in table_general_data:
		for item in tup[0:1]:
			current_title_length = len(tup[0])
			if max_title_length < current_title_length:
				max_title_length = current_title_length

	#get the largest data element length
	max_element_length = 0
	current_element_length = 0

	for tup in table_general_data:
		for item in tup[1:2]:
			current_element_length = len(str(item))
			if max_element_length < current_element_length:
				max_element_length = current_element_length

	max_title_length += 2
	max_element_length += 2
	#print "max title {} max data {}".format(max_title_length, max_element_length)
	
	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("General Stats",'Stats', title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in table_general_data:
		f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write('\n')
	f.close
		
	return

def print_cpu_stats(options):

	cpu_data = ConfigParser.ConfigParser()
	cpu_data.read(options.InFileName)
	
	###core P0###
	if int(options.NumCores) == 1 or int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		core_0_NumSyscalls = cpu_data.getint('Core_0', 'NumSyscalls')
		core_0_ROBStalls = cpu_data.getint('Core_0', 'ROBStalls')
		core_0_ROBStallLoad = cpu_data.getint('Core_0', 'ROBStallLoad')
		core_0_ROBStallStore = cpu_data.getint('Core_0', 'ROBStallStore')
		core_0_ROBStallOther = cpu_data.getint('Core_0', 'ROBStallOther')
		core_0_FirstFetchCycle = cpu_data.getint('Core_0', 'FirstFetchCycle')
		core_0_LastCommitCycle = cpu_data.getint('Core_0', 'FirstFetchCycle')
		core_0_FetchStall = cpu_data.getint('Core_0', 'FetchStall')
		core_0_RunTime = cpu_data.getint('Core_0', 'RunTime')
		core_0_IdleTime = cpu_data.getint('Core_0', 'IdleTime')
		core_0_SystemTime = cpu_data.getint('Core_0', 'SystemTime')
		core_0_StallTime = cpu_data.getint('Core_0', 'StallTime')
		core_0_BusyTime = cpu_data.getint('Core_0', 'BusyTime')
		core_0_IdlePct = cpu_data.getfloat('Core_0', 'IdlePct')
		core_0_RunPct = cpu_data.getfloat('Core_0', 'RunPct')
		core_0_SystemPct = cpu_data.getfloat('Core_0', 'SystemPct')
		core_0_StallPct = cpu_data.getfloat('Core_0', 'StallPct')
		core_0_BusyPct = cpu_data.getfloat('Core_0', 'BusyPct')
		core_0_StallfetchPct = cpu_data.getfloat('Core_0', 'StallfetchPct')
		core_0_StallLoadPct = cpu_data.getfloat('Core_0', 'StallLoadPct')
		core_0_StallStorePct = cpu_data.getfloat('Core_0', 'StallStorePct')
		core_0_StallOtherPct = cpu_data.getfloat('Core_0', 'StallOtherPct')

	###core P1###
	if int(options.NumCores) == 2 or int(options.NumCores) == 3 or int(options.NumCores) == 4:
		core_1_NumSyscalls = cpu_data.getint('Core_1', 'NumSyscalls')
		core_1_ROBStalls = cpu_data.getint('Core_1', 'ROBStalls')
		core_1_ROBStallLoad = cpu_data.getint('Core_1', 'ROBStallLoad')
		core_1_ROBStallStore = cpu_data.getint('Core_1', 'ROBStallStore')
		core_1_ROBStallOther = cpu_data.getint('Core_1', 'ROBStallOther')
		core_1_FirstFetchCycle = cpu_data.getint('Core_1', 'FirstFetchCycle')
		core_1_LastCommitCycle = cpu_data.getint('Core_1', 'FirstFetchCycle')
		core_1_FetchStall = cpu_data.getint('Core_1', 'FetchStall')
		core_1_RunTime = cpu_data.getint('Core_1', 'RunTime')
		core_1_IdleTime = cpu_data.getint('Core_1', 'IdleTime')
		core_1_SystemTime = cpu_data.getint('Core_1', 'SystemTime')
		core_1_StallTime = cpu_data.getint('Core_1', 'StallTime')
		core_1_BusyTime = cpu_data.getint('Core_1', 'BusyTime')
		core_1_IdlePct = cpu_data.getfloat('Core_1', 'IdlePct')
		core_1_RunPct = cpu_data.getfloat('Core_1', 'RunPct')
		core_1_SystemPct = cpu_data.getfloat('Core_1', 'SystemPct')
		core_1_StallPct = cpu_data.getfloat('Core_1', 'StallPct')
		core_1_BusyPct = cpu_data.getfloat('Core_1', 'BusyPct')
		core_1_StallfetchPct = cpu_data.getfloat('Core_1', 'StallfetchPct')
		core_1_StallLoadPct = cpu_data.getfloat('Core_1', 'StallLoadPct')
		core_1_StallStorePct = cpu_data.getfloat('Core_1', 'StallStorePct')
		core_1_StallOtherPct = cpu_data.getfloat('Core_1', 'StallOtherPct')

	###core P2###
	if int(options.NumCores) == 3 or int(options.NumCores) == 4:
		core_2_NumSyscalls = cpu_data.getint('Core_2', 'NumSyscalls')
		core_2_ROBStalls = cpu_data.getint('Core_2', 'ROBStalls')
		core_2_ROBStallLoad = cpu_data.getint('Core_2', 'ROBStallLoad')
		core_2_ROBStallStore = cpu_data.getint('Core_2', 'ROBStallStore')
		core_2_ROBStallOther = cpu_data.getint('Core_2', 'ROBStallOther')
		core_2_FirstFetchCycle = cpu_data.getint('Core_2', 'FirstFetchCycle')
		core_2_LastCommitCycle = cpu_data.getint('Core_2', 'FirstFetchCycle')
		core_2_FetchStall = cpu_data.getint('Core_2', 'FetchStall')
		core_2_RunTime = cpu_data.getint('Core_2', 'RunTime')
		core_2_IdleTime = cpu_data.getint('Core_2', 'IdleTime')
		core_2_SystemTime = cpu_data.getint('Core_2', 'SystemTime')
		core_2_StallTime = cpu_data.getint('Core_2', 'StallTime')
		core_2_BusyTime = cpu_data.getint('Core_2', 'BusyTime')
		core_2_IdlePct = cpu_data.getfloat('Core_2', 'IdlePct')
		core_2_RunPct = cpu_data.getfloat('Core_2', 'RunPct')
		core_2_SystemPct = cpu_data.getfloat('Core_2', 'SystemPct')
		core_2_StallPct = cpu_data.getfloat('Core_2', 'StallPct')
		core_2_BusyPct = cpu_data.getfloat('Core_2', 'BusyPct')
		core_2_StallfetchPct = cpu_data.getfloat('Core_2', 'StallfetchPct')
		core_2_StallLoadPct = cpu_data.getfloat('Core_2', 'StallLoadPct')
		core_2_StallStorePct = cpu_data.getfloat('Core_2', 'StallStorePct')
		core_2_StallOtherPct = cpu_data.getfloat('Core_2', 'StallOtherPct')

	###core P3###
	if int(options.NumCores) == 4:
		core_3_NumSyscalls = cpu_data.getint('Core_3', 'NumSyscalls')
		core_3_ROBStalls = cpu_data.getint('Core_3', 'ROBStalls')
		core_3_ROBStallLoad = cpu_data.getint('Core_3', 'ROBStallLoad')
		core_3_ROBStallStore = cpu_data.getint('Core_3', 'ROBStallStore')
		core_3_ROBStallOther = cpu_data.getint('Core_3', 'ROBStallOther')
		core_3_FirstFetchCycle = cpu_data.getint('Core_3', 'FirstFetchCycle')
		core_3_LastCommitCycle = cpu_data.getint('Core_3', 'FirstFetchCycle')
		core_3_FetchStall = cpu_data.getint('Core_3', 'FetchStall')
		core_3_RunTime = cpu_data.getint('Core_3', 'RunTime')
		core_3_IdleTime = cpu_data.getint('Core_3', 'IdleTime')
		core_3_SystemTime = cpu_data.getint('Core_3', 'SystemTime')
		core_3_StallTime = cpu_data.getint('Core_3', 'StallTime')
		core_3_BusyTime = cpu_data.getint('Core_3', 'BusyTime')
		core_3_IdlePct = cpu_data.getfloat('Core_3', 'IdlePct')
		core_3_RunPct = cpu_data.getfloat('Core_3', 'RunPct')
		core_3_SystemPct = cpu_data.getfloat('Core_3', 'SystemPct')
		core_3_StallPct = cpu_data.getfloat('Core_3', 'StallPct')
		core_3_BusyPct = cpu_data.getfloat('Core_3', 'BusyPct')
		core_3_StallfetchPct = cpu_data.getfloat('Core_3', 'StallfetchPct')
		core_3_StallLoadPct = cpu_data.getfloat('Core_3', 'StallLoadPct')
		core_3_StallStorePct = cpu_data.getfloat('Core_3', 'StallStorePct')
		core_3_StallOtherPct = cpu_data.getfloat('Core_3', 'StallOtherPct')


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


	table_P4 = [
		["NumSyscalls", core_0_NumSyscalls, core_1_NumSyscalls, core_2_NumSyscalls, core_3_NumSyscalls],
		["ROBStalls", core_0_ROBStalls, core_1_ROBStalls, core_2_ROBStalls, core_3_ROBStalls], 
		["ROBStallLoad", core_0_ROBStallLoad, core_1_ROBStallLoad, core_2_ROBStallLoad, core_3_ROBStallLoad],
		["ROBStallStore", core_0_ROBStallStore, core_1_ROBStallStore, core_2_ROBStallStore, core_3_ROBStallStore],
		["ROBStallOther", core_0_ROBStallOther, core_1_ROBStallOther, core_2_ROBStallOther, core_3_ROBStallOther],
		["FirstFetchCycle", core_0_FirstFetchCycle, core_1_FirstFetchCycle, core_2_FirstFetchCycle, core_3_FirstFetchCycle],
		["LastCommitCycle", core_0_LastCommitCycle, core_1_LastCommitCycle, core_2_LastCommitCycle, core_3_LastCommitCycle],
		["FetchStall", core_0_FetchStall, core_1_FetchStall, core_2_FetchStall, core_3_FetchStall],
		["RunTime", core_0_RunTime, core_1_RunTime, core_2_RunTime, core_3_RunTime],
		["IdleTime", core_0_IdleTime, core_1_IdleTime, core_2_IdleTime, core_3_IdleTime],
		["SystemTime(SysCalls)", core_0_SystemTime, core_1_SystemTime, core_2_SystemTime, core_3_SystemTime],
		["StallTime", core_0_StallTime, core_1_StallTime, core_2_StallTime, core_3_StallTime],
		["BusyTime", core_0_BusyTime, core_1_BusyTime, core_2_BusyTime, core_3_BusyTime],
		["IdlePct", core_0_IdlePct, core_1_IdlePct, core_2_IdlePct, core_3_IdlePct],
		["RunPct", core_0_RunPct, core_1_RunPct, core_2_RunPct, core_3_RunPct],
		["SystemPct", core_0_SystemPct, core_1_SystemPct, core_2_SystemPct, core_3_SystemPct],
		["StallPct", core_0_StallPct, core_1_StallPct, core_2_StallPct, core_3_StallPct],
		["BusyPct", core_0_BusyPct, core_1_BusyPct, core_2_BusyPct, core_3_BusyPct],
		["StallfetchPct", core_0_StallfetchPct, core_1_StallfetchPct, core_2_StallfetchPct, core_3_StallfetchPct],
		["StallLoadPct", core_0_StallLoadPct, core_1_StallLoadPct, core_2_StallLoadPct, core_3_StallLoadPct],	
		["StallStorePct", core_0_StallStorePct, core_1_StallStorePct, core_2_StallStorePct, core_3_StallStorePct],
		["StallOtherPct", core_0_StallOtherPct, core_1_StallOtherPct, core_2_StallOtherPct, core_3_StallOtherPct]
		]

	
	f = open(options.OutFileName, 'a')

	f.write("//CPU Stats////////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')


	###print stats for 1 core (P0)###
	if int(options.NumCores) == 1:
	
		max_title_length = len('CPU Stats Core_0')
		current_title_length = 0

		for tup in table_P0:
			for item in tup[0:1]:
				current_title_length = len(tup[0])
				if max_title_length < current_title_length:
					max_title_length = current_title_length

		#get the largest data element length
		max_element_length = 0
		current_element_length = 0

		for tup in table_P0:
			for item in tup[1:5]:
				current_element_length = len(str(item))
				if max_element_length < current_element_length:
					max_element_length = current_element_length

		max_title_length += 2
		max_element_length += 2
		#print "max title {} max data {}".format(max_title_length, max_element_length)

		title_bar = '-' * (max_title_length - 1)
		data_bar = '-' * (max_element_length - 1)

		#print the title and bars
		#print the title and barss
		f.write("{:<{title_width}}{:>{data_width}}".format("CPU Stats Core_0",'Stats', title_width=max_title_length, data_width=max_element_length) + '\n')
		f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

		#print the table's data
		for tup in table_P0:
			f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')


	###print stats combined 4 cores###
	if int(options.NumCores) == 4:
		#get the largest title length	
		max_title_length = len('CPU Stats All Cores')
		current_title_length = 0

		for tup in table_P4:
			for item in tup[0:1]:
				current_title_length = len(tup[0])
				if max_title_length < current_title_length:
					max_title_length = current_title_length

		#get the largest data element length
		max_element_length = 0
		current_element_length = 0

		for tup in table_P4:
			for item in tup[1:5]:
				current_element_length = len(str(item))
				if max_element_length < current_element_length:
					max_element_length = current_element_length

		max_title_length += 2
		max_element_length += 2
		#print "max title {} max data {}".format(max_title_length, max_element_length)
	
		title_bar = '-' * (max_title_length - 1)
		data_bar = '-' * (max_element_length - 1)

		#print the title and bars
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format("CPU Stats All Cores",'Core_0', 'Core_1', 'Core_2', 'Core_3', title_width=max_title_length, data_width=max_element_length) + '\n')
		f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

		#print the table's data
		for tup in table_P4:
			f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')

		f.write('\n')

	f.close
	
	return


parser = OptionParser()
parser.usage = "%prog -c numcores -i inputfile -o outputfile"
parser.add_option("-c", "--numcores", dest="NumCores", default="", help="Specifiy the number of cores.")
parser.add_option("-i", "--infile", dest="InFileName", default="", help="Specifiy the stats file and path to parse.")
parser.add_option("-o", "--outfile", dest="OutFileName", default="sim_stats.txt", help="Specifiy the outputfile name and path.")
(options, args) = parser.parse_args()

if not options.NumCores:
	parser.print_usage()
	exit(0)

if not options.InFileName:
	parser.print_usage()
	exit(0)


print_general_stats(options)
print_cpu_stats(options)
print_mem_system_stats(options)
print_cache_stats(options)
print_switch_stats(options)
print_samc_stats(options)

