#!/usr/bin/env python

from tabulate import tabulate
import ConfigParser

switch_data = ConfigParser.ConfigParser()
switch_data.read("/home/stardica/Desktop/m2s-cgm/src/scripts/m2s-cgm.out")

s_0_total_ctrl_loops = switch_data.getint('Switch_0', 'NumberSwitchCtrlLoops')
s_0_total_links_formed = switch_data.getint('Switch_0', 'NumberLinks')
s_0_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_0', 'AveNumberLinksPerCtrlLoop')
s_0_north_io_transfers = switch_data.getint('Switch_0', 'NorthIOTransfers')
s_0_north_io_cycles = switch_data.getint('Switch_0', 'NorthIOCycles')
s_0_north_io_bytes_transfered = switch_data.getfloat('Switch_0', 'NorthIOBytesTransfered')
s_0_east_io_transfers = switch_data.getint('Switch_0', 'EastIOTransfers')
s_0_east_io_cycles = switch_data.getint('Switch_0', 'EastIOCycles')
s_0_east_io_bytes_transfered = switch_data.getint('Switch_0', 'EastIOBytesTransfered')
s_0_south_io_transfers = switch_data.getint('Switch_0', 'SouthIOTransfers')
s_0_south_io_cycles = switch_data.getint('Switch_0', 'SouthIOCycles')
s_0_south_io_bytes_transfered = switch_data.getint('Switch_0', 'SouthIOBytesTransfered')
s_0_west_io_transfers = switch_data.getint('Switch_0', 'WestIOTransfers')
s_0_west_io_cycles = switch_data.getint('Switch_0', 'WestIOCycles')
s_0_west_io_bytes_transfered = switch_data.getint('Switch_0', 'WestIOBytesTransfered')

s_1_total_ctrl_loops = switch_data.getint('Switch_1', 'NumberSwitchCtrlLoops')
s_1_total_links_formed = switch_data.getint('Switch_1', 'NumberLinks')
s_1_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_1', 'AveNumberLinksPerCtrlLoop')
s_1_north_io_transfers = switch_data.getint('Switch_1', 'NorthIOTransfers')
s_1_north_io_cycles = switch_data.getint('Switch_1', 'NorthIOCycles')
s_1_north_io_bytes_transfered = switch_data.getint('Switch_1', 'NorthIOBytesTransfered')
s_1_east_io_transfers = switch_data.getint('Switch_1', 'EastIOTransfers')
s_1_east_io_cycles = switch_data.getint('Switch_1', 'EastIOCycles')
s_1_east_io_bytes_transfered = switch_data.getint('Switch_1', 'EastIOBytesTransfered')
s_1_south_io_transfers = switch_data.getint('Switch_1', 'SouthIOTransfers')
s_1_south_io_cycles = switch_data.getint('Switch_1', 'SouthIOCycles')
s_1_south_io_bytes_transfered = switch_data.getint('Switch_1', 'SouthIOBytesTransfered')
s_1_west_io_transfers = switch_data.getint('Switch_1', 'WestIOTransfers')
s_1_west_io_cycles = switch_data.getint('Switch_1', 'WestIOCycles')
s_1_west_io_bytes_transfered = switch_data.getint('Switch_1', 'WestIOBytesTransfered')

s_2_total_ctrl_loops = switch_data.getint('Switch_2', 'NumberSwitchCtrlLoops')
s_2_total_links_formed = switch_data.getint('Switch_2', 'NumberLinks')
s_2_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_2', 'AveNumberLinksPerCtrlLoop')
s_2_north_io_transfers = switch_data.getint('Switch_2', 'NorthIOTransfers')
s_2_north_io_cycles = switch_data.getint('Switch_2', 'NorthIOCycles')
s_2_north_io_bytes_transfered = switch_data.getint('Switch_2', 'NorthIOBytesTransfered')
s_2_east_io_transfers = switch_data.getint('Switch_2', 'EastIOTransfers')
s_2_east_io_cycles = switch_data.getint('Switch_2', 'EastIOCycles')
s_2_east_io_bytes_transfered = switch_data.getint('Switch_2', 'EastIOBytesTransfered')
s_2_south_io_transfers = switch_data.getint('Switch_2', 'SouthIOTransfers')
s_2_south_io_cycles = switch_data.getint('Switch_2', 'SouthIOCycles')
s_2_south_io_bytes_transfered = switch_data.getint('Switch_2', 'SouthIOBytesTransfered')
s_2_west_io_transfers = switch_data.getint('Switch_2', 'WestIOTransfers')
s_2_west_io_cycles = switch_data.getint('Switch_2', 'WestIOCycles')
s_2_west_io_bytes_transfered = switch_data.getint('Switch_2', 'WestIOBytesTransfered')

s_3_total_ctrl_loops = switch_data.getint('Switch_3', 'NumberSwitchCtrlLoops')
s_3_total_links_formed = switch_data.getint('Switch_3', 'NumberLinks')
s_3_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_3', 'AveNumberLinksPerCtrlLoop')
s_3_north_io_transfers = switch_data.getint('Switch_3', 'NorthIOTransfers')
s_3_north_io_cycles = switch_data.getint('Switch_3', 'NorthIOCycles')
s_3_north_io_bytes_transfered = switch_data.getint('Switch_3', 'NorthIOBytesTransfered')
s_3_east_io_transfers = switch_data.getint('Switch_3', 'EastIOTransfers')
s_3_east_io_cycles = switch_data.getint('Switch_3', 'EastIOCycles')
s_3_east_io_bytes_transfered = switch_data.getint('Switch_3', 'EastIOBytesTransfered')
s_3_south_io_transfers = switch_data.getint('Switch_3', 'SouthIOTransfers')
s_3_south_io_cycles = switch_data.getint('Switch_3', 'SouthIOCycles')
s_3_south_io_bytes_transfered = switch_data.getint('Switch_3', 'SouthIOBytesTransfered')
s_3_west_io_transfers = switch_data.getint('Switch_3', 'WestIOTransfers')
s_3_west_io_cycles = switch_data.getint('Switch_3', 'WestIOCycles')
s_3_west_io_bytes_transfered = switch_data.getint('Switch_3', 'WestIOBytesTransfered')

s_4_total_ctrl_loops = switch_data.getint('Switch_4', 'NumberSwitchCtrlLoops')
s_4_total_links_formed = switch_data.getint('Switch_4', 'NumberLinks')
s_4_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_4', 'AveNumberLinksPerCtrlLoop')
s_4_north_io_transfers = switch_data.getint('Switch_4', 'NorthIOTransfers')
s_4_north_io_cycles = switch_data.getint('Switch_4', 'NorthIOCycles')
s_4_north_io_bytes_transfered = switch_data.getint('Switch_4', 'NorthIOBytesTransfered')
s_4_east_io_transfers = switch_data.getint('Switch_4', 'EastIOTransfers')
s_4_east_io_cycles = switch_data.getint('Switch_4', 'EastIOCycles')
s_4_east_io_bytes_transfered = switch_data.getint('Switch_4', 'EastIOBytesTransfered')
s_4_south_io_transfers = switch_data.getint('Switch_4', 'SouthIOTransfers')
s_4_south_io_cycles = switch_data.getint('Switch_4', 'SouthIOCycles')
s_4_south_io_bytes_transfered = switch_data.getint('Switch_4', 'SouthIOBytesTransfered')
s_4_west_io_transfers = switch_data.getint('Switch_4', 'WestIOTransfers')
s_4_west_io_cycles = switch_data.getint('Switch_4', 'WestIOCycles')
s_4_west_io_bytes_transfered = switch_data.getint('Switch_4', 'WestIOBytesTransfered')

table_switch_data = [
["TotalSwitchCtrlLoops", s_0_total_ctrl_loops, s_1_total_ctrl_loops, s_2_total_ctrl_loops, s_3_total_ctrl_loops, s_4_total_ctrl_loops],
["TotalLinksFormed", s_0_total_links_formed, s_1_total_links_formed, s_2_total_links_formed, s_3_total_links_formed, s_4_total_ctrl_loops],
["TotalAveNumberLinksPerCtrlLoop", s_0_ave_links_formed_per_ctrl_loop, s_1_ave_links_formed_per_ctrl_loop, s_2_ave_links_formed_per_ctrl_loop, s_3_ave_links_formed_per_ctrl_loop, s_4_ave_links_formed_per_ctrl_loop],
["NorthIOTransfers", s_0_north_io_transfers, s_1_north_io_transfers, s_2_north_io_transfers, s_3_north_io_transfers, s_4_north_io_transfers ],
["NorthIOCycles", s_0_north_io_cycles, s_1_north_io_cycles, s_2_north_io_cycles, s_3_north_io_cycles, s_4_north_io_cycles],
["NorthIOBytesTransfered", s_0_north_io_bytes_transfered, s_1_north_io_bytes_transfered, s_2_north_io_bytes_transfered, s_3_north_io_bytes_transfered, s_4_north_io_bytes_transfered],
["EastIOTransfers", s_0_east_io_transfers, s_1_east_io_transfers, s_2_east_io_transfers, s_3_east_io_transfers, s_4_east_io_transfers],
["EastIOCycles", s_0_east_io_cycles, s_1_east_io_cycles, s_2_east_io_cycles, s_3_east_io_cycles, s_4_east_io_cycles],
["EastIOBytesTransfered", s_0_east_io_bytes_transfered, s_1_east_io_bytes_transfered, s_2_east_io_bytes_transfered, s_3_east_io_bytes_transfered, s_4_east_io_bytes_transfered],
["SouthIOTransfers", s_0_south_io_transfers, s_1_south_io_transfers, s_2_south_io_transfers, s_3_south_io_transfers, s_4_south_io_transfers],
["SouthIOCycles", s_0_south_io_cycles, s_1_south_io_cycles, s_2_south_io_cycles, s_3_south_io_cycles, s_4_south_io_cycles],
["SouthIOBytesTransfered", s_0_south_io_bytes_transfered, s_1_south_io_bytes_transfered, s_3_south_io_bytes_transfered, s_4_south_io_bytes_transfered, s_4_south_io_bytes_transfered],
["WestIOTransfers", s_0_west_io_transfers, s_1_west_io_transfers, s_2_west_io_transfers, s_3_west_io_transfers, s_4_west_io_transfers],
["WestIOCycles", s_0_west_io_cycles, s_1_west_io_cycles, s_2_west_io_cycles, s_3_west_io_cycles, s_4_west_io_cycles],
["WestIOBytesTransfered", s_0_west_io_bytes_transfered, s_1_west_io_bytes_transfered, s_2_west_io_bytes_transfered, s_3_west_io_bytes_transfered, s_4_west_io_bytes_transfered],
]

print tabulate(table_switch_data, headers=["Stat Switch", "S0", "S1", "S2", "S3", "S4"], tablefmt="simple", numalign="right", floatfmt="16.4f")
