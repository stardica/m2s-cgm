#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser

num_cores = 2
cache_levels = 3
gpu_stats = 1

def get_stats(options):

	stats = ConfigParser.ConfigParser()
	stats.optionxform = str
	stats.read(options.InFileName)

	#pull stats
	if options.PrintSection == 'FullRunStats':
		stats_dict = dict(stats.items('FullRunStats'))
	elif options.PrintSection == 'ParallelStats':
		stats_dict = dict(stats.items('ParallelStats'))
	else:
		print "print_switch_io_stats(): invalid section"
		exit(0)

	for key, value in stats_dict.items(): # get the (key, value) tuples one at a time
		try:
			stats_dict[key] = int(value)
		except ValueError:
			stats_dict[key] = float(value)

	return stats_dict


def get_margins(table, title, element):

	#combined swtich stats

	#get the largest title length	
	max_title_length = len(title)
	current_title_length = 0

	for tup in table:
		for item in tup[0:1]:
			current_title_length = len(tup[0])
			if max_title_length < current_title_length:
				max_title_length = current_title_length

	#get the largest data element length
	max_element_length = len(element)
	current_element_length = 0

	for tup in table:
		for item in tup[1:5]:
			current_element_length = len(str(item))
			if max_element_length < current_element_length:
				max_element_length = current_element_length

	max_title_length += 2
	max_element_length += 2
	#print "max title {} max data {}".format(max_title_length, max_element_length)

	return max_title_length, max_element_length



def print_switch_io_stats(options):
	
	io_stats_dict = get_stats(options)
	
	
	var = ""
	io_stats = 	[
			"IONorthOccupancy",
			"IONorthOccupancyPct",
			"IOEastOccupancy",
			"IOEastOccupancyPct",
			"IOSouthOccupancy",
			"IOSouthOccupancyPct",
			"IOWestOccupancy",
			"IOWestOccupancyPct"
			]
	
	
	switch_io_stats_table = [[0 for x in range(num_cores + 2)] for y in range(len(io_stats))]

	
	for i in range (0,len(io_stats)):
		switch_io_stats_table[i][0] = io_stats[i]
		for j in range (0,(num_cores+1)):
			var = "s_" + str(j) + "_" + io_stats[i]
			switch_io_stats_table[i][(j+1)] = io_stats_dict[var]

	switch_stats_table_combined = [[0 for x in range(2)] for y in range(len(io_stats))]
	
	#need to account for ave stats here...
	for i in range (0,len(io_stats)):
		switch_stats_table_combined[i][0] = io_stats[i]
		if switch_stats_table_combined[i][0] == "IONorthOccupancyPct" or switch_stats_table_combined[i][0] == "IOEastOccupancyPct" \
		or switch_stats_table_combined[i][0] == "IOSouthOccupancyPct" or switch_stats_table_combined[i][0] == "IOWestOccupancyPct":
			switch_stats_table_combined[i][1] = float(switch_stats_table_combined[i-1][1])/float(total_paralell_cycles)
		else:
			for j in range (1, (num_cores + 2)):
				switch_stats_table_combined[i][1] += switch_io_stats_table[i][j]
			switch_stats_table_combined[i][1] = switch_stats_table_combined[i][1]/(num_cores + 1)
		
	#print switch_stats_table_combined
	

	f = open(options.OutFileName, 'a')
	f.write('//Switch IO Stats//////////////////////////////////////////////' +'\n')
	f.write('///////////////////////////////////////////////////////////////'  + '\n\n')	
	

	#combined swtich stats
	max_title_length, max_element_length = get_margins(switch_stats_table_combined, 'Switch IO stats combined', "")

	
	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("Switch IO stats combined",'SW', title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in switch_stats_table_combined:
			if isinstance(tup[1], int):
				f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
			else:
				f.write("{:<{title_width}s}{:>{data_width}.3f}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write('\n')

	
	#individual swtich stats
	max_title_length, max_element_length = get_margins(switch_io_stats_table, 'Switch IO stats individual', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the table headers
	f.write("{:<{title_width}}".format("Switch IO stats individual", title_width=max_title_length))

	for i in range(0,num_cores + 1):
		var = "S_" + str(i)
		f.write("{:>{data_width}}".format(var, data_width=max_element_length))

	f.write('\n')

	
	#print the bars
	f.write("{:<{title_width}}".format(title_bar, title_width=max_title_length))

	for i in range(0,num_cores + 1):
		f.write("{:>{data_width}}".format(data_bar, data_width=max_element_length))

	f.write('\n')

	#print the stats
	for tup in switch_io_stats_table:
		f.write("{:<{title_width}}".format(tup[0], title_width=max_title_length))
		for i in range(0,num_cores+1):
			if isinstance(tup[1], int):
				f.write("{:>{data_width}}".format(tup[i+1], data_width=max_element_length))	
			else: 
				f.write("{:>{data_width}.3f}".format(tup[i+1], data_width=max_element_length))	

		f.write('\n')
	f.write('\n')

	f.close()

	return


def print_cache_io_stats(options):

	io_stats_dict = get_stats(options)

	var = ""
	io_stats = 	[
			"IOUpOccupancy",
			"IOUpOccupancyPct",
			"IODownOccupancy",
			"IOdownOccupancyPct"
			]


	l1_i_io_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(io_stats))]
	l1_d_io_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(io_stats))]
	l2_io_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(io_stats))]
	l3_io_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(io_stats))]

	
	for i in range (0,len(io_stats)):
		l1_i_io_stats_table[i][0] = io_stats[i]
		for j in range (0,num_cores):
			var = "l1_i_" + str(j) + "_" + io_stats[i]
			try:
				l1_i_io_stats_table[i][(j+1)] = io_stats_dict[var]
			except:
				l1_i_io_stats_table[i][(j+1)] = 0
	
	for i in range (0,len(io_stats)):
		l1_d_io_stats_table[i][0] = io_stats[i]
		for j in range (0,num_cores):
			var = "l1_d_" + str(j) + "_" + io_stats[i]
			try:
				l1_d_io_stats_table[i][(j+1)] = io_stats_dict[var]
			except:
				l1_d_io_stats_table[i][(j+1)] = 0

	for i in range (0,len(io_stats)):
		l2_io_stats_table[i][0] = io_stats[i]
		for j in range (0,num_cores):
			var = "l2_" + str(j) + "_" + io_stats[i]
			l2_io_stats_table[i][(j+1)] = io_stats_dict[var]
			
	for i in range (0,len(io_stats)):
		l3_io_stats_table[i][0] = io_stats[i]
		for j in range (0,num_cores):
			var = "l3_" + str(j) + "_" + io_stats[i]
			l3_io_stats_table[i][(j+1)] = io_stats_dict[var]


	cache_stats_table_combined = [[0 for x in range(cache_levels + 2)] for y in range(len(io_stats))]
	
		#need to account for ave stats here...
	for i in range (0,len(io_stats)):
		cache_stats_table_combined[i][0] = io_stats[i]
		if cache_stats_table_combined[i][0] == "IOUpOccupancyPct" or cache_stats_table_combined[i][0] == "IOdownOccupancyPct":
			cache_stats_table_combined[i][1] = float(cache_stats_table_combined[i-1][1]) / float(total_paralell_cycles)
			cache_stats_table_combined[i][2] = float(cache_stats_table_combined[i-1][2]) / float(total_paralell_cycles)
			cache_stats_table_combined[i][3] = float(cache_stats_table_combined[i-1][3]) / float(total_paralell_cycles)
			cache_stats_table_combined[i][4] = float(cache_stats_table_combined[i-1][4]) / float(total_paralell_cycles)
		else:
			for j in range (1, num_cores + 1):
				cache_stats_table_combined[i][1] += l1_i_io_stats_table[i][j]
				cache_stats_table_combined[i][2] += l1_d_io_stats_table[i][j]
				cache_stats_table_combined[i][3] += l2_io_stats_table[i][j]
				cache_stats_table_combined[i][4] += l3_io_stats_table[i][j]
			cache_stats_table_combined[i][1] = cache_stats_table_combined[i][1]/num_cores
			cache_stats_table_combined[i][2] = cache_stats_table_combined[i][2]/num_cores
			cache_stats_table_combined[i][3] = cache_stats_table_combined[i][3]/num_cores
			cache_stats_table_combined[i][4] = cache_stats_table_combined[i][4]/num_cores


	f = open(options.OutFileName, 'a')

	f.write("//Cache IO Stats///////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')

	#combined swtich stats
	max_title_length, max_element_length = get_margins(cache_stats_table_combined, 'Cache io stats combined', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format("Cache stats combined",'I$', 'D$', 'L2$', 'L3$', title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in cache_stats_table_combined:
		if tup[0] == "IOUpOccupancyPct" or tup[0] == "IOdownOccupancyPct":
			f.write("{:<{title_width}s}{:>{data_width}.3f}{:>{data_width}.3f}{:>{data_width}.3f}{:>{data_width}.3f}".format(tup[0], (tup[1]), (tup[2]), (tup[3]), (tup[4]), title_width=max_title_length, data_width=max_element_length) + '\n')
		else:			
			if isinstance(tup[1], int):
				f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')
			else:
				f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')
	
	f.write('\n')


	#take our 4 tables and make them one big table...
	io_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(io_stats) * 4)]

	j = 0	
	for i in range(0, len(io_stats)):
		io_stats_table[j] = l1_i_io_stats_table[i]
		io_stats_table[j+1] = l1_d_io_stats_table[i]
		io_stats_table[j+2] = l2_io_stats_table[i]
		io_stats_table[j+3] = l3_io_stats_table[i]
		j += 4


	#combined swtich stats
	max_title_length, max_element_length = get_margins(io_stats_table, 'Cache IO stats individual', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)


	for j in ("I", "D", "L2", "L3"):
		#print the table headers
		f.write("{:<{title_width}}".format("Cache IO stats individual", title_width=max_title_length))

		for i in range(0,num_cores):
			var = j + "_" + str(i)
			f.write("{:>{data_width}}".format(var, data_width=max_element_length))

		f.write('\n')


		#print the bars
		f.write("{:<{title_width}}".format(title_bar, title_width=max_title_length))

		for i in range(0,num_cores):
			f.write("{:>{data_width}}".format(data_bar, data_width=max_element_length))

		f.write('\n')

		if j == "I":
			print_table = l1_i_io_stats_table
		elif j == "D":
			print_table = l1_d_io_stats_table
		elif j == "L2":
			print_table = l2_io_stats_table
		else:
			print_table = l3_io_stats_table

		
		#print the stats
		for tup in print_table:
			f.write("{:<{title_width}}".format(tup[0], title_width=max_title_length))
			if isinstance(tup[1], int):
				for i in range(0,num_cores):
					f.write("{:>{data_width}}".format(tup[i+1], data_width=max_element_length))	
			else:
				for i in range(0,num_cores):
					f.write("{:>{data_width}.3f}".format(tup[i+1], data_width=max_element_length))	

			f.write('\n')
		f.write('\n')
	f.write('\n')

	
	f.close()

	return



def print_cache_stats(options):

	cache_stats_dict = io_stats_dict = get_stats(options)

	var = ""
	cache_stats = [	
			"CacheUtilization",
			"Occupancy",
			"OccupancyPct"
			#"Stalls",
			#"TotalWriteBlocks",
			#"CoalescePut",
			#"CoalesceGet",
			#"WbMerges",
			#"MergeRetries",
			#"WbRecieved",
			#"WbSent",
			#"SharingWbSent",
			#"WbDropped",
			#"UpgradeMisses",
			#"TotalUpgrades",
			#"TotalUpgradeAcks",
			#"TotalUpgradeInvals",
			#"TotalDowngrades",
			#"TotalGetxFwdInvals",
			#"EvictInv",
			#"TotalUpgradeInvals",
			#"AveCyclesPerAdvance",
			#"TotalAdvances",
			#"TotalAccesses",
			#"TotalHits",
			#"TotalMisses",
			#"MissRate",
			#"TotalReads",
			#"TotalReadMisses",
			#"ReadMissRate",
			#"TotalWrites",
			#"TotalWriteMisses",
			#"WriteMissRate",
			#"TotalGet",
			#"TotalGet",
			#"TotalGetx",
			#"GetMissRate",
			#"GetMissRate",
			#"GetxMissRate",
			#"UpgradeMissRate",
			#"TotalWriteBacks"
			]

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
	
	cache_stats_table_combined = [[0 for x in range(cache_levels + 2)] for y in range(len(cache_stats))]
	
		#need to account for ave stats here...
	for i in range (0,len(cache_stats)):
		cache_stats_table_combined[i][0] = cache_stats[i]
		if cache_stats_table_combined[i][0] == "OccupancyPct":
			cache_stats_table_combined[i][1] = float(cache_stats_table_combined[1][1]) / float(total_paralell_cycles)
			cache_stats_table_combined[i][2] = float(cache_stats_table_combined[1][2]) / float(total_paralell_cycles)
			cache_stats_table_combined[i][3] = float(cache_stats_table_combined[1][3]) / float(total_paralell_cycles)
			cache_stats_table_combined[i][4] = float(cache_stats_table_combined[1][4]) / float(total_paralell_cycles)
		else:
			for j in range (1, num_cores + 1):
				cache_stats_table_combined[i][1] += l1_i_stats_table[i][j]
				cache_stats_table_combined[i][2] += l1_d_stats_table[i][j]
				cache_stats_table_combined[i][3] += l2_stats_table[i][j]
				cache_stats_table_combined[i][4] += l3_stats_table[i][j]
			cache_stats_table_combined[i][1] = cache_stats_table_combined[i][1]/num_cores
			cache_stats_table_combined[i][2] = cache_stats_table_combined[i][2]/num_cores
			cache_stats_table_combined[i][3] = cache_stats_table_combined[i][3]/num_cores
			cache_stats_table_combined[i][4] = cache_stats_table_combined[i][4]/num_cores

	#print cache_stats_table_combined[2]
	#print l1_i_stats_table[1]
	#print l1_d_stats_table[1]
	#print l2_stats_table[1]
	#print l3_stats_table[1]

	f = open(options.OutFileName, 'a')

	f.write("//Cache Stats//////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')


	#combined swtich stats
	max_title_length, max_element_length = get_margins(cache_stats_table_combined, 'Cache stats combined', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format("Cache stats combined",'I$', 'D$', 'L2$', 'L3$', title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write("{:<{title_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(title_bar, data_bar, data_bar, data_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in cache_stats_table_combined:
		if tup[0] == "CacheUtilization":
			f.write("{:<{title_width}s}{:>{data_width}.2f}{:>{data_width}.2f}{:>{data_width}.2f}{:>{data_width}.2f}".format(tup[0], (tup[1]/8), (tup[2]/8), (tup[3]/8), (tup[4]/8), title_width=max_title_length, data_width=max_element_length) + '\n')
		elif tup[0] == "OccupancyPct":
			f.write("{:<{title_width}s}{:>{data_width}.3f}{:>{data_width}.3f}{:>{data_width}.3f}{:>{data_width}.3f}".format(tup[0], (tup[1]), (tup[2]), (tup[3]), (tup[4]), title_width=max_title_length, data_width=max_element_length) + '\n')
		else:
			if isinstance(tup[1], int):
				f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')
			else:
				f.write("{:<{title_width}s}{:>{data_width}}{:>{data_width}}{:>{data_width}}{:>{data_width}}".format(tup[0], tup[1], tup[2], tup[3], tup[4], title_width=max_title_length, data_width=max_element_length) + '\n')
	
	f.write('\n')

	#take our 4 tables and make them one big table...
	cache_stats_table = [[0 for x in range(num_cores + 1)] for y in range(len(cache_stats) * 4)]

	j = 0	
	for i in range(0, len(cache_stats)):
		cache_stats_table[j] = l1_i_stats_table[i]
		cache_stats_table[j+1] = l1_d_stats_table[i]
		cache_stats_table[j+2] = l2_stats_table[i]
		cache_stats_table[j+3] = l3_stats_table[i]
		j += 4


	max_title_length, max_element_length = get_margins(cache_stats_table, 'Cache stats individual', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)


	for j in ("I", "D", "L2", "L3"):

		#print the table headers
		f.write("{:<{title_width}}".format("Cache stats individual", title_width=max_title_length))

		for i in range(0,num_cores):
			var = j + "_" + str(i)
			f.write("{:>{data_width}}".format(var, data_width=max_element_length))

		f.write('\n')


		#print the bars
		f.write("{:<{title_width}}".format(title_bar, title_width=max_title_length))

		for i in range(0,num_cores):
			f.write("{:>{data_width}}".format(data_bar, data_width=max_element_length))

		f.write('\n')

		if j == "I":
			print_table = l1_i_stats_table
		elif j == "D":
			print_table = l1_d_stats_table
		elif j == "L2":
			print_table = l2_stats_table
		else:
			print_table = l3_stats_table

		#print the stats
		for tup in print_table:
			f.write("{:<{title_width}}".format(tup[0], title_width=max_title_length))
			if isinstance(tup[1], int):
				for i in range(0,num_cores):
					f.write("{:>{data_width}}".format(tup[i+1], data_width=max_element_length))	
			else:
				for i in range(0,num_cores):
					f.write("{:>{data_width}.3f}".format(tup[i+1], data_width=max_element_length))	

			f.write('\n')
		f.write('\n')
	f.write('\n')
	
	f.close()

	return


def print_switch_stats(options):

	switch_stats_dict = io_stats_dict = get_stats(options)
	
	var = ""
	switch_stats = [
			"Occupancy",
			"OccupancyPct"
			#"NumberLinks",
			#"MaxNumberLinks",
			#"AveNumberLinksPerAccess",
			#"NorthIOTransfers",
			#"NorthIOCycles",
			#"NorthIOBytesTransfered",
			#"NorthRxQueueMaxDepth",
			#"NorthRxQueueAveDepth",
			#"NorthTxQueueMaxDepth",
			#"NorthTxQueueAveDepth",
			#"EastIOTransfers",
			#"EastIOCycles",
			#"EastIOBytesTransfered",
			#"EastRxQueueMaxDepth",
			#"EastRxQueueAveDepth",
			#"EastTxQueueMaxDepth",
			#"EastTxQueueAveDepth",
			#"SouthIOTransfers",
			#"SouthIOCycles",
			#"SouthIOBytesTransfered",
			#"SouthRxQueueMaxDepth",
			#"SouthRxQueueAveDepth",
			#"SouthTxQueueMaxDepth",
			#"SouthTxQueueAveDepth",
			#"WestIOTransfers",
			#"WestIOCycles",
			#"WestIOBytesTransfered",
			#"WestRxQueueMaxDepth",
			#"WestRxQueueAveDepth",
			#"WestTxQueueMaxDepth",
			#"WestTxQueueAveDepth"
			]

	switch_stats_table = [[0 for x in range(num_cores + 2)] for y in range(len(switch_stats))]
		
	for i in range (0,len(switch_stats)):
		switch_stats_table[i][0] = switch_stats[i]
		for j in range (0,num_cores + 1):
			var = "s_" + str(j) + "_" + switch_stats[i]
			switch_stats_table[i][(j+1)] = switch_stats_dict[var]


	#for a combined switch stat
	switch_stats_table_combined = [[0 for x in range(2)] for y in range(len(switch_stats))]
	
	#need to account for ave stats here...
	for i in range (0,len(switch_stats)):
		switch_stats_table_combined[i][0] = switch_stats[i]
		if switch_stats_table_combined[i][0] == "OccupancyPct":
			switch_stats_table_combined[i][1] = float(switch_stats_table_combined[0][1]) / float(total_paralell_cycles)
		else:
			for j in range (1, num_cores + 2):
				switch_stats_table_combined[i][1] += switch_stats_table[i][j]
			switch_stats_table_combined[i][1] = switch_stats_table_combined[i][1]/(num_cores + 1)

	f = open(options.OutFileName, 'a')
	f.write('//Switch Stats/////////////////////////////////////////////////' +'\n')
	f.write('///////////////////////////////////////////////////////////////'  + '\n\n')	
	

	#combined swtich stats
	max_title_length, max_element_length = get_margins(switch_stats_table_combined, 'Switch stats combined', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("Switch stats combined",'SW', title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in switch_stats_table_combined:
			if isinstance(tup[1], int):
				f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
			else:
				f.write("{:<{title_width}s}{:>{data_width}.3f}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write('\n')


	#combined swtich stats
	max_title_length, max_element_length = get_margins(switch_stats_table, 'Switch stats individual', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)


	#print the table headers
	f.write("{:<{title_width}}".format("Switch stats individual", title_width=max_title_length))

	for i in range(0,num_cores + 1):
		var = "S_" + str(i)
		f.write("{:>{data_width}}".format(var, data_width=max_element_length))

	f.write('\n')

	
	#print the bars
	f.write("{:<{title_width}}".format(title_bar, title_width=max_title_length))

	for i in range(0,num_cores + 1):
		f.write("{:>{data_width}}".format(data_bar, data_width=max_element_length))

	f.write('\n')

	#print the stats
	for tup in switch_stats_table:
		f.write("{:<{title_width}}".format(tup[0], title_width=max_title_length))
		for i in range(0,num_cores+1):
			if isinstance(tup[1], int):
				f.write("{:>{data_width}}".format(tup[i+1], data_width=max_element_length))	
			else: 
				f.write("{:>{data_width}.3f}".format(tup[i+1], data_width=max_element_length))	

		f.write('\n')
	f.write('\n')

	f.close
	return


def print_samc_stats(options):

	samc_stats_dict = io_stats_dict = get_stats(options)

	sa_stats = [
			"Occupancy",
			"OccupancyPct",
			"IOUpOccupancy",
			"IOUpOccupancyPct",
			"IODownOccupancy",
			"IODownOccupancyPct"
			#"TotalLoads",
			#"TotalStores",
			#"TotalReturns",
			#"NorthIOCycles",
			#"NorthMaxRxQueueDepth",
			#"NorthAveRxQueueDepth",
			#"NorthMaxTxQueueDepth",
			#"NorthAveTxQueueDepth",
			#"SouthIOCycles",
			#"SouthMaxRxQueueDepth",
			#"SouthAveRxQueueDepth",
			#"SouthMaxTxQueueDepth",
			#"SouthAveTxQueueDepth",
			#"DramBusyCycles",
			#"DramAveReadLat",
			#"DramAveWriteLat",
			#"DramAveTotalLat(cyc)",
			#"DramAveTotalLat(ns)",
			#"DramReadMinLat",
			#"DramReadMaxLat",
			#"DramWriteMinLat",
			#"DramWriteMaxLat",
			#"DramMaxQueueDepth",
			#"DramAveQueueDepth",
			#"DramTotalBytesRead",
			#"DramTotalBytesWritten",
			]

	mc_stats = [
			"Occupancy",
			"OccupancyPct",
			"IOUpOccupancy",
			"IOUpOccupancyPct"
			#"TotalCtrlLoops",
			#"TotalLoads",
			#"TotalStores",
			#"TotalReturns",
			#"NorthIOCycles",
			#"NorthMaxRxQueueDepth",
			#"NorthAveRxQueueDepth",
			#"NorthMaxTxQueueDepth",
			#"NorthAveTxQueueDepth",
			#"SouthIOCycles",
			#"SouthMaxRxQueueDepth",
			#"SouthAveRxQueueDepth",
			#"SouthMaxTxQueueDepth",
			#"SouthAveTxQueueDepth",
			#"DramBusyCycles",
			#"DramAveReadLat",
			#"DramAveWriteLat",
			#"DramAveTotalLat(cyc)",
			#"DramAveTotalLat(ns)",
			#"DramReadMinLat",
			#"DramReadMaxLat",
			#"DramWriteMinLat",
			#"DramWriteMaxLat",
			#"DramMaxQueueDepth",
			#"DramAveQueueDepth",
			#"DramTotalBytesRead",
			#"DramTotalBytesWritten",
			]



	sa_stats_table = [[0 for x in range(2)] for y in range(len(sa_stats))]
	mc_stats_table = [[0 for x in range(2)] for y in range(len(mc_stats))]

	var = ""
	for i in range (0,len(sa_stats)):
		sa_stats_table[i][0] = sa_stats[i]
		var = "sa_" + sa_stats[i]
		sa_stats_table[i][1] = samc_stats_dict[var]

	for i in range (0,len(mc_stats)):
		mc_stats_table[i][0] = mc_stats[i]
		var = "mc_" + mc_stats[i]
		mc_stats_table[i][1] = samc_stats_dict[var]


	f = open(options.OutFileName, 'a')

	f.write("//System Agent Stats///////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')
		

	#combined swtich stats
	max_title_length, max_element_length = get_margins(sa_stats_table, 'System agent stats', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("System agent stats",'SA', title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in sa_stats_table:
		if isinstance(tup[1], int):
			f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
		else:
			f.write("{:<{title_width}s}{:>{data_width}.3f}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write('\n')



	f.write("//Memory Ctrl Stats////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')
	
	
	#combined swtich stats
	max_title_length, max_element_length = get_margins(mc_stats_table, 'Memory ctrl stats', "")
	
	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("Memory ctrl stats",'MC', title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in mc_stats_table:
		if isinstance(tup[1], int):
			f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
		else:
			f.write("{:<{title_width}s}{:>{data_width}.3f}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write('\n')

	f.close
	return


def print_mem_system_stats(options):

	ms_stats = get_stats(options)

	var = ""
	mem_stats = ["FirstAccessLat(Fetch)",
			"TotalCPUFetchRequests",
			"TotalCPUFetchReplys",
			"L1FetchHits",
			"L2TotalFetches",
			"L2FetchHits",
			"L3TotalFetches",
			"L3FetchHits",
			"FetchesMemory",
			"TotalCPULoadRequests",
			"TotalCPULoadReplys",
			"L1LoadHits",
			"L2TotalLoads",
			"L2LoadHits",
			"L3TotalLoads",
			"L3LoadHits",
			"LoadsMemory",
			"LoadsGetFwd",
			"l2_LoadNacks",
			"l3_LoadNacks",
			"TotalCPUStoreRequests",
			"TotalCPUStoreReplys",
			"L1StoreHits",
			"L2TotalStores",
			"L2StoreHits",
			"L3TotalStores",
			"L3StoreHits",
			"StoresMemory",
			"StoresGetxFwd",
			"StoresUpgrade",
			"l2_StoreNacks",
			"l3_StoreNacks"]

	mem_sys_stats = [[0 for x in range(2)] for y in range(len(mem_stats))]
	
	for i in range (0,len(mem_stats)):
		mem_sys_stats[i][0] = mem_stats[i]
		var = "MemSystem_" + mem_stats[i]
		mem_sys_stats[i][1] = ms_stats[var]

	
	f = open(options.OutFileName, 'a')

	f.write("//Mem-System Stats/////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')

	#combined swtich stats
	max_title_length, max_element_length = get_margins(mem_sys_stats, 'Stat Mem System', "")

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("Stat Mem System",'Stats', title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in mem_sys_stats:
		f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write('\n')
	f.close

	return


def print_gpu_stats(options):

	gpu_stats = get_stats(options)

	var = ""
	stats = [
		"GPUTime",
		"SCTime",
		"SCROBStalls"
		]

	stat_table = [[0 for x in range(2)] for y in range(len(stats))]
	
	for i in range (0,len(stats)):
		stat_table[i][0] = stats[i]
		for j in range (0, 1):
			var = "GPU_" + stats[i]
			stat_table[i][(j+1)] = gpu_stats[var]

	


	#stats_table_combined = [[0 for x in range(2)] for y in range(len(stats))]

	#need to account for ave stats here...
	#for i in range (0,len(stats)):
	#	stats_table_combined[i][0] = stats[i]
	#	for j in range (1, (num_cores + 1)):
	#		stats_table_combined[i][1] += stat_table[i][j]
	#	#stats_table_combined[i][1] = float(stats_table_combined[i][1])/float(num_cores)
	#	stats_table_combined[i][1] = stats_table_combined[i][1]/num_cores

	f = open(options.OutFileName, 'a')

	f.write("//GPU Stats////////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')


	max_title_length, max_element_length = get_margins(stat_table, "GPU stats", "GPU")

	
	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format('GPU stats', "GPU", title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in stat_table:
			if isinstance(tup[1], int):
				f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
			else:
				f.write("{:<{title_width}s}{:>{data_width}.3f}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write('\n')

	return



def print_cpu_stats(options):

	cpu_stats = get_stats(options)

	var = ""
	stats = [
		"TotalBusy",
		"DrainTime",

		"TotalStalls",
		"StallFetch",
		
		"ROBStalls",
		"ROBStallLoad",
		"ROBStallStore",
		"ROBStallOther",
		
		"LSQStalls",
		"LSQStallLoad",
		"LSQStallStore",

		"IQStalls",

		"RenameStalls",
	
		"NumSyscalls",
		"StallSyscall",
		#"FirstFetchCycle",
		#"LastCommitCycle",
		#"RunTime",
		#"IdleTime",
		"SystemTime",
		"StallTime",
		"BusyTime",
		"SystemTimePct",
		"StallTimePct",
		"BusyTimePct"
		#"IdlePct",
		#"RunPct",
		#"StallfetchPct",
		#"StallLoadPct",
		#"StallStorePct",
		#"StallOtherPct"
		]

	stat_table = [[0 for x in range(num_cores + 1)] for y in range(len(stats))]
	
	
	for i in range (0,len(stats)):
		stat_table[i][0] = stats[i]
		for j in range (0,num_cores):
			var = "core_" + str(j) + "_" + stats[i]
			stat_table[i][(j+1)] = cpu_stats[var]

	


	stats_table_combined = [[0 for x in range(2)] for y in range(len(stats))]

	#need to account for ave stats here...
	for i in range (0,len(stats)):
		stats_table_combined[i][0] = stats[i]
		for j in range (1, (num_cores + 1)):
			stats_table_combined[i][1] += stat_table[i][j]
		#stats_table_combined[i][1] = float(stats_table_combined[i][1])/float(num_cores)
		stats_table_combined[i][1] = stats_table_combined[i][1]/num_cores

	f = open(options.OutFileName, 'a')

	f.write("//CPU Stats////////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')


	#combined swtich stats
	max_title_length, max_element_length = get_margins(stats_table_combined, 'CPU stats combined', "CPU")

	
	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("CPU stats combined",'CPU', title_width=max_title_length, data_width=max_element_length) + '\n')

	f.write("{:<{title_width}}{:>{data_width}}".format(title_bar, data_bar, title_width=max_title_length, data_width=max_element_length) + '\n')

	#print the table's data
	for tup in stats_table_combined:
			if isinstance(tup[1], int):
				f.write("{:<{title_width}s}{:>{data_width}}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
			else:
				f.write("{:<{title_width}s}{:>{data_width}.3f}".format(tup[0], tup[1], title_width=max_title_length, data_width=max_element_length) + '\n')
	f.write('\n')



	#Individual CPU stats
	max_title_length, max_element_length = get_margins(stat_table, 'CPU Stats All Cores', "Core_0")

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
	
	#pull off the general stats 	
	general_stats = dict(general_data.items('General'))

	table_general_data = [
	["Benchmark", general_stats['Benchmark']],
	["BenchmarkArgs", general_stats['Args']],
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
	["GPUFreqGhz", general_stats['GPU_FreqGHz']],
	["MemLatFactor", general_stats['Mem_LatFactor']]
	]

	#for other stats...

	global total_paralell_cycles 

	total_paralell_cycles = general_stats['ParallelSectionCycles']
		
	f = open(options.OutFileName, 'w')

	f.write("//General Stats////////////////////////////////////////////////" + '\n')
	f.write("///////////////////////////////////////////////////////////////"  + '\n\n')

	#combined swtich stats
	max_title_length, max_element_length = get_margins(table_general_data, 'General Stats', 'Stats')

	title_bar = '-' * (max_title_length - 1)
	data_bar = '-' * (max_element_length - 1)

	#print the title and bars
	f.write("{:<{title_width}}{:>{data_width}}".format("General Stats","Stats", title_width=max_title_length, data_width=max_element_length) + '\n')
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
print_gpu_stats(options)
print_cache_stats(options)
print_cache_io_stats(options)
#print_mem_system_stats(options)
print_switch_stats(options)
print_switch_io_stats(options)
print_samc_stats(options)
