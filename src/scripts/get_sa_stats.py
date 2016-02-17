#!/usr/bin/env python

from tabulate import tabulate
import ConfigParser


sa_data = ConfigParser.ConfigParser()
sa_data.read("/home/stardica/Desktop/m2s-cgm/src/scripts/m2s-cgm.out")

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

f = open('sim_stats.out', 'a')

f.write("//SA-MC Stats//////////////////////////////////////////////////" + '\n')
f.write("///////////////////////////////////////////////////////////////"  + '\n\n')
f.write(tabulate(table_sa_data, headers=["Stat SA/MC", "SA", "MC"], tablefmt="simple", numalign="right", floatfmt="16.4f"))
f.write('\n\n\n')
f.close
