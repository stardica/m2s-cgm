#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser

cache_combined = 1



def print_cache_stats(options):

	cache_data = ConfigParser.RawConfigParser()
	cache_data.optionxform = str 
	cache_data.read(options.InFileName)

	#pull stats
	if options.PrintSection == 'FullRunStats':
		cache_stats = dict(cache_data.items('FullRunStats'))
	elif options.PrintSection == 'ParallelStats':
		cache_stats = dict(cache_data.items('ParallelStats'))
	else:
		print "print_cache_stats(): invalid section"
		exit(0)


	for key, value in cache_stats.items(): # get the (key, value) tuples one at a time
		try:
			cache_stats[key] = int(value)
		except ValueError:
			cache_stats[key] = float(value)
		
	
	l1_i_Occupancy = cache_stats['l1_i_0_Occupancy'] + cache_stats['l1_i_1_Occupancy'] + cache_stats['l1_i_2_Occupancy'] + cache_stats['l1_i_3_Occupancy']
	l1_i_OccupancyPct = cache_stats['l1_i_0_OccupancyPct'] + cache_stats['l1_i_1_OccupancyPct'] + cache_stats['l1_i_2_OccupancyPct'] + cache_stats['l1_i_3_OccupancyPct']
	l1_i_TotalAdvances = cache_stats['l1_i_0_TotalAdvances'] + cache_stats['l1_i_1_TotalAdvances'] + cache_stats['l1_i_2_TotalAdvances'] + cache_stats['l1_i_3_TotalAdvances']
	l1_i_TotalAccesses = cache_stats['l1_i_0_TotalAccesses'] + cache_stats['l1_i_1_TotalAccesses'] + cache_stats['l1_i_2_TotalAccesses'] + cache_stats['l1_i_3_TotalAccesses']
	l1_i_TotalHits = cache_stats['l1_i_0_TotalHits'] + cache_stats['l1_i_1_TotalHits'] + cache_stats['l1_i_2_TotalHits'] + cache_stats['l1_i_3_TotalHits']
	l1_i_TotalMisses = cache_stats['l1_i_0_TotalMisses'] + cache_stats['l1_i_1_TotalMisses'] + cache_stats['l1_i_2_TotalMisses'] + cache_stats['l1_i_3_TotalMisses']
	l1_i_MissRate = cache_stats['l1_i_0_MissRate'] + cache_stats['l1_i_1_MissRate'] + cache_stats['l1_i_2_MissRate'] + cache_stats['l1_i_3_MissRate']
	l1_i_TotalReads = cache_stats['l1_i_0_TotalReads'] + cache_stats['l1_i_1_TotalReads'] + cache_stats['l1_i_2_TotalReads'] + cache_stats['l1_i_3_TotalReads']
	l1_i_TotalReadMisses = cache_stats['l1_i_0_TotalReadMisses'] + cache_stats['l1_i_1_TotalReadMisses'] + cache_stats['l1_i_2_TotalReadMisses'] + cache_stats['l1_i_3_TotalReadMisses']
	l1_i_ReadMissRate = cache_stats['l1_i_0_ReadMissRate'] + cache_stats['l1_i_1_ReadMissRate'] + cache_stats['l1_i_2_ReadMissRate'] + cache_stats['l1_i_3_ReadMissRate']
	l1_i_TotalWrites = cache_stats['l1_i_0_TotalWrites'] + cache_stats['l1_i_1_TotalWrites'] + cache_stats['l1_i_2_TotalWrites'] + cache_stats['l1_i_3_TotalWrites']
	l1_i_TotalWriteMisses = cache_stats['l1_i_0_TotalWriteMisses'] + cache_stats['l1_i_1_TotalWriteMisses'] + cache_stats['l1_i_2_TotalWriteMisses'] + cache_stats['l1_i_3_TotalWriteMisses']
	l1_i_WriteMissRate = cache_stats['l1_i_0_WriteMissRate'] + cache_stats['l1_i_1_WriteMissRate'] + cache_stats['l1_i_2_WriteMissRate'] + cache_stats['l1_i_3_WriteMissRate']
	l1_i_TotalGets = cache_stats['l1_i_0_TotalGets'] +  cache_stats['l1_i_1_TotalGets'] +  cache_stats['l1_i_2_TotalGets'] + cache_stats['l1_i_3_TotalGets']
	l1_i_TotalGet = cache_stats['l1_i_0_TotalGet'] +  cache_stats['l1_i_1_TotalGet'] +  cache_stats['l1_i_2_TotalGet'] + cache_stats['l1_i_3_TotalGet']
	l1_i_TotalGetx = cache_stats['l1_i_0_TotalGetx'] + cache_stats['l1_i_1_TotalGetx'] + cache_stats['l1_i_2_TotalGetx'] + cache_stats['l1_i_3_TotalGetx']
	l1_i_GetsMissRate = cache_stats['l1_i_0_GetsMissRate'] + cache_stats['l1_i_1_GetsMissRate'] + cache_stats['l1_i_2_GetsMissRate'] + cache_stats['l1_i_3_GetsMissRate']
	l1_i_GetMissRate = cache_stats['l1_i_0_GetMissRate'] + cache_stats['l1_i_1_GetMissRate'] + cache_stats['l1_i_2_GetMissRate'] + cache_stats['l1_i_3_GetMissRate']
	l1_i_GetxMissRate = cache_stats['l1_i_0_GetxMissRate'] + cache_stats['l1_i_1_GetxMissRate'] + cache_stats['l1_i_2_GetxMissRate'] + cache_stats['l1_i_3_GetxMissRate']
	l1_i_TotalUpgrades = cache_stats['l1_i_0_TotalUpgrades'] + cache_stats['l1_i_1_TotalUpgrades'] + cache_stats['l1_i_2_TotalUpgrades'] + cache_stats['l1_i_3_TotalUpgrades']
	l1_i_UpgradeMissRate = cache_stats['l1_i_0_UpgradeMissRate'] + cache_stats['l1_i_1_UpgradeMissRate'] + cache_stats['l1_i_2_UpgradeMissRate'] + cache_stats['l1_i_3_UpgradeMissRate']
	l1_i_TotalWriteBacks = cache_stats['l1_i_0_TotalWriteBacks'] + cache_stats['l1_i_1_TotalWriteBacks'] + cache_stats['l1_i_2_TotalWriteBacks'] + cache_stats['l1_i_3_TotalWriteBacks']
	l1_i_CacheUtilization = cache_stats['l1_i_0_CacheUtilization'] + cache_stats['l1_i_1_CacheUtilization'] + cache_stats['l1_i_2_CacheUtilization'] + cache_stats['l1_i_3_CacheUtilization']

	l1_d_Occupancy = cache_stats['l1_d_0_Occupancy'] + cache_stats['l1_d_1_Occupancy'] + cache_stats['l1_d_2_Occupancy'] + cache_stats['l1_d_3_Occupancy']
	l1_d_OccupancyPct = cache_stats['l1_d_0_OccupancyPct'] + cache_stats['l1_d_1_OccupancyPct'] + cache_stats['l1_d_2_OccupancyPct'] + cache_stats['l1_d_3_OccupancyPct']
	l1_d_TotalAdvances = cache_stats['l1_d_0_TotalAdvances'] + cache_stats['l1_d_1_TotalAdvances'] + cache_stats['l1_d_2_TotalAdvances'] + cache_stats['l1_d_3_TotalAdvances']
	l1_d_TotalAccesses = cache_stats['l1_d_0_TotalAccesses'] + cache_stats['l1_d_1_TotalAccesses'] + cache_stats['l1_d_2_TotalAccesses'] + cache_stats['l1_d_3_TotalAccesses']
	l1_d_TotalHits = cache_stats['l1_d_0_TotalHits'] + cache_stats['l1_d_1_TotalHits'] + cache_stats['l1_d_2_TotalHits'] + cache_stats['l1_d_3_TotalHits']
	l1_d_TotalMisses = cache_stats['l1_d_0_TotalMisses'] + cache_stats['l1_d_1_TotalMisses'] + cache_stats['l1_d_2_TotalMisses'] + cache_stats['l1_d_3_TotalMisses']
	l1_d_MissRate = cache_stats['l1_d_0_MissRate'] + cache_stats['l1_d_1_MissRate'] + cache_stats['l1_d_2_MissRate'] + cache_stats['l1_d_3_MissRate']
	l1_d_TotalReads = cache_stats['l1_d_0_TotalReads'] + cache_stats['l1_d_1_TotalReads'] + cache_stats['l1_d_2_TotalReads'] + cache_stats['l1_d_3_TotalReads']
	l1_d_TotalReadMisses = cache_stats['l1_d_0_TotalReadMisses'] + cache_stats['l1_d_1_TotalReadMisses'] + cache_stats['l1_d_2_TotalReadMisses'] + cache_stats['l1_d_3_TotalReadMisses']
	l1_d_ReadMissRate = cache_stats['l1_d_0_ReadMissRate'] + cache_stats['l1_d_1_ReadMissRate'] + cache_stats['l1_d_2_ReadMissRate'] + cache_stats['l1_d_3_ReadMissRate']
	l1_d_TotalWrites = cache_stats['l1_d_0_TotalWrites'] + cache_stats['l1_d_1_TotalWrites'] + cache_stats['l1_d_2_TotalWrites'] + cache_stats['l1_d_3_TotalWrites']
	l1_d_TotalWriteMisses = cache_stats['l1_d_0_TotalWriteMisses'] + cache_stats['l1_d_1_TotalWriteMisses'] + cache_stats['l1_d_2_TotalWriteMisses'] + cache_stats['l1_d_3_TotalWriteMisses']
	l1_d_WriteMissRate = cache_stats['l1_d_0_WriteMissRate'] + cache_stats['l1_d_1_WriteMissRate'] + cache_stats['l1_d_2_WriteMissRate'] + cache_stats['l1_d_3_WriteMissRate']
	l1_d_TotalGets = cache_stats['l1_d_0_TotalGets'] +  cache_stats['l1_d_1_TotalGets'] +  cache_stats['l1_d_2_TotalGets'] + cache_stats['l1_d_3_TotalGets']
	l1_d_TotalGet = cache_stats['l1_d_0_TotalGet'] +  cache_stats['l1_d_1_TotalGet'] +  cache_stats['l1_d_2_TotalGet'] + cache_stats['l1_d_3_TotalGet']
	l1_d_TotalGetx = cache_stats['l1_d_0_TotalGetx'] + cache_stats['l1_d_1_TotalGetx'] + cache_stats['l1_d_2_TotalGetx'] + cache_stats['l1_d_3_TotalGetx']
	l1_d_GetsMissRate = cache_stats['l1_d_0_GetsMissRate'] + cache_stats['l1_d_1_GetsMissRate'] + cache_stats['l1_d_2_GetsMissRate'] + cache_stats['l1_d_3_GetsMissRate']
	l1_d_GetMissRate = cache_stats['l1_d_0_GetMissRate'] + cache_stats['l1_d_1_GetMissRate'] + cache_stats['l1_d_2_GetMissRate'] + cache_stats['l1_d_3_GetMissRate']
	l1_d_GetxMissRate = cache_stats['l1_d_0_GetxMissRate'] + cache_stats['l1_d_1_GetxMissRate'] + cache_stats['l1_d_2_GetxMissRate'] + cache_stats['l1_d_3_GetxMissRate']
	l1_d_TotalUpgrades = cache_stats['l1_d_0_TotalUpgrades'] + cache_stats['l1_d_1_TotalUpgrades'] + cache_stats['l1_d_2_TotalUpgrades'] + cache_stats['l1_d_3_TotalUpgrades']
	l1_d_UpgradeMissRate = cache_stats['l1_d_0_UpgradeMissRate'] + cache_stats['l1_d_1_UpgradeMissRate'] + cache_stats['l1_d_2_UpgradeMissRate'] + cache_stats['l1_d_3_UpgradeMissRate']
	l1_d_TotalWriteBacks = cache_stats['l1_d_0_TotalWriteBacks'] + cache_stats['l1_d_1_TotalWriteBacks'] + cache_stats['l1_d_2_TotalWriteBacks'] + cache_stats['l1_d_3_TotalWriteBacks']
	l1_d_CacheUtilization = cache_stats['l1_d_0_CacheUtilization'] + cache_stats['l1_d_1_CacheUtilization'] + cache_stats['l1_d_2_CacheUtilization'] + cache_stats['l1_d_3_CacheUtilization']

	l2_Occupancy = cache_stats['l2_0_Occupancy'] + cache_stats['l2_1_Occupancy'] + cache_stats['l2_2_Occupancy'] + cache_stats['l2_3_Occupancy']
	l2_OccupancyPct = cache_stats['l2_0_OccupancyPct'] + cache_stats['l2_1_OccupancyPct'] + cache_stats['l2_2_OccupancyPct'] + cache_stats['l2_3_OccupancyPct']
	l2_TotalAdvances = cache_stats['l2_0_TotalAdvances'] + cache_stats['l2_1_TotalAdvances'] + cache_stats['l2_2_TotalAdvances'] + cache_stats['l2_3_TotalAdvances']
	l2_TotalAccesses = cache_stats['l2_0_TotalAccesses'] + cache_stats['l2_1_TotalAccesses'] + cache_stats['l2_2_TotalAccesses'] + cache_stats['l2_3_TotalAccesses']
	l2_TotalHits = cache_stats['l2_0_TotalHits'] + cache_stats['l2_1_TotalHits'] + cache_stats['l2_2_TotalHits'] + cache_stats['l2_3_TotalHits']
	l2_TotalMisses = cache_stats['l2_0_TotalMisses'] + cache_stats['l2_1_TotalMisses'] + cache_stats['l2_2_TotalMisses'] + cache_stats['l2_3_TotalMisses']
	l2_MissRate = cache_stats['l2_0_MissRate'] + cache_stats['l2_1_MissRate'] + cache_stats['l2_2_MissRate'] + cache_stats['l2_3_MissRate']
	l2_TotalReads = cache_stats['l2_0_TotalReads'] + cache_stats['l2_1_TotalReads'] + cache_stats['l2_2_TotalReads'] + cache_stats['l2_3_TotalReads']
	l2_TotalReadMisses = cache_stats['l2_0_TotalReadMisses'] + cache_stats['l2_1_TotalReadMisses'] + cache_stats['l2_2_TotalReadMisses'] + cache_stats['l2_3_TotalReadMisses']
	l2_ReadMissRate = cache_stats['l2_0_ReadMissRate'] + cache_stats['l2_1_ReadMissRate'] + cache_stats['l2_2_ReadMissRate'] + cache_stats['l2_3_ReadMissRate']
	l2_TotalWrites = cache_stats['l2_0_TotalWrites'] + cache_stats['l2_1_TotalWrites'] + cache_stats['l2_2_TotalWrites'] + cache_stats['l2_3_TotalWrites']
	l2_TotalWriteMisses = cache_stats['l2_0_TotalWriteMisses'] + cache_stats['l2_1_TotalWriteMisses'] + cache_stats['l2_2_TotalWriteMisses'] + cache_stats['l2_3_TotalWriteMisses']
	l2_WriteMissRate = cache_stats['l2_0_WriteMissRate'] + cache_stats['l2_1_WriteMissRate'] + cache_stats['l2_2_WriteMissRate'] + cache_stats['l2_3_WriteMissRate']
	l2_TotalGets = cache_stats['l2_0_TotalGets'] +  cache_stats['l2_1_TotalGets'] +  cache_stats['l2_2_TotalGets'] + cache_stats['l2_3_TotalGets']
	l2_TotalGet = cache_stats['l2_0_TotalGet'] +  cache_stats['l2_1_TotalGet'] +  cache_stats['l2_2_TotalGet'] + cache_stats['l2_3_TotalGet']
	l2_TotalGetx = cache_stats['l2_0_TotalGetx'] + cache_stats['l2_1_TotalGetx'] + cache_stats['l2_2_TotalGetx'] + cache_stats['l2_3_TotalGetx']
	l2_GetsMissRate = cache_stats['l2_0_GetsMissRate'] + cache_stats['l2_1_GetsMissRate'] + cache_stats['l2_2_GetsMissRate'] + cache_stats['l2_3_GetsMissRate']
	l2_GetMissRate = cache_stats['l2_0_GetMissRate'] + cache_stats['l2_1_GetMissRate'] + cache_stats['l2_2_GetMissRate'] + cache_stats['l2_3_GetMissRate']
	l2_GetxMissRate = cache_stats['l2_0_GetxMissRate'] + cache_stats['l2_1_GetxMissRate'] + cache_stats['l2_2_GetxMissRate'] + cache_stats['l2_3_GetxMissRate']
	l2_TotalUpgrades = cache_stats['l2_0_TotalUpgrades'] + cache_stats['l2_1_TotalUpgrades'] + cache_stats['l2_2_TotalUpgrades'] + cache_stats['l2_3_TotalUpgrades']
	l2_UpgradeMissRate = cache_stats['l2_0_UpgradeMissRate'] + cache_stats['l2_1_UpgradeMissRate'] + cache_stats['l2_2_UpgradeMissRate'] + cache_stats['l2_3_UpgradeMissRate']
	l2_TotalWriteBacks = cache_stats['l2_0_TotalWriteBacks'] + cache_stats['l2_1_TotalWriteBacks'] + cache_stats['l2_2_TotalWriteBacks'] + cache_stats['l2_3_TotalWriteBacks']
	l2_CacheUtilization = cache_stats['l2_0_CacheUtilization'] + cache_stats['l2_1_CacheUtilization'] + cache_stats['l2_2_CacheUtilization'] + cache_stats['l2_3_CacheUtilization']

	l3_Occupancy = cache_stats['l3_0_Occupancy'] + cache_stats['l3_1_Occupancy'] + cache_stats['l3_2_Occupancy'] + cache_stats['l3_3_Occupancy']
	l3_OccupancyPct = cache_stats['l3_0_OccupancyPct'] + cache_stats['l3_1_OccupancyPct'] + cache_stats['l3_2_OccupancyPct'] + cache_stats['l3_3_OccupancyPct']
	l3_TotalAdvances = cache_stats['l3_0_TotalAdvances'] + cache_stats['l3_1_TotalAdvances'] + cache_stats['l3_2_TotalAdvances'] + cache_stats['l3_3_TotalAdvances']
	l3_TotalAccesses = cache_stats['l3_0_TotalAccesses'] + cache_stats['l3_1_TotalAccesses'] + cache_stats['l3_2_TotalAccesses'] + cache_stats['l3_3_TotalAccesses']
	l3_TotalHits = cache_stats['l3_0_TotalHits'] + cache_stats['l3_1_TotalHits'] + cache_stats['l3_2_TotalHits'] + cache_stats['l3_3_TotalHits']
	l3_TotalMisses = cache_stats['l3_0_TotalMisses'] + cache_stats['l3_1_TotalMisses'] + cache_stats['l3_2_TotalMisses'] + cache_stats['l3_3_TotalMisses']
	l3_MissRate = cache_stats['l3_0_MissRate'] + cache_stats['l3_1_MissRate'] + cache_stats['l3_2_MissRate'] + cache_stats['l3_3_MissRate']
	l3_TotalReads = cache_stats['l3_0_TotalReads'] + cache_stats['l3_1_TotalReads'] + cache_stats['l3_2_TotalReads'] + cache_stats['l3_3_TotalReads']
	l3_TotalReadMisses = cache_stats['l3_0_TotalReadMisses'] + cache_stats['l3_1_TotalReadMisses'] + cache_stats['l3_2_TotalReadMisses'] + cache_stats['l3_3_TotalReadMisses']
	l3_ReadMissRate = cache_stats['l3_0_ReadMissRate'] + cache_stats['l3_1_ReadMissRate'] + cache_stats['l3_2_ReadMissRate'] + cache_stats['l3_3_ReadMissRate']
	l3_TotalWrites = cache_stats['l3_0_TotalWrites'] + cache_stats['l3_1_TotalWrites'] + cache_stats['l3_2_TotalWrites'] + cache_stats['l3_3_TotalWrites']
	l3_TotalWriteMisses = cache_stats['l3_0_TotalWriteMisses'] + cache_stats['l3_1_TotalWriteMisses'] + cache_stats['l3_2_TotalWriteMisses'] + cache_stats['l3_3_TotalWriteMisses']
	l3_WriteMissRate = cache_stats['l3_0_WriteMissRate'] + cache_stats['l3_1_WriteMissRate'] + cache_stats['l3_2_WriteMissRate'] + cache_stats['l3_3_WriteMissRate']
	l3_TotalGets = cache_stats['l3_0_TotalGets'] + cache_stats['l3_1_TotalGets'] + cache_stats['l3_2_TotalGets'] + cache_stats['l3_3_TotalGets']
	l3_TotalGet = cache_stats['l3_0_TotalGet'] + cache_stats['l3_1_TotalGet'] + cache_stats['l3_2_TotalGet'] + cache_stats['l3_3_TotalGet']
	l3_TotalGetx = cache_stats['l3_0_TotalGetx'] + cache_stats['l3_1_TotalGetx'] + cache_stats['l3_2_TotalGetx'] + cache_stats['l3_3_TotalGetx']
	l3_GetsMissRate = cache_stats['l3_0_GetsMissRate'] + cache_stats['l3_1_GetsMissRate'] + cache_stats['l3_2_GetsMissRate'] + cache_stats['l3_3_GetsMissRate']
	l3_GetMissRate = cache_stats['l3_0_GetMissRate'] + cache_stats['l3_1_GetMissRate'] + cache_stats['l3_2_GetMissRate'] + cache_stats['l3_3_GetMissRate']
	l3_GetxMissRate = cache_stats['l3_0_GetxMissRate'] + cache_stats['l3_1_GetxMissRate'] + cache_stats['l3_2_GetxMissRate'] + cache_stats['l3_3_GetxMissRate']
	l3_TotalUpgrades = cache_stats['l3_0_TotalUpgrades'] + cache_stats['l3_1_TotalUpgrades'] + cache_stats['l3_2_TotalUpgrades'] + cache_stats['l3_3_TotalUpgrades']
	l3_UpgradeMissRate = cache_stats['l3_0_UpgradeMissRate'] + cache_stats['l3_1_UpgradeMissRate'] + cache_stats['l3_2_UpgradeMissRate'] + cache_stats['l3_3_UpgradeMissRate']
	l3_TotalWriteBacks = cache_stats['l3_0_TotalWriteBacks'] + cache_stats['l3_1_TotalWriteBacks'] + cache_stats['l3_2_TotalWriteBacks'] + cache_stats['l3_3_TotalWriteBacks']
	l3_CacheUtilization = cache_stats['l3_0_CacheUtilization'] + cache_stats['l3_1_CacheUtilization'] + cache_stats['l3_2_CacheUtilization'] + cache_stats['l3_3_CacheUtilization']

	table_cache_combined = [
			["Occupancy", l1_i_Occupancy, l1_d_Occupancy, l2_Occupancy, l3_Occupancy],
			["OccupancyPct(X.XX%)", l1_i_OccupancyPct, l1_d_OccupancyPct, l2_OccupancyPct, l3_OccupancyPct],
			["TotalAdvances", l1_i_TotalAdvances, l1_d_TotalAdvances, l2_TotalAdvances, l3_TotalAdvances],
			["TotalAccesses", l1_i_TotalAccesses, l1_d_TotalAccesses, l2_TotalAccesses, l3_TotalAccesses], 
			["TotalHits", l1_i_TotalHits, l1_d_TotalHits, l2_TotalHits, l3_TotalHits],
			["TotalMisses", l1_i_TotalMisses, l1_d_TotalMisses, l2_TotalMisses, l3_TotalMisses],
			["MissRate", l1_i_MissRate, l1_d_MissRate, l2_MissRate, l3_MissRate],
			["TotalReads", l1_i_TotalReads, l1_d_TotalReads, l2_TotalReads, l3_TotalReads],
			["TotalReadMisses", l1_i_TotalReadMisses, l1_d_TotalReadMisses, l2_TotalReadMisses, l3_TotalReadMisses],
			["ReadMissRate", l1_i_ReadMissRate, l1_d_ReadMissRate, l2_ReadMissRate, l3_ReadMissRate],
			["TotalWrites", l1_i_TotalWrites, l1_d_TotalWrites, l2_TotalWrites, l3_TotalWrites],
			["TotalWriteMisses", l1_i_TotalWriteMisses, l1_d_TotalWriteMisses, l2_TotalWriteMisses, l3_TotalWriteMisses],
			["WriteMissRate", l1_i_WriteMissRate, l1_d_WriteMissRate, l2_WriteMissRate, l3_WriteMissRate],
			["TotalGet", l1_i_TotalGets, l1_d_TotalGets, l2_TotalGets, l3_TotalGets],
			["TotalGet", l1_i_TotalGet, l1_d_TotalGet, l2_TotalGet, l3_TotalGet],
			["TotalGetx", l1_i_TotalGetx, l1_d_TotalGetx, l2_TotalGetx, l3_TotalGetx],
			["GetMissRate", l1_i_GetsMissRate, l1_d_GetsMissRate, l2_GetsMissRate, l3_GetsMissRate],
			["GetMissRate", l1_i_GetMissRate, l1_d_GetMissRate, l2_GetMissRate, l3_GetMissRate],
			["GetxMissRate", l1_i_GetxMissRate, l1_d_GetxMissRate, l2_GetxMissRate, l3_GetxMissRate],
			["TotalUpgrades", l1_i_TotalUpgrades, l1_d_TotalUpgrades, l2_TotalUpgrades, l3_TotalUpgrades],
			["UpgradeMissRate", l1_i_UpgradeMissRate, l1_d_UpgradeMissRate, l2_UpgradeMissRate, l3_UpgradeMissRate],
			["TotalWriteBacks", l1_i_TotalWriteBacks, l1_d_TotalWriteBacks, l2_TotalWriteBacks, l3_TotalWriteBacks],
			["CacheUtilization", l1_i_CacheUtilization, l1_d_CacheUtilization, l2_CacheUtilization, l3_CacheUtilization]
			]

	f = open(options.OutFileName, 'a')

	f.write("//Cache Stats//////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')

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
		if isinstance(tup[1], int):
			f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')
		else:
			f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write('\n')
		
	f.close()

	return

def print_switch_stats(options):
	
	switch_data = ConfigParser.ConfigParser()
	switch_data.optionxform = str 
	switch_data.read(options.InFileName)

	#pull of the memory system stats 	
	switch_stats = dict(switch_data.items('FullRunStats'))

	for key, value in switch_stats.items(): #get the (key, value) tuples one at a time
		try:
			switch_stats[key] = int(value)
		except ValueError:
			switch_stats[key] = float(value)
	
	
	table_switch_data_p4 = [
	["TotalSwitchCtrlLoops", switch_stats['s_0_TotalSwitchCtrlLoops'], switch_stats['s_1_TotalSwitchCtrlLoops'], switch_stats['s_2_TotalSwitchCtrlLoops'], switch_stats['s_3_TotalSwitchCtrlLoops'], switch_stats['s_4_TotalSwitchCtrlLoops']],
	["SwitchOccupancy", switch_stats['s_0_SwitchOccupance'], switch_stats['s_1_SwitchOccupance'], switch_stats['s_2_SwitchOccupance'], switch_stats['s_3_SwitchOccupance'], switch_stats['s_4_SwitchOccupance']],
	["NumberLinks", switch_stats['s_0_NumberLinks'], switch_stats['s_1_NumberLinks'], switch_stats['s_2_NumberLinks'], switch_stats['s_3_NumberLinks'], switch_stats['s_4_NumberLinks']],
	["MaxNumberLinks", switch_stats['s_0_MaxNumberLinks'], switch_stats['s_1_MaxNumberLinks'], switch_stats['s_2_MaxNumberLinks'], switch_stats['s_3_MaxNumberLinks'], switch_stats['s_4_MaxNumberLinks']],
	["AveNumberLinksPerAccess", switch_stats['s_0_AveNumberLinksPerAccess'], switch_stats['s_1_AveNumberLinksPerAccess'], switch_stats['s_2_AveNumberLinksPerAccess'], switch_stats['s_3_AveNumberLinksPerAccess'], switch_stats['s_4_AveNumberLinksPerAccess']],
	["NorthIOTransfers", switch_stats['s_0_NorthIOTransfers'], switch_stats['s_1_NorthIOTransfers'], switch_stats['s_2_NorthIOTransfers'], switch_stats['s_3_NorthIOTransfers'], switch_stats['s_4_NorthIOTransfers']],
	["NorthIOCycles", switch_stats['s_0_NorthIOCycles'], switch_stats['s_1_NorthIOCycles'], switch_stats['s_2_NorthIOCycles'], switch_stats['s_3_NorthIOCycles'], switch_stats['s_4_NorthIOCycles']],
	["NorthIOBytesTransfered", switch_stats['s_0_NorthIOBytesTransfered'], switch_stats['s_1_NorthIOBytesTransfered'], switch_stats['s_2_NorthIOBytesTransfered'], switch_stats['s_3_NorthIOBytesTransfered'], switch_stats['s_4_NorthIOBytesTransfered']],
	["NorthRxQueueMaxDepth", switch_stats['s_0_NorthRxQueueMaxDepth'], switch_stats['s_1_NorthRxQueueMaxDepth'], switch_stats['s_2_NorthRxQueueMaxDepth'], switch_stats['s_3_NorthRxQueueMaxDepth'], switch_stats['s_4_NorthRxQueueMaxDepth']],
	["NorthRxQueueAveDepth", switch_stats['s_0_NorthRxQueueAveDepth'], switch_stats['s_1_NorthRxQueueAveDepth'], switch_stats['s_2_NorthRxQueueAveDepth'], switch_stats['s_3_NorthRxQueueAveDepth'], switch_stats['s_4_NorthRxQueueAveDepth']],
	["NorthTxQueueMaxDepth", switch_stats['s_0_NorthTxQueueMaxDepth'], switch_stats['s_1_NorthTxQueueMaxDepth'], switch_stats['s_2_NorthTxQueueMaxDepth'], switch_stats['s_3_NorthTxQueueMaxDepth'], switch_stats['s_4_NorthTxQueueMaxDepth']],
	["NorthTxQueueAveDepth", switch_stats['s_0_NorthTxQueueAveDepth'], switch_stats['s_1_NorthTxQueueAveDepth'], switch_stats['s_2_NorthTxQueueAveDepth'], switch_stats['s_3_NorthTxQueueAveDepth'], switch_stats['s_4_NorthTxQueueAveDepth']],
	["EastIOTransfers", switch_stats['s_0_EastIOTransfers'], switch_stats['s_1_EastIOTransfers'], switch_stats['s_2_EastIOTransfers'], switch_stats['s_3_EastIOTransfers'], switch_stats['s_4_EastIOTransfers']],
	["EastIOCycles", switch_stats['s_0_EastIOCycles'], switch_stats['s_1_EastIOCycles'], switch_stats['s_2_EastIOCycles'], switch_stats['s_3_EastIOCycles'], switch_stats['s_4_EastIOCycles']],
	["EastIOBytesTransfered", switch_stats['s_0_EastIOBytesTransfered'], switch_stats['s_1_EastIOBytesTransfered'], switch_stats['s_2_EastIOBytesTransfered'], switch_stats['s_3_EastIOBytesTransfered'], switch_stats['s_4_EastIOBytesTransfered']],
	["EastRxQueueMaxDepth", switch_stats['s_0_EastRxQueueMaxDepth'], switch_stats['s_1_EastRxQueueMaxDepth'], switch_stats['s_2_EastRxQueueMaxDepth'], switch_stats['s_3_EastRxQueueMaxDepth'], switch_stats['s_4_EastRxQueueMaxDepth']],
	["EastRxQueueAveDepth", switch_stats['s_0_EastRxQueueAveDepth'], switch_stats['s_1_EastRxQueueAveDepth'], switch_stats['s_2_EastRxQueueAveDepth'], switch_stats['s_3_EastRxQueueAveDepth'], switch_stats['s_4_EastRxQueueAveDepth']],
	["EastTxQueueMaxDepth", switch_stats['s_0_EastTxQueueMaxDepth'], switch_stats['s_1_EastTxQueueMaxDepth'], switch_stats['s_2_EastTxQueueMaxDepth'], switch_stats['s_3_EastTxQueueMaxDepth'], switch_stats['s_4_EastTxQueueMaxDepth']],
	["EastTxQueueAveDepth", switch_stats['s_0_EastTxQueueAveDepth'], switch_stats['s_1_EastTxQueueAveDepth'], switch_stats['s_2_EastTxQueueAveDepth'], switch_stats['s_3_EastTxQueueAveDepth'], switch_stats['s_4_EastTxQueueAveDepth']],
	["SouthIOTransfers", switch_stats['s_0_SouthIOTransfers'], switch_stats['s_1_SouthIOTransfers'], switch_stats['s_2_SouthIOTransfers'], switch_stats['s_3_SouthIOTransfers'], switch_stats['s_4_SouthIOTransfers']],
	["SouthIOCycles", switch_stats['s_0_SouthIOCycles'], switch_stats['s_1_SouthIOCycles'], switch_stats['s_2_SouthIOCycles'], switch_stats['s_3_SouthIOCycles'], switch_stats['s_4_SouthIOCycles']],
	["SouthIOBytesTransfered", switch_stats['s_0_SouthIOBytesTransfered'], switch_stats['s_1_SouthIOBytesTransfered'], switch_stats['s_2_SouthIOBytesTransfered'], switch_stats['s_3_SouthIOBytesTransfered'], switch_stats['s_4_SouthIOBytesTransfered']],
	["SouthRxQueueMaxDepth", switch_stats['s_0_SouthRxQueueMaxDepth'], switch_stats['s_1_SouthRxQueueMaxDepth'], switch_stats['s_2_SouthRxQueueMaxDepth'], switch_stats['s_3_SouthRxQueueMaxDepth'], switch_stats['s_4_SouthRxQueueMaxDepth']],
	["SouthRxQueueAveDepth", switch_stats['s_0_SouthRxQueueAveDepth'], switch_stats['s_1_SouthRxQueueAveDepth'], switch_stats['s_2_SouthRxQueueAveDepth'], switch_stats['s_3_SouthRxQueueAveDepth'], switch_stats['s_4_SouthRxQueueAveDepth']],
	["SouthTxQueueMaxDepth", switch_stats['s_0_SouthTxQueueMaxDepth'], switch_stats['s_1_SouthTxQueueMaxDepth'], switch_stats['s_2_SouthTxQueueMaxDepth'], switch_stats['s_3_SouthTxQueueMaxDepth'], switch_stats['s_4_SouthTxQueueMaxDepth']],
	["SouthTxQueueAveDepth", switch_stats['s_0_SouthTxQueueAveDepth'], switch_stats['s_1_SouthTxQueueAveDepth'], switch_stats['s_2_SouthTxQueueAveDepth'], switch_stats['s_3_SouthTxQueueAveDepth'], switch_stats['s_4_SouthTxQueueAveDepth']],
	["WestIOTransfers", switch_stats['s_0_WestIOTransfers'], switch_stats['s_1_WestIOTransfers'], switch_stats['s_2_WestIOTransfers'], switch_stats['s_3_WestIOTransfers'], switch_stats['s_4_WestIOTransfers']],
	["WestIOCycles", switch_stats['s_0_WestIOCycles'], switch_stats['s_1_WestIOCycles'], switch_stats['s_2_WestIOCycles'], switch_stats['s_3_WestIOCycles'], switch_stats['s_4_WestIOCycles']],
	["WestIOBytesTransfered", switch_stats['s_0_WestIOBytesTransfered'], switch_stats['s_1_WestIOBytesTransfered'], switch_stats['s_2_WestIOBytesTransfered'], switch_stats['s_3_WestIOBytesTransfered'], switch_stats['s_4_WestIOBytesTransfered']],
	["WestRxQueueMaxDepth", switch_stats['s_0_WestRxQueueMaxDepth'], switch_stats['s_1_WestRxQueueMaxDepth'], switch_stats['s_2_WestRxQueueMaxDepth'], switch_stats['s_3_WestRxQueueMaxDepth'], switch_stats['s_4_WestRxQueueMaxDepth']],
	["WestRxQueueAveDepth", switch_stats['s_0_WestRxQueueAveDepth'], switch_stats['s_1_WestRxQueueAveDepth'], switch_stats['s_2_WestRxQueueAveDepth'], switch_stats['s_3_WestRxQueueAveDepth'], switch_stats['s_4_WestRxQueueAveDepth']],
	["WestTxQueueMaxDepth", switch_stats['s_0_WestTxQueueMaxDepth'], switch_stats['s_1_WestTxQueueMaxDepth'], switch_stats['s_2_WestTxQueueMaxDepth'], switch_stats['s_3_WestTxQueueMaxDepth'], switch_stats['s_4_WestTxQueueMaxDepth']],
	["WestTxQueueAveDepth", switch_stats['s_0_WestTxQueueAveDepth'], switch_stats['s_1_WestTxQueueAveDepth'], switch_stats['s_2_WestTxQueueAveDepth'], switch_stats['s_3_WestTxQueueAveDepth'], switch_stats['s_4_WestTxQueueAveDepth']],
	]

	f = open(options.OutFileName, 'a')
	f.write('//Switch Stats/////////////////////////////////////////////////' +'\n')
	f.write('///////////////////////////////////////////////////////////////'  + '\n\n')	
	
	#get the largest title length	
	max_title_length = len('Stat Switches')
	current_title_length = 0

	
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

	samc_data = ConfigParser.ConfigParser()
	samc_data.optionxform = str 
	samc_data.read(options.InFileName)

	#pull of the memory system stats 	
	samc_stats = dict(samc_data.items('FullRunStats'))

	for key, value in samc_stats.items(): #get the (key, value) tuples one at a time
		try:
			samc_stats[key] = int(value)
		except ValueError:
			samc_stats[key] = float(value)

	#bridge some of the stats so we can put evetyhing in one table...
	samc_0 = 0;

	table_sa_data = [
	["TotalCtrlLoops", samc_stats['sa_TotalCtrlLoops'], samc_stats['mc_MemCtrlBusyCycles']],
	["TotalLoads", samc_stats['sa_MCLoads'], samc_stats['mc_TotalReads']],
	["TotalStores", samc_stats['sa_MCStores'], samc_stats['mc_TotalWrites']],
	["TotalReturns", samc_stats['sa_MCReturns'], samc_0],
	["NorthIOCycles", samc_stats['sa_NorthIOBusyCycles'], samc_stats['mc_IOBusyCycles']],
	["NorthMaxRxQueueDepth", samc_stats['sa_MaxNorthRxQueueDepth'], samc_stats['mc_RxMax']],
	["NorthAveRxQueueDepth", samc_stats['sa_AveNorthRxQueueDepth'], samc_0],
	["NorthMaxTxQueueDepth", samc_stats['sa_MaxNorthTxQueueDepth'], samc_stats['mc_TxMax']],
	["NorthAveTxQueueDepth", samc_stats['sa_AveNorthTxQueueDepth'], samc_0],
	["SouthIOCycles", samc_stats['sa_SouthIOBusyCycles'], samc_0],
	["SouthMaxRxQueueDepth", samc_stats['sa_MaxSouthRxQueueDepth'], samc_0],
	["SouthAveRxQueueDepth", samc_stats['sa_AveSouthRxQueueDepth'], samc_0],
	["SouthMaxTxQueueDepth", samc_stats['sa_MaxSouthTxQueueDepth'], samc_0],
	["SouthAveTxQueueDepth", samc_stats['sa_AveSouthTxQueueDepth'], samc_0],
	["DramBusyCycles", samc_0, samc_stats['mc_DramBusyCycles']],
	["DramAveReadLat", samc_0, samc_stats['mc_AveDramReadLat']],
	["DramAveWriteLat", samc_0, samc_stats['mc_AveDramWriteLat']],
	["DramAveTotalLat(cyc)", samc_0, samc_stats['mc_AveDramTotalLat(cycles)']],
	["DramAveTotalLat(ns)", samc_0, samc_stats['mc_AveDramTotalLat(ns)']],
	["DramReadMinLat", samc_0, samc_stats['mc_ReadMinLat']],
	["DramReadMaxLat", samc_0, samc_stats['mc_ReadMaxLat']],
	["DramWriteMinLat", samc_0, samc_stats['mc_WriteMinLat']],
	["DramWriteMaxLat", samc_0, samc_stats['mc_WriteMaxLat']],
	["DramMaxQueueDepth", samc_0, samc_stats['mc_DramMaxQueueDepth']],
	["DramAveQueueDepth", samc_0, samc_stats['mc_DramAveQueueDepth']],
	["DramTotalBytesRead", samc_0, samc_stats['mc_ByteRead']],
	["DramTotalBytesWritten", samc_0, samc_stats['mc_BytesWrote']],
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

	ms_data = ConfigParser.ConfigParser()
	ms_data.optionxform = str
	ms_data.read(options.InFileName)

	#memory system stats 	
	ms_stats = dict(ms_data.items('FullRunStats'))
			
	table_mem_system_data = [
	["FirstAccessLat(Fetch)", ms_stats['MemSystem_FirstAccessLat(Fetch)']],
	["TotalFetches", ms_stats['MemSystem_TotalFetches']],
	["FetchesL1", ms_stats['MemSystem_FetchesL1']],
	["FetchesL2", ms_stats['MemSystem_FetchesL2']],
	["FetchesL3", ms_stats['MemSystem_FetchesL3']],
	["FetchesMemory", ms_stats['MemSystem_FetchesMemory']],
	["TotalLoads", ms_stats['MemSystem_TotalLoads']],
	["LoadsL1", ms_stats['MemSystem_LoadsL1']],
	["LoadsL2", ms_stats['MemSystem_LoadsL2']],
	["LoadsL3", ms_stats['MemSystem_LoadsL3']],
	["LoadsMemory", ms_stats['MemSystem_LoadsMemory']],
	["LoadsGetFwd", ms_stats['MemSystem_LoadsGetFwd']],
	["TotalStore", ms_stats['MemSystem_TotalStore']],
	["StoresL1", ms_stats['MemSystem_StoresL1']],
	["StoresL2", ms_stats['MemSystem_StoresL2']],
	["StoresL3", ms_stats['MemSystem_StoresL3']],
	["StoresMemory", ms_stats['MemSystem_StoresMemory']],
	["StoresGetxFwd", ms_stats['MemSystem_StoresGetxFwd']],
	["StoresUpgrade", ms_stats['MemSystem_StoresUpgrade']]
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



def print_cpu_stats(options):

	cpu_data = ConfigParser.ConfigParser()
	cpu_data.optionxform = str
	cpu_data.read(options.InFileName)
	
	#pull stats
	if options.PrintSection == 'FullRunStats':
		cpu_stats = dict(cpu_data.items('FullRunStats'))
	elif options.PrintSection == 'ParallelStats':
		cpu_stats = dict(cpu_data.items('ParallelStats'))
	else:
		print "print_cpu_stats(): invalid section"
		exit(0)

	table_P4 = [
		["NumSyscalls", cpu_stats['core_0_NumSyscalls'], cpu_stats['core_1_NumSyscalls'], cpu_stats['core_2_NumSyscalls'], cpu_stats['core_3_NumSyscalls']],
		["ROBStalls", cpu_stats['core_0_ROBStalls'], cpu_stats['core_1_ROBStalls'], cpu_stats['core_2_ROBStalls'], cpu_stats['core_3_ROBStalls']],
		["ROBStallLoad", cpu_stats['core_0_ROBStallLoad'], cpu_stats['core_1_ROBStallLoad'], cpu_stats['core_2_ROBStallLoad'], cpu_stats['core_3_ROBStallLoad']],
		["ROBStallStore", cpu_stats['core_0_ROBStallStore'], cpu_stats['core_1_ROBStallStore'], cpu_stats['core_2_ROBStallStore'], cpu_stats['core_3_ROBStallStore']],
		["ROBStallOther", cpu_stats['core_0_ROBStallOther'], cpu_stats['core_1_ROBStallOther'], cpu_stats['core_2_ROBStallOther'], cpu_stats['core_3_ROBStallOther']],
		["FirstFetchCycle", cpu_stats['core_0_FirstFetchCycle'], cpu_stats['core_1_FirstFetchCycle'], cpu_stats['core_2_FirstFetchCycle'], cpu_stats['core_3_FirstFetchCycle']],
		["LastCommitCycle", cpu_stats['core_0_LastCommitCycle'], cpu_stats['core_1_LastCommitCycle'], cpu_stats['core_2_LastCommitCycle'], cpu_stats['core_3_LastCommitCycle']],
		["FetchStall", cpu_stats['core_0_FetchStall'], cpu_stats['core_1_FetchStall'], cpu_stats['core_2_FetchStall'], cpu_stats['core_3_FetchStall']],
		["RunTime", cpu_stats['core_0_RunTime'], cpu_stats['core_1_RunTime'], cpu_stats['core_2_RunTime'], cpu_stats['core_3_RunTime']],
		["IdleTime", cpu_stats['core_0_IdleTime'], cpu_stats['core_1_IdleTime'], cpu_stats['core_2_IdleTime'], cpu_stats['core_3_IdleTime']],
		["SystemTime(SysCalls)", cpu_stats['core_0_SystemTime'], cpu_stats['core_1_SystemTime'], cpu_stats['core_2_SystemTime'], cpu_stats['core_3_SystemTime']],
		["StallTime", cpu_stats['core_0_StallTime'], cpu_stats['core_1_StallTime'], cpu_stats['core_2_StallTime'], cpu_stats['core_3_StallTime']],
		["BusyTime", cpu_stats['core_0_BusyTime'], cpu_stats['core_1_BusyTime'], cpu_stats['core_2_BusyTime'], cpu_stats['core_3_BusyTime']],
		["IdlePct", cpu_stats['core_0_IdlePct'], cpu_stats['core_1_IdlePct'], cpu_stats['core_2_IdlePct'], cpu_stats['core_3_IdlePct']],
		["RunPct", cpu_stats['core_0_RunPct'], cpu_stats['core_1_RunPct'], cpu_stats['core_2_RunPct'], cpu_stats['core_3_RunPct']],
		["SystemPct", cpu_stats['core_0_SystemPct'], cpu_stats['core_1_SystemPct'], cpu_stats['core_2_SystemPct'], cpu_stats['core_3_SystemPct']],
		["StallPct", cpu_stats['core_0_StallPct'], cpu_stats['core_1_StallPct'], cpu_stats['core_2_StallPct'], cpu_stats['core_3_StallPct']],
		["BusyPct", cpu_stats['core_0_BusyPct'], cpu_stats['core_1_BusyPct'], cpu_stats['core_2_BusyPct'], cpu_stats['core_3_BusyPct']],
		["StallfetchPct", cpu_stats['core_0_StallfetchPct'], cpu_stats['core_1_StallfetchPct'], cpu_stats['core_2_StallfetchPct'], cpu_stats['core_3_StallfetchPct']],
		["StallLoadPct", cpu_stats['core_0_StallLoadPct'], cpu_stats['core_1_StallLoadPct'], cpu_stats['core_2_StallLoadPct'], cpu_stats['core_3_StallLoadPct']],	
		["StallStorePct", cpu_stats['core_0_StallStorePct'], cpu_stats['core_1_StallStorePct'], cpu_stats['core_2_StallStorePct'], cpu_stats['core_3_StallStorePct']],
		["StallOtherPct", cpu_stats['core_0_StallOtherPct'], cpu_stats['core_1_StallOtherPct'], cpu_stats['core_2_StallOtherPct'], cpu_stats['core_3_StallOtherPct']]
		]

	
	f = open(options.OutFileName, 'a')

	f.write("//CPU Stats////////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')
	
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

def print_general_stats(options):



	general_data = ConfigParser.ConfigParser()
	general_data.optionxform = str 
	general_data.read(options.InFileName)
	
	#pull of the general stats 	
	general_stats = dict(general_data.items('General'))

	#bench_args = general_data.get('General', 'Benchmark')
	#day_time = general_data.get('General', 'Day&Time')
	#total_cycles = general_data.getint('General', 'TotalCycles')
	#run_time_cpu = general_data.getfloat('General', 'SimulationRunTimeSeconds(cpu)')
	#run_time_wall = general_data.getfloat('General', 'SimulationRunTimeSeconds(wall)')
	#simulated_cycles_per_sec = general_data.getfloat('General', 'SimulatedCyclesPerSec')
	#cpu_num_cores = general_data.getint('General', 'CPU_NumCores')
	#cpu_threads_per_core = general_data.getint('General', 'CPU_ThreadsPerCore')
	#cpu_freq_ghz = general_data.getint('General', 'CPU_FreqGHz')
	
	table_general_data = [
	["BenchmarkArgs", general_stats['Benchmark']],
	["DateTime", general_stats['Day&Time']],
	["ExeSuccessful", general_stats['ExecutionSuccessful']],
	["TotalCycles", general_stats['TotalCycles']],
	["ParallelSectionCycles", general_stats['ParallelSectionCycles']],
	["CPURunTime(sec)", general_stats['SimulationRunTimeSeconds(cpu)']],
	["WallRunTime(sec)", general_stats['SimulationRunTimeSeconds(wall)']],
	["SimulatedCyclesPerSec", general_stats['SimulatedCyclesPerSec']],
	["CPUNumCores", general_stats['CPU_NumCores']],
	["CPUThreadsPerCore", general_stats['CPU_ThreadsPerCore']],
	["CPUFreqGhz", general_stats['CPU_FreqGHz']]
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


parser = OptionParser()
parser.usage = "%prog -s section -i inputfile -o outputfile"
parser.add_option("-s", "--section", dest="PrintSection", default="", help="Specifiy the stats section to parse.")
parser.add_option("-i", "--infile", dest="InFileName", default="", help="Specifiy the stats file and path to parse.")
parser.add_option("-o", "--outfile", dest="OutFileName", default="sim_stats.txt", help="Specifiy the outputfile name and path.")
(options, args) = parser.parse_args()

if not options.PrintSection:
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
