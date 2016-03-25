#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser
import matplotlib
import matplotlib.pyplot as plt
plt.style.use('ggplot')
import pandas as pd
import numpy as np


def plot_stats(options):

	if int(options.NumCores) == 1:
		print "No point in running this with less than 4 cores"	
		exit(0)

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
		table_P0 = [core_0_IdleTime, core_0_FetchStall, core_0_ROBStallLoad, core_0_ROBStallStore, core_0_ROBStallOther, core_0_SystemTime, core_0_BusyTime]

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
		table_P1 = [core_1_IdleTime, core_1_FetchStall, core_1_ROBStallLoad, core_1_ROBStallStore, core_1_ROBStallOther, core_1_SystemTime, core_1_BusyTime]

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
		table_P2 = [core_2_IdleTime, core_2_FetchStall, core_2_ROBStallLoad, core_2_ROBStallStore, core_2_ROBStallOther, core_2_SystemTime, core_2_BusyTime]

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
		table_P3 = [core_3_IdleTime, core_3_FetchStall, core_3_ROBStallLoad, core_3_ROBStallStore, core_3_ROBStallOther, core_3_SystemTime, core_3_BusyTime]

	#["ROBStalls", core_0_ROBStalls], 
	#["ROBStallLoad", core_0_ROBStallLoad],
	#["ROBStallStore", core_0_ROBStallStore],
	#["ROBStallOther", core_0_ROBStallOther],
	#["FetchStall", core_0_FetchStall],

	#["IdleTime", core_0_IdleTime, core_1_IdleTime, core_2_IdleTime, core_3_IdleTime],
	#["StallTime", core_0_StallTime, core_1_StallTime, core_2_StallTime, core_3_StallTime],
	#["SystemTime(SysCalls)", core_0_SystemTime, core_1_SystemTime, core_2_SystemTime, core_3_SystemTime],
	#["BusyTime", core_0_BusyTime, core_1_BusyTime, core_2_BusyTime, core_3_BusyTime]

	if int(options.OutChart) == 1:
		total_cycles = cpu_data.getfloat('General', 'TotalCycles')

		table_P4 = [
			[core_0_BusyTime, core_0_SystemTime, core_0_FetchStall, core_0_ROBStallLoad, core_0_ROBStallStore, core_0_ROBStallOther, core_0_IdleTime],
			[core_1_BusyTime, core_1_SystemTime, core_1_FetchStall, core_1_ROBStallLoad, core_1_ROBStallStore, core_1_ROBStallOther, core_1_IdleTime],
			[core_2_BusyTime, core_2_SystemTime, core_2_FetchStall, core_2_ROBStallLoad, core_2_ROBStallStore, core_2_ROBStallOther, core_2_IdleTime],
			[core_3_BusyTime, core_3_SystemTime, core_3_FetchStall, core_3_ROBStallLoad, core_3_ROBStallStore, core_3_ROBStallOther, core_3_IdleTime]
			]

		#cpu_stats = np.array()
		df = pd.DataFrame(table_P4, columns=['Busy', 'System', 'Fetch stall', 'Load stall', 'Store stall', 'Functional stall', 'Idle'], index=['P0', 'P1', 'P2', 'P3'])
		axes = df.plot(kind='bar', stacked=True, colormap='bone', title="Backprop OMP 4096", rot=0)
	
		y_major_ticks = np.arange(0, (total_cycles*1.4), (total_cycles*.20))
		axes.set_yticks(y_major_ticks)
		y_ticks = [0, total_cycles*0.2, total_cycles*0.4, total_cycles*0.6, total_cycles*0.8, total_cycles*1.0, total_cycles*1.2, total_cycles*1.4]

		axes.set(xlabel="Core", ylabel="Percent of Cycles", yticklabels=['{:0.2f}'.format(y_ticks[0]/total_cycles), '{:0.2f}'.format(y_ticks[1]/total_cycles), '{:0.2f}'.format(y_ticks[2]/total_cycles), '{:0.2f}'.format(y_ticks[3]/total_cycles), '{:0.2f}'.format(y_ticks[4]/total_cycles), '{:0.2f}'.format(y_ticks[5]/total_cycles), '{:0.2f}'.format(y_ticks[6]/total_cycles), '{:0.2f}'.format(y_ticks[7]/total_cycles)])
		axes.grid(b=True, which='major', color='black', linestyle='--')
		axes.grid(b=True, which='minor', color='black', linestyle='--')
		axes.legend(loc='upper right', ncol=4)
	
		#for container in axes.containers:
	        #      plt.setp(container, width=0.5)

		#x0, x1 = axes.get_xlim()
		#axes.set_xlim(x0 -0.25, x1 + 0.25)

		plt.show()


	if int(options.OutChart) == 2:
		total_cycles = cpu_data.getfloat('General', 'TotalCycles')

		ave_busy_time = (core_0_BusyTime + core_1_BusyTime + core_2_BusyTime + core_3_BusyTime)/int(options.NumCores)
		ave_SystemTime = (core_0_SystemTime + core_1_SystemTime + core_2_SystemTime + core_3_SystemTime)/int(options.NumCores)
		ave_FetchStall = (core_0_FetchStall + core_1_FetchStall + core_2_FetchStall + core_3_FetchStall)/int(options.NumCores)
		ave_ROBStallLoad = (core_0_ROBStallLoad + core_1_ROBStallLoad + core_2_ROBStallLoad + core_3_ROBStallLoad)/int(options.NumCores)
		ave_ROBStallStore = (core_0_ROBStallStore + core_1_ROBStallStore + core_2_ROBStallStore + core_3_ROBStallStore)/int(options.NumCores)
		ave_ROBStallOther = (core_0_ROBStallOther + core_1_ROBStallOther + core_2_ROBStallOther + core_3_ROBStallOther)/int(options.NumCores)
		ave_IdleTime = (core_0_IdleTime + core_1_IdleTime + core_2_IdleTime + core_3_IdleTime)/int(options.NumCores)

		table_P4 = [
			[ave_busy_time, ave_SystemTime, ave_FetchStall, ave_ROBStallLoad, ave_ROBStallStore, ave_ROBStallOther, ave_IdleTime]
			]

		#cpu_stats = np.array()
		df = pd.DataFrame(table_P4, index=['Ave All CoresP0'], columns=['Busy', 'System', 'Fetch stall', 'Load stall', 'Store stall', 'Functional stall', 'Idle'])
		axes = df.plot(kind='bar', stacked=True, colormap='bone', title="Backprop OMP 4096", rot=0)
	
		y_major_ticks = np.arange(0, (total_cycles*1.4), (total_cycles*.20))
		axes.set_yticks(y_major_ticks)
		y_ticks = [0, total_cycles*0.2, total_cycles*0.4, total_cycles*0.6, total_cycles*0.8, total_cycles*1.0, total_cycles*1.2, total_cycles*1.4]

		axes.set(xlabel="Core", ylabel="Percent of Cycles", yticklabels=['{:0.2f}'.format(y_ticks[0]/total_cycles), '{:0.2f}'.format(y_ticks[1]/total_cycles), '{:0.2f}'.format(y_ticks[2]/total_cycles), '{:0.2f}'.format(y_ticks[3]/total_cycles), '{:0.2f}'.format(y_ticks[4]/total_cycles), '{:0.2f}'.format(y_ticks[5]/total_cycles), '{:0.2f}'.format(y_ticks[6]/total_cycles), '{:0.2f}'.format(y_ticks[7]/total_cycles)])
		axes.grid(b=True, which='major', color='black', linestyle='--')
		axes.grid(b=True, which='minor', color='black', linestyle='--')
		axes.legend(loc='upper right', ncol=4)
		plt.show()



	return


def plot_stats_omp_1(options):

	p1_data = ConfigParser.ConfigParser()
	p1_data.optionxform = str
	p1_data.read(options.InFileName1)

	p2_data = ConfigParser.ConfigParser()
	p2_data.optionxform = str
	p2_data.read(options.InFileName2)

	p4_data = ConfigParser.ConfigParser()
	p4_data.optionxform = str
	p4_data.read(options.InFileName3)

	#pull stats 	
	p1_stats = dict(p1_data.items('ParallelStats'))
	for key, value in p1_stats.items(): #get the (key, value) tuples one at a time
		try:
			p1_stats[key] = int(value)
		except ValueError:
			p1_stats[key] = float(value)

	p2_stats = dict(p2_data.items('ParallelStats'))
	for key, value in p2_stats.items(): #get the (key, value) tuples one at a time
		try:
			p2_stats[key] = int(value)
		except ValueError:
			p2_stats[key] = float(value)

	p4_stats = dict(p4_data.items('ParallelStats'))
	for key, value in p4_stats.items(): #get the (key, value) tuples one at a time
		try:
			p4_stats[key] = int(value)
		except ValueError:
			p4_stats[key] = float(value)
	
	total_cycles_table = [
			[p1_stats['core_0_RunTime']], 
			[(p2_stats['core_0_RunTime'] + p2_stats['core_1_RunTime'])/2], 
			[(p4_stats['core_0_RunTime'] + p4_stats['core_1_RunTime'] + p4_stats['core_2_RunTime'] + p4_stats['core_3_RunTime'])/4]
			]

	total_cycles = p1_stats['core_0_RunTime']
	
	#cpu_stats = np.array()
	#df = pd.DataFrame(total_cycles, index=['Benchmark @ P1'], columns=['Busy', 'System', 'Fetch stall', 'Load stall', 'Store stall', 'Functional stall'])
	#axes = df.plot(kind='bar', stacked=True, colormap='bone', title="Backprop OMP P1 4096", rot=0)

	df = pd.DataFrame(total_cycles_table, index=['P1', 'P2', 'P4'], columns=['total cycles'])
	axes = df.plot(kind='bar', colormap='bone', title="Backprop OMP 4096 (Parallel section)", rot=0)

	y_major_ticks = np.arange(0, (total_cycles*1.4), (total_cycles*.20))
	axes.set_yticks(y_major_ticks)
	y_ticks = [0, total_cycles*0.2, total_cycles*0.4, total_cycles*0.6, total_cycles*0.8, total_cycles*1.0, total_cycles*1.2, total_cycles*1.4]

	axes.set(xlabel="Num Threads", ylabel="Percent of Cycles", yticklabels=['{:0.0f}%'.format((y_ticks[0]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[1]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[2]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[3]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[4]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[5]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[6]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[7]/total_cycles)*100)])
	axes.grid(b=True, which='major', color='black', linestyle='--')
	axes.grid(b=True, which='minor', color='black', linestyle='--')
	axes.legend(loc='upper right', ncol=4)
	plt.show()
	
	return

def plot_stats_omp_2(options):

	p1_data = ConfigParser.ConfigParser()
	p1_data.optionxform = str
	p1_data.read(options.InFileName1)

	p2_data = ConfigParser.ConfigParser()
	p2_data.optionxform = str
	p2_data.read(options.InFileName2)

	p4_data = ConfigParser.ConfigParser()
	p4_data.optionxform = str
	p4_data.read(options.InFileName3)

	#pull stats 	
	p1_stats = dict(p1_data.items('ParallelStats'))
	for key, value in p1_stats.items(): #get the (key, value) tuples one at a time
		try:
			p1_stats[key] = int(value)
		except ValueError:
			p1_stats[key] = float(value)

	p2_stats = dict(p2_data.items('ParallelStats'))
	for key, value in p2_stats.items(): #get the (key, value) tuples one at a time
		try:
			p2_stats[key] = int(value)
		except ValueError:
			p2_stats[key] = float(value)

	p4_stats = dict(p4_data.items('ParallelStats'))
	for key, value in p4_stats.items(): #get the (key, value) tuples one at a time
		try:
			p4_stats[key] = int(value)
		except ValueError:
			p4_stats[key] = float(value)
	
	total_cycles_table = [
			[p1_stats['core_0_BusyTime'], 
			p1_stats['core_0_SystemTime'], 
			p1_stats['core_0_FetchStall'], 
			p1_stats['core_0_ROBStallLoad'], 
			p1_stats['core_0_ROBStallStore'], 
			p1_stats['core_0_ROBStallOther']],
			[(p2_stats['core_0_BusyTime'] + p2_stats['core_1_BusyTime'])/2, 
			(p2_stats['core_0_SystemTime'] + p2_stats['core_1_SystemTime'])/2, 
			(p2_stats['core_0_FetchStall'] + p2_stats['core_1_FetchStall'])/2, 
			(p2_stats['core_0_ROBStallLoad'] + p2_stats['core_1_ROBStallLoad'])/2, 
			(p2_stats['core_0_ROBStallStore'] + p2_stats['core_1_ROBStallStore'])/2, 
			(p2_stats['core_1_ROBStallOther'] + p2_stats['core_0_ROBStallOther'])/2],
			[(p4_stats['core_0_BusyTime'] + p4_stats['core_1_BusyTime'] + p4_stats['core_2_BusyTime'] + p4_stats['core_3_BusyTime'])/4, 
			(p4_stats['core_0_SystemTime'] + p4_stats['core_1_SystemTime'] + p4_stats['core_2_SystemTime'] + p4_stats['core_3_SystemTime'])/4, 
			(p4_stats['core_0_FetchStall'] + p4_stats['core_1_FetchStall'] + p4_stats['core_2_FetchStall'] + p4_stats['core_3_FetchStall'])/4, 
			(p4_stats['core_0_ROBStallLoad'] + p4_stats['core_1_ROBStallLoad'] + p4_stats['core_2_ROBStallLoad'] + p4_stats['core_3_ROBStallLoad'])/4, 
			(p4_stats['core_0_ROBStallStore'] + p4_stats['core_1_ROBStallStore'] + p4_stats['core_2_ROBStallStore'] + p4_stats['core_3_ROBStallStore'])/4, 
			(p4_stats['core_1_ROBStallOther'] + p4_stats['core_0_ROBStallOther'] + p4_stats['core_2_ROBStallOther'] + p4_stats['core_3_ROBStallOther'])/4]
			]

	total_cycles = p1_stats['core_0_RunTime']
	
	#cpu_stats = np.array()
	#df = pd.DataFrame(total_cycles, index=['Benchmark @ P1'], columns=['Busy', 'System', 'Fetch stall', 'Load stall', 'Store stall', 'Functional stall'])
	#axes = df.plot(kind='bar', stacked=True, colormap='bone', title="Backprop OMP P1 4096", rot=0)

	df = pd.DataFrame(total_cycles_table, index=['P1', 'P2', 'P4'], columns=['Busy', 'System', 'Fetch stall', 'Load stall', 'Store stall', 'Functional stall'],)
	axes = df.plot(kind='bar', stacked=True, colormap='bone', title="Backprop OMP 4096 (Parallel section)", rot=0)

	y_major_ticks = np.arange(0, (total_cycles*1.4), (total_cycles*.20))
	axes.set_yticks(y_major_ticks)
	y_ticks = [0, total_cycles*0.2, total_cycles*0.4, total_cycles*0.6, total_cycles*0.8, total_cycles*1.0, total_cycles*1.2, total_cycles*1.4]

	axes.set(xlabel="Num Threads", ylabel="Percent of Cycles", yticklabels=['{:0.0f}%'.format((y_ticks[0]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[1]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[2]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[3]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[4]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[5]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[6]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[7]/total_cycles)*100)])
	axes.grid(b=True, which='major', color='black', linestyle='--')
	axes.grid(b=True, which='minor', color='black', linestyle='--')
	axes.legend(loc='upper right', ncol=3)
	plt.show()

	return

def plot_stats_ocl_1(options):


	p1_data = ConfigParser.ConfigParser()
	p1_data.optionxform = str
	p1_data.read(options.InFileName1)

	p2_data = ConfigParser.ConfigParser()
	p2_data.optionxform = str
	p2_data.read(options.InFileName2)

	p4_data = ConfigParser.ConfigParser()
	p4_data.optionxform = str
	p4_data.read(options.InFileName3)

	gpu_data = ConfigParser.ConfigParser()
	gpu_data.optionxform = str
	gpu_data.read(options.InFileName4)

	#pull stats 	
	p1_stats = dict(p1_data.items('ParallelStats'))
	for key, value in p1_stats.items(): #get the (key, value) tuples one at a time
		try:
			p1_stats[key] = int(value)
		except ValueError:
			p1_stats[key] = float(value)

	p2_stats = dict(p2_data.items('ParallelStats'))
	for key, value in p2_stats.items(): #get the (key, value) tuples one at a time
		try:
			p2_stats[key] = int(value)
		except ValueError:
			p2_stats[key] = float(value)

	p4_stats = dict(p4_data.items('ParallelStats'))
	for key, value in p4_stats.items(): #get the (key, value) tuples one at a time
		try:
			p4_stats[key] = int(value)
		except ValueError:
			p4_stats[key] = float(value)

	gpu_stats = dict(gpu_data.items('ParallelStats'))
	for key, value in gpu_stats.items(): #get the (key, value) tuples one at a time
		try:
			gpu_stats[key] = int(value)
		except ValueError:
			gpu_stats[key] = float(value)
	
	total_cycles_table = [
			[p1_stats['core_0_BusyTime'], 
			p1_stats['core_0_SystemTime'], 
			p1_stats['core_0_FetchStall'], 
			p1_stats['core_0_ROBStallLoad'], 
			p1_stats['core_0_ROBStallStore'], 
			p1_stats['core_0_ROBStallOther']],
			[(p2_stats['core_0_BusyTime'] + p2_stats['core_1_BusyTime'])/2, 
			(p2_stats['core_0_SystemTime'] + p2_stats['core_1_SystemTime'])/2, 
			(p2_stats['core_0_FetchStall'] + p2_stats['core_1_FetchStall'])/2, 
			(p2_stats['core_0_ROBStallLoad'] + p2_stats['core_1_ROBStallLoad'])/2, 
			(p2_stats['core_0_ROBStallStore'] + p2_stats['core_1_ROBStallStore'])/2, 
			(p2_stats['core_0_ROBStallOther'] + p2_stats['core_1_ROBStallOther'])/2],
			[(p4_stats['core_0_BusyTime'] + p4_stats['core_1_BusyTime'] + p4_stats['core_2_BusyTime'] + p4_stats['core_3_BusyTime'])/4, 
			(p4_stats['core_0_SystemTime'] + p4_stats['core_1_SystemTime'] + p4_stats['core_2_SystemTime'] + p4_stats['core_3_SystemTime'])/4, 
			(p4_stats['core_0_FetchStall'] + p4_stats['core_1_FetchStall'] + p4_stats['core_2_FetchStall'] + p4_stats['core_3_FetchStall'])/4, 
			(p4_stats['core_0_ROBStallLoad'] + p4_stats['core_1_ROBStallLoad'] + p4_stats['core_2_ROBStallLoad'] + p4_stats['core_3_ROBStallLoad'])/4, 
			(p4_stats['core_0_ROBStallStore'] + p4_stats['core_1_ROBStallStore'] + p4_stats['core_2_ROBStallStore'] + p4_stats['core_3_ROBStallStore'])/4, 
			(p4_stats['core_0_ROBStallOther'] + p4_stats['core_1_ROBStallOther'] + p4_stats['core_2_ROBStallOther'] + p4_stats['core_3_ROBStallOther'])/4],
			[(gpu_stats['core_0_BusyTime'] + gpu_stats['core_3_BusyTime'])/2, 
			(gpu_stats['core_0_SystemTime'] + gpu_stats['core_3_SystemTime'])/2, 
			(gpu_stats['core_0_FetchStall'] + gpu_stats['core_3_FetchStall'])/2, 
			(gpu_stats['core_0_ROBStallLoad'] + gpu_stats['core_3_ROBStallLoad'])/2, 
			(gpu_stats['core_0_ROBStallStore'] + gpu_stats['core_3_ROBStallStore'])/2, 
			(gpu_stats['core_0_ROBStallOther'] + gpu_stats['core_3_ROBStallOther'])/2]
			]

	total_cycles = p1_stats['core_0_RunTime']
	
	speed_up_table = [
		[p1_stats['core_0_RunTime']/p1_stats['core_0_RunTime'], 
		p1_stats['core_0_RunTime']/p2_stats['core_0_RunTime'],
		p1_stats['core_0_RunTime']/p4_stats['core_0_RunTime'],				
		p1_stats['core_0_RunTime']/gpu_stats['core_0_RunTime']]
		]

			


	#cpu_stats = np.array()
	#df = pd.DataFrame(total_cycles, index=['Benchmark @ P1'], columns=['Busy', 'System', 'Fetch stall', 'Load stall', 'Store stall', 'Functional stall'])
	#axes = df.plot(kind='bar', stacked=True, colormap='bone', title="Backprop OMP P1 4096", rot=0)

	df = pd.DataFrame(total_cycles_table, index=['P1', 'P2', 'P4', 'P4_GPGPU'], columns=['Busy', 'System', 'Fetch stall', 'Load stall', 'Store stall', 'Functional stall'],)

	#tb = pd.DataFrame(speed_up_table, index=['Speed Up'], columns=['', '', '' , ''])

	axes = df.plot(kind='bar', stacked=True, colormap='bone', title="Backprop OMP/OCL 4096 (Parallel section)", rot=0)

	y_major_ticks = np.arange(0, (total_cycles*1.2), (total_cycles*.20))
	axes.set_yticks(y_major_ticks)
	y_ticks = [0, total_cycles*0.2, total_cycles*0.4, total_cycles*0.6, total_cycles*0.8, total_cycles*1.0, total_cycles*1.2, total_cycles*1.4]

	axes.set(xlabel="Num Threads", ylabel="Percent of Cycles", yticklabels=['{:0.0f}%'.format((y_ticks[0]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[1]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[2]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[3]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[4]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[5]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[6]/total_cycles)*100), '{:0.0f}%'.format((y_ticks[7]/total_cycles)*100)])
	axes.grid(b=True, which='major', color='black', linestyle='--')
	axes.grid(b=True, which='minor', color='black', linestyle='--')
	axes.legend(loc='upper right', ncol=3)
	plt.show()

	return



parser = OptionParser()
parser.usage = "%prog -m mode -1 inputfile1 -2 inputfile2 -3 inputfile3 -g graphtype (1-2)"
parser.add_option("-m", "--mode", dest="Mode", default="omp", help="Specifiy the mode.")
parser.add_option("-n", "--name", dest="Name", default="", help="Specifiy the benchmark name.")
parser.add_option("-1", "--infile1", dest="InFileName1", default="", help="Specifiy the 1st stats file and path to parse.")
parser.add_option("-2", "--infile2", dest="InFileName2", default="", help="Specifiy the 2nd stats file and path to parse.")
parser.add_option("-3", "--infile3", dest="InFileName3", default="", help="Specifiy the 3rd stats file and path to parse.")
parser.add_option("-4", "--infile4", dest="InFileName4", default="", help="Specifiy the 4th stats file and path to parse.")
parser.add_option("-o", "--outchart", dest="OutChart", default="1", help="Specifiy the output chart.")
(options, args) = parser.parse_args()

if not options.Mode:
	parser.print_usage()
	exit(0)

if not options.InFileName1:
	parser.print_usage()
	exit(0)

print "using Matplotlib version " + matplotlib.__version__
print "using Pandas version " + pd.__version__
print "using Numpy version " + np.__version__

if options.Mode == 'fixme':
	print "bad mode"
	exit(0)
	plot_stats(options)

elif options.Mode == 'omp':

	if not (options.InFileName1 and options.InFileName2 and options.InFileName3):
		parser.print_usage()
		exit(0)

	if int(options.OutChart) <= 0 or int(options.OutChart) > 2:
		parser.print_usage()
		exit(0)

	#print the chart for total cycles
	if int(options.OutChart) == 1:
		plot_stats_omp_1(options)

	#print the chart for total cycles broken down
	if int(options.OutChart) == 2:
		plot_stats_omp_2(options)

elif options.Mode == 'ocl':

	if not (options.InFileName1 and options.InFileName2 and options.InFileName3 and options.InFileName4):
		parser.print_usage()
		exit(0)

	if int(options.OutChart) <= 0 or int(options.OutChart) > 2:
		parser.print_usage()
		exit(0)

	#print the chart for total cycles
	if int(options.OutChart) == 1:
		plot_stats_ocl_1(options)
	
else:
	print "bad mode"
	exit(0)

