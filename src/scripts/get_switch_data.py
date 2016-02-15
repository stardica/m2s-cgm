#!/usr/bin/env python
from tabulate import tabulate
import ConfigParser

sim_data = ConfigParser.RawConfigParser()
sim_data.read('/home/stardica/Desktop/m2s-cgm/src/scripts/m2s-cgm.out')

###Switch S0###
S0_total_cache_ctrl_loops = sim_data.getint("L1_I_Cache_0", "TotalCacheCtrlLoops")
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



print tabulate(table_P0, headers=["Stat P0", "P0_I", "P0_D", "P0_L2", "P0_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f")
print('\n')
print tabulate(table_P1, headers=["Stat P1", "P1_I", "P1_D", "P1_L2", "P1_L3"], tablefmt="simple",  numalign="right", floatfmt="16.4f")
print('\n')
print tabulate(table_P2, headers=["Stat P2", "P2_I", "P2_D", "P2_L2", "P2_L3"], tablefmt="simple",  numalign="right", floatfmt="16.4f")
print('\n')
print tabulate(table_P3, headers=["Stat P3", "P3_I", "P3_D", "P3_L2", "P3_L3"], tablefmt="simple",  numalign="right", floatfmt="16.4f")        

#f = open('Cache_stats.out', 'w')
#f.write(tabulate(table_P0, headers=["Stat P0", "P0_I", "P0_D", "P0_L2", "P0_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
#f.write("\n\n")
#f.write(tabulate(table_P1, headers=["Stat P1", "P1_I", "P1_D", "P1_L2", "P1_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
#f.write("\n\n")
#f.write(tabulate(table_P2, headers=["Stat P2", "P2_I", "P2_D", "P2_L2", "P2_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
#f.write("\n\n")
#f.write(tabulate(table_P3, headers=["Stat P3", "P3_I", "P3_D", "P3_L2", "P3_L3"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
#f.close()
