#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser


def print_cache_stats(options):

	cache_data = ConfigParser.RawConfigParser()
	cache_data.optionxform = str 
	cache_data.read(options.InFileName)

	#pull stats
	if options.PrintSection == 'FullRunStats':
		cache_stats_dict = dict(cache_data.items('FullRunStats'))
	elif options.PrintSection == 'ParallelStats':
		cache_stats_dict = dict(cache_data.items('ParallelStats'))
	else:
		print "print_cache_stats(): invalid section"
		exit(0)

	for key, value in cache_stats_dict.items(): # get the (key, value) tuples one at a time
		try:
			cache_stats_dict[key] = int(value)
		except ValueError:
			cache_stats_dict[key] = float(value)


	var = ""
	cache_stats = ["Occupancy", "OccupancyPct", "Stalls", "TotalWriteBlocks", "CoalescePut", "CoalesceGet", "WbMerges", "MergeRetries", "WbRecieved", "WbSent", "SharingWbSent", "WbDropped", "UpgradeMisses", "TotalUpgrades", "TotalUpgradeAcks", "TotalUpgradeInvals", "TotalDowngrades", "TotalGetxFwdInvals", "EvictInv", "TotalUpgradeInvals", "AveCyclesPerAdvance", "TotalAdvances", "TotalAccesses", "TotalHits", "TotalMisses", "MissRate", "TotalReads", "TotalReadMisses", "ReadMissRate", "TotalWrites", "TotalWriteMisses", "WriteMissRate", "TotalGet", "TotalGet", "TotalGetx", "GetMissRate", "GetMissRate", "GetxMissRate", "UpgradeMissRate", "TotalWriteBacks", "CacheUtilization"]

	l1_i_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(cache_stats))]
	l1_d_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(cache_stats))]
	l2_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(cache_stats))]
	l3_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(cache_stats))]
	
	for i in range (0,len(cache_stats)):
		l1_i_stats_table[i][0] = cache_stats[i]
		for j in range (0,num_cores):
			var = "l1_i_" + str(j) + "_" + cache_stats[i]
			l1_i_stats_table[i][(j+1)] = cache_stats_dict[var]

	for i in range (0,len(cache_stats)):
		l1_d_stats_table[i][0] = cache_stats[i]
		for j in range (0,num_cores):
			var = "l1_d_" + str(j) + "_" + cache_stats[i]
			l1_d_stats_table[i][(j+1)] = cache_stats_dict[var]

	for i in range (0,len(cache_stats)):
		l2_stats_table[i][0] = cache_stats[i]
		for j in range (0,num_cores):
			var = "l2_" + str(j) + "_" + cache_stats[i]
			l2_stats_table[i][(j+1)] = cache_stats_dict[var]
			
	for i in range (0,len(cache_stats)):
		l3_stats_table[i][0] = cache_stats[i]
		for j in range (0,num_cores):
			var = "l3_" + str(j) + "_" + cache_stats[i]
			l3_stats_table[i][(j+1)] = cache_stats_dict[var]


	
		
	


	table_cache_combined = [
			["Occupancy", l1_i_Occupancy, l1_d_Occupancy, l2_Occupancy, l3_Occupancy],
			["OccupancyPct(X.XX%)", l1_i_OccupancyPct, l1_d_OccupancyPct, l2_OccupancyPct, l3_OccupancyPct],			
			["Stalls", l1_i_Stalls, l1_d_Stalls, l2_Stalls, l3_Stalls],
			["WriteBlocks", l1_i_WriteBlocks, l1_d_WriteBlocks, l2_WriteBlocks, l3_WriteBlocks],
			["CoalescePut", l1_i_CoalescePut, l1_d_CoalescePut, l2_CoalescePut, l3_CoalescePut],
			["CoalesceGet", l1_i_CoalesceGet, l1_d_CoalesceGet, l2_CoalesceGet, l3_CoalesceGet],
			["WbMerges", l1_i_WbMerges, l1_d_WbMerges, l2_WbMerges, l3_WbMerges],
			["MergeRetries", l1_i_MergeRetries, l1_d_MergeRetries, l2_MergeRetries, l3_MergeRetries],
			["WbRecieved", l1_i_WbRecieved, l1_d_WbRecieved, l2_WbRecieved, l3_WbRecieved],
			["WbSent", l1_i_WbSent, l1_d_WbSent, l2_WbSent, l3_WbSent],
			["SharingWbSent", l1_i_SharingWbSent, l1_d_SharingWbSent, l2_SharingWbSent, l3_SharingWbSent],
			["WbDropped", l1_i_WbDropped, l1_d_WbDropped, l2_WbDropped, l3_WbDropped],
			["UpgradeMisses", l1_i_UpgradeMisses, l1_d_UpgradeMisses, l2_UpgradeMisses, l3_UpgradeMisses],
			["TotalUpgrades", l1_i_TotalUpgrades, l1_d_TotalUpgrades, l2_TotalUpgrades, l3_TotalUpgrades],
			["UpgradeAcks", l1_i_UpgradeAcks, l1_d_UpgradeAcks, l2_UpgradeAcks, l3_UpgradeAcks],
			["UpgradeInvals", l1_i_UpgradeInvals, l1_d_UpgradeInvals, l2_UpgradeInvals, l3_UpgradeInvals],
			["Downgrades", l1_i_Downgrades, l1_d_Downgrades, l2_Downgrades, l3_Downgrades],
			["GetxFwdInval", l1_i_GetxFwdInval, l1_d_GetxFwdInval, l2_GetxFwdInval, l3_GetxFwdInval],
			["EvictInv", l1_i_EvictInv, l1_d_EvictInv, l2_EvictInv, l3_EvictInv],			
			
			
			["AveCyclesPerAdvance", l1_i_AveCyclesPerAdvance, l1_d_AveCyclesPerAdvance, l2_AveCyclesPerAdvance, l3_AveCyclesPerAdvance],
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


	ms_data = ConfigParser.RawConfigParser()
	ms_data.optionxform = str 
	ms_data.read(options.InFileName)

	#pull stats
	if options.PrintSection == 'FullRunStats':
		ms_stats = dict(ms_data.items('FullRunStats'))
	elif options.PrintSection == 'ParallelStats':
		ms_stats = dict(ms_data.items('ParallelStats'))
	else:
		print "print_cache_stats(): invalid section"
		exit(0)


	for key, value in ms_stats.items(): # get the (key, value) tuples one at a time
		try:
			ms_stats[key] = int(value)
		except ValueError:
			ms_stats[key] = float(value)

	table_mem_system_data = [
			["FirstAccessLat(Fetch)", ms_stats['MemSystem_FirstAccessLat(Fetch)']],
			["TotalCPUFetchRequests", ms_stats['MemSystem_TotalCPUFetchRequests']],
			["TotalCPUFetchReplys", ms_stats['MemSystem_TotalCPUFetchReplys']],
			["L1FetchHits", ms_stats['MemSystem_L1FetchHits']],
			["L2TotalFetches", ms_stats['MemSystem_L2TotalFetches']],
			["L2FetchHits", ms_stats['MemSystem_L2FetchHits']],
			["L3TotalFetches", ms_stats['MemSystem_L3TotalFetches']],
			["L3FetchHits", ms_stats['MemSystem_L3FetchHits']],
			["MMFetches", ms_stats['MemSystem_FetchesMemory']],

			["TotalCPULoadRequests", ms_stats['MemSystem_TotalCPULoadRequests']],
			["TotalCPULoadReplys", ms_stats['MemSystem_TotalCPULoadReplys']],
			["L1LoadHits", ms_stats['MemSystem_L1LoadHits']],
			["L2TotalLoads", ms_stats['MemSystem_L2TotalLoads']],
			["L2LoadHits", ms_stats['MemSystem_L2LoadHits']],
			["L3TotalLoads", ms_stats['MemSystem_L3TotalLoads']],
			["L3LoadHits", ms_stats['MemSystem_L3LoadHits']],
			["MMLoads", ms_stats['MemSystem_LoadsMemory']],
			["LoadGetFwd", ms_stats['MemSystem_LoadsGetFwd']],
			["L2LoadNacks", ms_stats['MemSystem_l2_LoadNacks']],
			["L3LoadNacks", ms_stats['MemSystem_l3_LoadNacks']],

			["TotalCPUStoreRequests", ms_stats['MemSystem_TotalCPUStoreRequests']],
			["TotalCPUStoreReplys", ms_stats['MemSystem_TotalCPUStoreReplys']],
			["L1StoreHits", ms_stats['MemSystem_L1StoreHits']],
			["L2TotalStores", ms_stats['MemSystem_L2TotalStores']],
			["L2StoreHits", ms_stats['MemSystem_L2StoreHits']],
			["L3TotalStores", ms_stats['MemSystem_L3TotalStores']],
			["L3StoreHits", ms_stats['MemSystem_L3StoreHits']],
			["MMStores", ms_stats['MemSystem_StoresMemory']],
			["StoreGetxFwd", ms_stats['MemSystem_StoresGetxFwd']],
			["StoreUpgrade", ms_stats['MemSystem_StoresUpgrade']],
			["L2StoreNacks", ms_stats['MemSystem_l2_StoreNacks']],
			["L3StoreNacks", ms_stats['MemSystem_l3_StoreNacks']],
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
	
	var = ""
	stats = ["NumSyscalls", "ROBStalls", "ROBStallLoad", "ROBStallStore", "ROBStallOther", "FirstFetchCycle", "LastCommitCycle", "FetchStall", "RunTime", "IdleTime", "SystemTime", "StallTime", "BusyTime", "IdlePct", "RunPct", "SystemPct", "StallPct", "BusyPct", "StallfetchPct", "StallLoadPct", "StallStorePct", "StallOtherPct"]
	stat_table = [[0 for x in range(num_cores + 1)] for y in range(len(stats))]
	
	
	for i in range (0,len(stats)):
		stat_table[i][0] = stats[i]
		for j in range (0,num_cores):
			var = "core_" + str(j) + "_" + stats[i]
			stat_table[i][(j+1)] = cpu_stats[var]

			
	f = open(options.OutFileName, 'a')

	f.write("//CPU Stats////////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')
	
	#get the largest title length	
	max_title_length = len('CPU Stats All Cores')
	current_title_length = 0

	for tup in stat_table:
		for item in tup[0:1]:
			current_title_length = len(tup[0])
			if max_title_length < current_title_length:
				max_title_length = current_title_length

	#get the largest data element length
	max_element_length = 0
	current_element_length = 0

	for tup in stat_table:
		for item in tup[1:num_cores]:
			current_element_length = len(str(item))
			if max_element_length < current_element_length:
				max_element_length = current_element_length

	max_title_length += 2
	max_element_length += 2
	#print "max title {} max data {}".format(max_title_length, max_element_length)

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)


	#print the table headers
	f.write("{:<{title_width}}".format("CPU Stats All Cores", title_width=max_title_length))

	for i in range(0,num_cores):
		var = "Core_" + str(i)
		f.write("{:>{data_width}}".format(var, data_width=max_element_length))

	f.write('\n')


	#print the bars
	f.write("{:<{title_width}}".format(title_bar, title_width=max_title_length))

	for i in range(0,num_cores):
		f.write("{:>{data_width}}".format(data_bar, data_width=max_element_length))

	f.write('\n')


	#print the stats
	for tup in stat_table:
		f.write("{:<{title_width}}".format(tup[0], title_width=max_title_length))
		for i in range(0,num_cores):
			f.write("{:>{data_width}}".format(tup[i+1], data_width=max_element_length))	

		f.write('\n')
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
	["SimulatedCyclesPerSec(ave)", general_stats['SimulatedCyclesPerSec']],
	["CPUNumCores", general_stats['CPU_NumCores']],
	["CPUThreadsPerCore", general_stats['CPU_ThreadsPerCore']],
	["CPUFreqGhz", general_stats['CPU_FreqGHz']],
	["GPUNumCUs", general_stats['GPU_NumCUs']],
	["GPUFreqGhz", general_stats['GPU_FreqGHz']]
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

#globals
num_cores = 8
cache_combined = 1

print_general_stats(options)
print_cpu_stats(options)
print_cache_stats(options)
#print_mem_system_stats(options)

#print_switch_stats(options)
#print_samc_stats(options)
