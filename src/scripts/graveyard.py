
#get the largest title length	
	#max_title_length = len('Switch IO stats combined')
	#current_title_length = 0

	#for tup in switch_stats_table_combined:
	#	for item in tup[0:1]:
	#		current_title_length = len(tup[0])
	#		if max_title_length < current_title_length:
	#			max_title_length = current_title_length

	#get the largest data element length
	#max_element_length = 0
	#current_element_length = 0

	#for tup in switch_stats_table_combined:
	#	for item in tup[1:5]:
	#		current_element_length = len(str(item))
	#		if max_element_length < current_element_length:
	#			max_element_length = current_element_length

	#max_title_length += 2
	#max_element_length += 2
	#print "max title {} max data {}".format(max_title_length, max_element_length)


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



	#print the title and bars
	#f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format("Stat Switch",'S0', 'S1', 'S2', 'S3', 'S4', title_width=max_title_length, data_width=max_element_length) + '\n')
	#f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, data_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	#for tup in table_switch_data_p4:
	#	f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], tup[5], title_width=max_title_length, data_width=max_element_length) + '\n')

	#f.write('\n')

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


	

	
	l1_i_Occupancy = cache_stats['l1_i_0_Occupancy'] + cache_stats['l1_i_1_Occupancy'] + cache_stats['l1_i_2_Occupancy'] + cache_stats['l1_i_3_Occupancy']
	l1_i_OccupancyPct = cache_stats['l1_i_0_OccupancyPct'] + cache_stats['l1_i_1_OccupancyPct'] + cache_stats['l1_i_2_OccupancyPct'] + cache_stats['l1_i_3_OccupancyPct']	
	l1_i_Stalls = cache_stats['l1_i_0_Stalls'] + cache_stats['l1_i_1_Stalls'] + cache_stats['l1_i_2_Stalls'] + cache_stats['l1_i_3_Stalls']
	l1_i_WriteBlocks = cache_stats['l1_i_0_TotalWriteBlocks'] + cache_stats['l1_i_1_TotalWriteBlocks'] + cache_stats['l1_i_2_TotalWriteBlocks'] + cache_stats['l1_i_3_TotalWriteBlocks']
	l1_i_CoalescePut = cache_stats['l1_i_0_CoalescePut'] + cache_stats['l1_i_1_CoalescePut'] + cache_stats['l1_i_2_CoalescePut'] + cache_stats['l1_i_3_CoalescePut']
	l1_i_CoalesceGet = cache_stats['l1_i_0_CoalesceGet'] + cache_stats['l1_i_1_CoalesceGet'] + cache_stats['l1_i_2_CoalesceGet'] + cache_stats['l1_i_3_CoalesceGet']
	l1_i_WbMerges = cache_stats['l1_i_0_WbMerges'] + cache_stats['l1_i_1_WbMerges'] + cache_stats['l1_i_2_WbMerges'] + cache_stats['l1_i_3_WbMerges']
	l1_i_MergeRetries = cache_stats['l1_i_0_MergeRetries'] + cache_stats['l1_i_1_MergeRetries'] + cache_stats['l1_i_2_MergeRetries'] + cache_stats['l1_i_3_MergeRetries']
	l1_i_WbRecieved = cache_stats['l1_i_0_WbRecieved'] + cache_stats['l1_i_1_WbRecieved'] + cache_stats['l1_i_2_WbRecieved'] + cache_stats['l1_i_3_WbRecieved']
	l1_i_WbSent = cache_stats['l1_i_0_WbSent'] + cache_stats['l1_i_1_WbSent'] + cache_stats['l1_i_2_WbSent'] + cache_stats['l1_i_3_WbSent']
	l1_i_SharingWbSent = cache_stats['l1_i_0_SharingWbSent'] + cache_stats['l1_i_1_SharingWbSent'] + cache_stats['l1_i_2_SharingWbSent'] + cache_stats['l1_i_3_SharingWbSent']
	l1_i_WbDropped = cache_stats['l1_i_0_WbDropped'] + cache_stats['l1_i_1_WbDropped'] + cache_stats['l1_i_2_WbDropped'] + cache_stats['l1_i_3_WbDropped']
	l1_i_UpgradeMisses = cache_stats['l1_i_0_UpgradeMisses'] + cache_stats['l1_i_1_UpgradeMisses'] + cache_stats['l1_i_2_UpgradeMisses'] + cache_stats['l1_i_3_UpgradeMisses']
	l1_i_UpgradeAcks = cache_stats['l1_i_0_TotalUpgradeAcks'] + cache_stats['l1_i_1_TotalUpgradeAcks'] + cache_stats['l1_i_2_TotalUpgradeAcks'] + cache_stats['l1_i_3_TotalUpgradeAcks']
	l1_i_Downgrades = cache_stats['l1_i_0_TotalDowngrades'] + cache_stats['l1_i_1_TotalDowngrades'] + cache_stats['l1_i_2_TotalDowngrades'] + cache_stats['l1_i_3_TotalDowngrades']
	l1_i_GetxFwdInval = cache_stats['l1_i_0_TotalGetxFwdInvals'] + cache_stats['l1_i_1_TotalGetxFwdInvals'] + cache_stats['l1_i_2_TotalGetxFwdInvals'] + cache_stats['l1_i_3_TotalGetxFwdInvals']
	l1_i_EvictInv = cache_stats['l1_i_0_EvictInv'] + cache_stats['l1_i_1_EvictInv'] + cache_stats['l1_i_2_EvictInv'] + cache_stats['l1_i_3_EvictInv']
	l1_i_UpgradeInvals = cache_stats['l1_i_0_TotalUpgradeInvals'] + cache_stats['l1_i_1_TotalUpgradeInvals'] + cache_stats['l1_i_2_TotalUpgradeInvals'] + cache_stats['l1_i_3_TotalUpgradeInvals']
	
	l1_i_AveCyclesPerAdvance = cache_stats['l1_i_0_AveCyclesPerAdvance'] + cache_stats['l1_i_1_AveCyclesPerAdvance'] + cache_stats['l1_i_2_AveCyclesPerAdvance'] + cache_stats['l1_i_3_AveCyclesPerAdvance']
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
	l1_d_Stalls = cache_stats['l1_d_0_Stalls'] + cache_stats['l1_d_1_Stalls'] + cache_stats['l1_d_2_Stalls'] + cache_stats['l1_d_3_Stalls']
	l1_d_WriteBlocks = cache_stats['l1_d_0_TotalWriteBlocks'] + cache_stats['l1_d_1_TotalWriteBlocks'] + cache_stats['l1_d_2_TotalWriteBlocks'] + cache_stats['l1_d_3_TotalWriteBlocks']
	l1_d_CoalescePut = cache_stats['l1_d_0_CoalescePut'] + cache_stats['l1_d_1_CoalescePut'] + cache_stats['l1_d_2_CoalescePut'] + cache_stats['l1_d_3_CoalescePut']
	l1_d_CoalesceGet = cache_stats['l1_d_0_CoalesceGet'] + cache_stats['l1_d_1_CoalesceGet'] + cache_stats['l1_d_2_CoalesceGet'] + cache_stats['l1_d_3_CoalesceGet']
	l1_d_WbMerges = cache_stats['l1_d_0_WbMerges'] + cache_stats['l1_d_1_WbMerges'] + cache_stats['l1_d_2_WbMerges'] + cache_stats['l1_d_3_WbMerges']
	l1_d_MergeRetries = cache_stats['l1_d_0_MergeRetries'] + cache_stats['l1_d_1_MergeRetries'] + cache_stats['l1_d_2_MergeRetries'] + cache_stats['l1_d_3_MergeRetries']
	l1_d_WbRecieved = cache_stats['l1_d_0_WbRecieved'] + cache_stats['l1_d_1_WbRecieved'] + cache_stats['l1_d_2_WbRecieved'] + cache_stats['l1_d_3_WbRecieved']
	l1_d_WbSent = cache_stats['l1_d_0_WbSent'] + cache_stats['l1_d_1_WbSent'] + cache_stats['l1_d_2_WbSent'] + cache_stats['l1_d_3_WbSent']
	l1_d_SharingWbSent = cache_stats['l1_d_0_SharingWbSent'] + cache_stats['l1_d_1_SharingWbSent'] + cache_stats['l1_d_2_SharingWbSent'] + cache_stats['l1_d_3_SharingWbSent']
	l1_d_WbDropped = cache_stats['l1_d_0_WbDropped'] + cache_stats['l1_d_1_WbDropped'] + cache_stats['l1_d_2_WbDropped'] + cache_stats['l1_d_3_WbDropped']
	l1_d_UpgradeMisses = cache_stats['l1_d_0_UpgradeMisses'] + cache_stats['l1_d_1_UpgradeMisses'] + cache_stats['l1_d_2_UpgradeMisses'] + cache_stats['l1_d_3_UpgradeMisses']
	l1_d_UpgradeAcks = cache_stats['l1_d_0_TotalUpgradeAcks'] + cache_stats['l1_d_1_TotalUpgradeAcks'] + cache_stats['l1_d_2_TotalUpgradeAcks'] + cache_stats['l1_d_3_TotalUpgradeAcks']
	l1_d_Downgrades = cache_stats['l1_d_0_TotalDowngrades'] + cache_stats['l1_d_1_TotalDowngrades'] + cache_stats['l1_d_2_TotalDowngrades'] + cache_stats['l1_d_3_TotalDowngrades']
	l1_d_GetxFwdInval = cache_stats['l1_d_0_TotalGetxFwdInvals'] + cache_stats['l1_d_1_TotalGetxFwdInvals'] + cache_stats['l1_d_2_TotalGetxFwdInvals'] + cache_stats['l1_d_3_TotalGetxFwdInvals']
	l1_d_EvictInv = cache_stats['l1_d_0_EvictInv'] + cache_stats['l1_d_1_EvictInv'] + cache_stats['l1_d_2_EvictInv'] + cache_stats['l1_d_3_EvictInv']	
	l1_d_UpgradeInvals = cache_stats['l1_d_0_TotalUpgradeInvals'] + cache_stats['l1_d_1_TotalUpgradeInvals'] + cache_stats['l1_d_2_TotalUpgradeInvals'] + cache_stats['l1_d_3_TotalUpgradeInvals']
	
	
	l1_d_AveCyclesPerAdvance = cache_stats['l1_d_0_AveCyclesPerAdvance'] + cache_stats['l1_d_1_AveCyclesPerAdvance'] + cache_stats['l1_d_2_AveCyclesPerAdvance'] + cache_stats['l1_d_3_AveCyclesPerAdvance']
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
	l2_Stalls = cache_stats['l2_0_Stalls'] + cache_stats['l2_1_Stalls'] + cache_stats['l2_2_Stalls'] + cache_stats['l2_3_Stalls']
	l2_WriteBlocks = cache_stats['l2_0_TotalWriteBlocks'] + cache_stats['l2_1_TotalWriteBlocks'] + cache_stats['l2_2_TotalWriteBlocks'] + cache_stats['l2_3_TotalWriteBlocks']	
	l2_CoalescePut = cache_stats['l2_0_CoalescePut'] + cache_stats['l2_1_CoalescePut'] + cache_stats['l2_2_CoalescePut'] + cache_stats['l2_3_CoalescePut']
	l2_CoalesceGet = cache_stats['l2_0_CoalesceGet'] + cache_stats['l2_1_CoalesceGet'] + cache_stats['l2_2_CoalesceGet'] + cache_stats['l2_3_CoalesceGet']
	l2_WbMerges = cache_stats['l2_0_WbMerges'] + cache_stats['l2_1_WbMerges'] + cache_stats['l2_2_WbMerges'] + cache_stats['l2_3_WbMerges']
	l2_MergeRetries = cache_stats['l2_0_MergeRetries'] + cache_stats['l2_1_MergeRetries'] + cache_stats['l2_2_MergeRetries'] + cache_stats['l2_3_MergeRetries']
	l2_WbRecieved = cache_stats['l2_0_WbRecieved'] + cache_stats['l2_1_WbRecieved'] + cache_stats['l2_2_WbRecieved'] + cache_stats['l2_3_WbRecieved']
	l2_WbSent = cache_stats['l2_0_WbSent'] + cache_stats['l2_1_WbSent'] + cache_stats['l2_2_WbSent'] + cache_stats['l2_3_WbSent']
	l2_SharingWbSent = cache_stats['l2_0_SharingWbSent'] + cache_stats['l2_1_SharingWbSent'] + cache_stats['l2_2_SharingWbSent'] + cache_stats['l2_3_SharingWbSent']
	l2_WbDropped = cache_stats['l2_0_WbDropped'] + cache_stats['l2_1_WbDropped'] + cache_stats['l2_2_WbDropped'] + cache_stats['l2_3_WbDropped']
	l2_UpgradeMisses = cache_stats['l2_0_UpgradeMisses'] + cache_stats['l2_1_UpgradeMisses'] + cache_stats['l2_2_UpgradeMisses'] + cache_stats['l2_3_UpgradeMisses']
	l2_UpgradeAcks = cache_stats['l2_0_TotalUpgradeAcks'] + cache_stats['l2_1_TotalUpgradeAcks'] + cache_stats['l2_2_TotalUpgradeAcks'] + cache_stats['l2_3_TotalUpgradeAcks']
	l2_Downgrades = 0
	l2_GetxFwdInval = 0
	l2_EvictInv = cache_stats['l2_0_EvictInv'] + cache_stats['l2_1_EvictInv'] + cache_stats['l2_2_EvictInv'] + cache_stats['l2_3_EvictInv']
	l2_UpgradeInvals = cache_stats['l2_0_TotalUpgradeInvals'] + cache_stats['l2_1_TotalUpgradeInvals'] + cache_stats['l2_2_TotalUpgradeInvals'] + cache_stats['l2_3_TotalUpgradeInvals']

	
	l2_AveCyclesPerAdvance = cache_stats['l2_0_AveCyclesPerAdvance'] + cache_stats['l2_1_AveCyclesPerAdvance'] + cache_stats['l2_2_AveCyclesPerAdvance'] + cache_stats['l2_3_AveCyclesPerAdvance']
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
	l3_Stalls = cache_stats['l3_0_Stalls'] + cache_stats['l3_1_Stalls'] + cache_stats['l3_2_Stalls'] + cache_stats['l3_3_Stalls']
	l3_WriteBlocks = cache_stats['l3_0_TotalWriteBlocks'] + cache_stats['l3_1_TotalWriteBlocks'] + cache_stats['l3_2_TotalWriteBlocks'] + cache_stats['l3_3_TotalWriteBlocks']	
	l3_CoalescePut = cache_stats['l3_0_CoalescePut'] + cache_stats['l3_1_CoalescePut'] + cache_stats['l3_2_CoalescePut'] + cache_stats['l3_3_CoalescePut']
	l3_CoalesceGet = cache_stats['l3_0_CoalesceGet'] + cache_stats['l3_1_CoalesceGet'] + cache_stats['l3_2_CoalesceGet'] + cache_stats['l3_3_CoalesceGet']
	l3_WbMerges = cache_stats['l3_0_WbMerges'] + cache_stats['l3_1_WbMerges'] + cache_stats['l3_2_WbMerges'] + cache_stats['l3_3_WbMerges']
	l3_MergeRetries = cache_stats['l3_0_MergeRetries'] + cache_stats['l3_1_MergeRetries'] + cache_stats['l3_2_MergeRetries'] + cache_stats['l3_3_MergeRetries']
	l3_WbRecieved = cache_stats['l3_0_WbRecieved'] + cache_stats['l3_1_WbRecieved'] + cache_stats['l3_2_WbRecieved'] + cache_stats['l3_3_WbRecieved']
	l3_WbSent = cache_stats['l3_0_WbSent'] + cache_stats['l3_1_WbSent'] + cache_stats['l3_2_WbSent'] + cache_stats['l3_3_WbSent']
	l3_SharingWbSent = cache_stats['l3_0_SharingWbSent'] + cache_stats['l3_1_SharingWbSent'] + cache_stats['l3_2_SharingWbSent'] + cache_stats['l3_3_SharingWbSent']
	l3_WbDropped = cache_stats['l3_0_WbDropped'] + cache_stats['l3_1_WbDropped'] + cache_stats['l3_2_WbDropped'] + cache_stats['l3_3_WbDropped']
	l3_UpgradeMisses = cache_stats['l3_0_UpgradeMisses'] + cache_stats['l3_1_UpgradeMisses'] + cache_stats['l3_2_UpgradeMisses'] + cache_stats['l3_3_UpgradeMisses']
	l3_UpgradeAcks = cache_stats['l3_0_TotalUpgradeAcks'] + cache_stats['l3_1_TotalUpgradeAcks'] + cache_stats['l3_2_TotalUpgradeAcks'] + cache_stats['l3_3_TotalUpgradeAcks']
	l3_Downgrades = 0
	l3_GetxFwdInval = 0
	l3_EvictInv = cache_stats['l3_0_EvictInv'] + cache_stats['l3_1_EvictInv'] + cache_stats['l3_2_EvictInv'] + cache_stats['l3_3_EvictInv']
	l3_UpgradeInvals = cache_stats['l3_0_TotalUpgradeInvals'] + cache_stats['l3_1_TotalUpgradeInvals'] + cache_stats['l3_2_TotalUpgradeInvals'] + cache_stats['l3_3_TotalUpgradeInvals']

	
	l3_AveCyclesPerAdvance = cache_stats['l3_0_AveCyclesPerAdvance'] + cache_stats['l3_1_AveCyclesPerAdvance'] + cache_stats['l3_2_AveCyclesPerAdvance'] + cache_stats['l3_3_AveCyclesPerAdvance']
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










ave_busy_time = (core_0_BusyTime + core_1_BusyTime + core_2_BusyTime + core_3_BusyTime)/int(options.NumCores)
	ave_SystemTime = (core_0_SystemTime + core_1_SystemTime + core_2_SystemTime + core_3_SystemTime)/int(options.NumCores)
	ave_FetchStall = (core_0_FetchStall + core_1_FetchStall + core_2_FetchStall + core_3_FetchStall)/int(options.NumCores)
	ave_ROBStallLoad = (core_0_ROBStallLoad + core_1_ROBStallLoad + core_2_ROBStallLoad + core_3_ROBStallLoad)/int(options.NumCores)
	ave_ROBStallStore = (core_0_ROBStallStore + core_1_ROBStallStore + core_2_ROBStallStore + core_3_ROBStallStore)/int(options.NumCores)
	ave_ROBStallOther = (core_0_ROBStallOther + core_1_ROBStallOther + core_2_ROBStallOther + core_3_ROBStallOther)/int(options.NumCores)
	ave_IdleTime = (core_0_IdleTime + core_1_IdleTime + core_2_IdleTime + core_3_IdleTime)/int(options.NumCores)

	table_P4 = [
		[ave_busy_time, ave_SystemTime, ave_FetchStall, ave_ROBStallLoad, ave_ROBStallStore, ave_ROBStallOther,]
		]
	
	
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
	table_P0 = [core_0_IdleTime, core_0_FetchStall, core_0_ROBStallLoad, core_0_ROBStallStore, core_0_ROBStallOther, core_0_SystemTime, core_0_BusyTime]

	
	#if int(options.OutChart) == 2:
	#ave_busy_time = (core_0_BusyTime + core_1_BusyTime + core_2_BusyTime + core_3_BusyTime)/int(options.NumCores)
	#ave_SystemTime = (core_0_SystemTime + core_1_SystemTime + core_2_SystemTime + core_3_SystemTime)/int(options.NumCores)
	#ave_FetchStall = (core_0_FetchStall + core_1_FetchStall + core_2_FetchStall + core_3_FetchStall)/int(options.NumCores)
	#ave_ROBStallLoad = (core_0_ROBStallLoad + core_1_ROBStallLoad + core_2_ROBStallLoad + core_3_ROBStallLoad)/int(options.NumCores)
	#ave_ROBStallStore = (core_0_ROBStallStore + core_1_ROBStallStore + core_2_ROBStallStore + core_3_ROBStallStore)/int(options.NumCores)
	#ave_ROBStallOther = (core_0_ROBStallOther + core_1_ROBStallOther + core_2_ROBStallOther + core_3_ROBStallOther)/int(options.NumCores)
	#ave_IdleTime = (core_0_IdleTime + core_1_IdleTime + core_2_IdleTime + core_3_IdleTime)/int(options.NumCores)

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
	table_P0 = [core_0_IdleTime, core_0_FetchStall, core_0_ROBStallLoad, core_0_ROBStallStore, core_0_ROBStallOther, core_0_SystemTime, core_0_BusyTime]

	
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
	table_P1 = [core_1_IdleTime, core_1_FetchStall, core_1_ROBStallLoad, core_1_ROBStallStore, core_1_ROBStallOther, core_1_SystemTime, core_1_BusyTime]

	
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
	table_P2 = [core_2_IdleTime, core_2_FetchStall, core_2_ROBStallLoad, core_2_ROBStallStore, core_2_ROBStallOther, core_2_SystemTime, core_2_BusyTime]

	
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
	table_P3 = [core_3_IdleTime, core_3_FetchStall, core_3_ROBStallLoad, core_3_ROBStallStore, core_3_ROBStallOther, core_3_SystemTime, core_3_BusyTime]

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

###print stats combined 4 cores###
	if int(options.NumCores) == 4:


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
