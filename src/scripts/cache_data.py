#!/usr/bin/env python
from tabulate import tabulate
import ConfigParser

sim_data = ConfigParser.RawConfigParser()
sim_data.read('/home/stardica/Desktop/m2s-cgm/Release/m2s-cgm.out')


###core P0###
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
["TotalWriteBacks", l1_i_0_total_write_backs, l1_d_0_total_write_backs, l2_0_total_write_backs, l3_0_total_write_backs]
]

###core P1###

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
["TotalWriteBacks", l1_i_1_total_write_backs, l1_d_1_total_write_backs, l2_1_total_write_backs, l3_1_total_write_backs]
]


###core P2###

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
["TotalWriteBacks", l1_i_2_total_write_backs, l1_d_2_total_write_backs, l2_2_total_write_backs, l3_2_total_write_backs]
]


###core P3###

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
["TotalWriteBacks", l1_i_3_total_write_backs, l1_d_3_total_write_backs, l2_3_total_write_backs, l3_3_total_write_backs]
]


print tabulate(table_P0, headers=["Stat P0", "P0_I", "P0_D", "P0_L2", "P0_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f")
print('\n')
print tabulate(table_P1, headers=["Stat P1", "P1_I", "P1_D", "P1_L2", "P1_L3"], tablefmt="simple",  numalign="right", floatfmt="16.4f")
print('\n')
print tabulate(table_P2, headers=["Stat P2", "P2_I", "P2_D", "P2_L2", "P2_L3"], tablefmt="simple",  numalign="right", floatfmt="16.4f")
print('\n')
print tabulate(table_P3, headers=["Stat P3", "P3_I", "P3_D", "P3_L2", "P3_L3"], tablefmt="simple",  numalign="right", floatfmt="16.4f")        

f = open('Cache_stats.out', 'w')
f.write(tabulate(table_P0, headers=["Stat P0", "P0_I", "P0_D", "P0_L2", "P0_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
f.write("\n\n")
f.write(tabulate(table_P1, headers=["Stat P1", "P1_I", "P1_D", "P1_L2", "P1_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
f.write("\n\n")
f.write(tabulate(table_P2, headers=["Stat P2", "P2_I", "P2_D", "P2_L2", "P2_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
f.write("\n\n")
f.write(tabulate(table_P3, headers=["Stat P3", "P3_I", "P3_D", "P3_L2", "P3_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
f.close()
