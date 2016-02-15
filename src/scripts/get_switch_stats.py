#!/usr/bin/env python

from tabulate import tabulate
import ConfigParser

switch_data = ConfigParser.ConfigParser()
switch_data.read("/home/stardica/Desktop/m2s-cgm/src/scripts/m2s-cgm.out")

s_0_total_ctrl_loops = switch_data.getint('Switch_0', 'NumberSwitchCtrlLoops')
s_0_occupance = switch_data.getfloat('Switch_0', 'SwitchOccupance')
s_0_total_links_formed = switch_data.getint('Switch_0', 'NumberLinks')
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


s_1_total_ctrl_loops = switch_data.getint('Switch_1', 'NumberSwitchCtrlLoops')
s_1_occupance = switch_data.getfloat('Switch_1', 'SwitchOccupance')
s_1_total_links_formed = switch_data.getint('Switch_1', 'NumberLinks')
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


s_2_total_ctrl_loops = switch_data.getint('Switch_2', 'NumberSwitchCtrlLoops')
s_2_occupance = switch_data.getfloat('Switch_2', 'SwitchOccupance')
s_2_total_links_formed = switch_data.getint('Switch_2', 'NumberLinks')
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


s_3_total_ctrl_loops = switch_data.getint('Switch_3', 'NumberSwitchCtrlLoops')
s_3_occupance = switch_data.getfloat('Switch_3', 'SwitchOccupance')
s_3_total_links_formed = switch_data.getint('Switch_3', 'NumberLinks')
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

s_4_total_ctrl_loops = switch_data.getint('Switch_4', 'NumberSwitchCtrlLoops')
s_4_occupance = switch_data.getfloat('Switch_4', 'SwitchOccupance')
s_4_total_links_formed = switch_data.getint('Switch_4', 'NumberLinks')
s_4_ave_links_formed_per_ctrl_loop = switch_data.getfloat('Switch_4', 'AveNumberLinksPerCtrlLoop')
s_4_north_io_transfers = switch_data.getint('Switch_4', 'NorthIOTransfers')
s_4_north_io_cycles = switch_data.getint('Switch_4', 'NorthIOCycles')
s_4_north_io_bytes_transfered = switch_data.getint('Switch_4', 'NorthIOBytesTransfered')
s_4_north_rxqueue_max_depth = switch_data.getint('Switch_4', 'NorthRxQueueMaxDepth')
s_4_north_rxqueue_ave_depth = switch_data.getfloat('Switch_4', 'NorthRxQueueAveDepth')
s_4_north_txqueue_max_depth = switch_data.getint('Switch_4', 'NorthTxQueueMaxDepth')
s_4_north_txqueue_ave_depth = switch_data.getfloat('Switch_4', 'NorthTxQueueAveDepth')
s_4_east_io_transfers = switch_data.getint('Switch_4', 'EastIOTransfers')
s_4_east_io_cycles = switch_data.getint('Switch_4', 'EastIOCycles')
s_4_east_io_bytes_transfered = switch_data.getint('Switch_4', 'EastIOBytesTransfered')
s_4_east_io_bytes_transfered = switch_data.getint('Switch_4', 'EastIOBytesTransfered')
s_4_east_io_bytes_transfered = switch_data.getint('Switch_4', 'EastIOBytesTransfered')
s_4_east_rxqueue_max_depth = switch_data.getint('Switch_4', 'EastRxQueueMaxDepth')
s_4_east_rxqueue_ave_depth = switch_data.getfloat('Switch_4', 'EastRxQueueAveDepth')
s_4_east_txqueue_max_depth = switch_data.getint('Switch_4', 'EastTxQueueMaxDepth')
s_4_east_txqueue_ave_depth = switch_data.getfloat('Switch_4', 'EastTxQueueAveDepth')
s_4_south_io_transfers = switch_data.getint('Switch_4', 'SouthIOTransfers')
s_4_south_io_cycles = switch_data.getint('Switch_4', 'SouthIOCycles')
s_4_south_io_bytes_transfered = switch_data.getint('Switch_4', 'SouthIOBytesTransfered')
s_4_south_io_bytes_transfered = switch_data.getint('Switch_4', 'SouthIOBytesTransfered')
s_4_south_rxqueue_max_depth = switch_data.getint('Switch_4', 'SouthRxQueueMaxDepth')
s_4_south_rxqueue_ave_depth = switch_data.getfloat('Switch_4', 'SouthRxQueueAveDepth')
s_4_south_txqueue_max_depth = switch_data.getint('Switch_4', 'SouthTxQueueMaxDepth')
s_4_south_txqueue_ave_depth = switch_data.getfloat('Switch_4', 'SouthTxQueueAveDepth')
s_4_west_io_transfers = switch_data.getint('Switch_4', 'WestIOTransfers')
s_4_west_io_cycles = switch_data.getint('Switch_4', 'WestIOCycles')
s_4_west_io_bytes_transfered = switch_data.getint('Switch_4', 'WestIOBytesTransfered')
s_4_west_io_bytes_transfered = switch_data.getint('Switch_4', 'WestIOBytesTransfered')
s_4_west_rxqueue_max_depth = switch_data.getint('Switch_4', 'WestRxQueueMaxDepth')
s_4_west_rxqueue_ave_depth = switch_data.getfloat('Switch_4', 'WestRxQueueAveDepth')
s_4_west_txqueue_max_depth = switch_data.getint('Switch_4', 'WestTxQueueMaxDepth')
s_4_west_txqueue_ave_depth = switch_data.getfloat('Switch_4', 'WestTxQueueAveDepth')

table_switch_data = [
["TotalSwitchCtrlLoops", s_0_total_ctrl_loops, s_1_total_ctrl_loops, s_2_total_ctrl_loops, s_3_total_ctrl_loops, s_4_total_ctrl_loops],
["SwitchOccupance", s_0_occupance, s_1_occupance, s_2_occupance, s_3_occupance, s_4_occupance],
["TotalLinksFormed", s_0_total_links_formed, s_1_total_links_formed, s_2_total_links_formed, s_3_total_links_formed, s_4_total_ctrl_loops],
["TotalAveNumberLinksPerCtrlLoop", s_0_ave_links_formed_per_ctrl_loop, s_1_ave_links_formed_per_ctrl_loop, s_2_ave_links_formed_per_ctrl_loop, s_3_ave_links_formed_per_ctrl_loop, s_4_ave_links_formed_per_ctrl_loop],
["NorthIOTransfers", s_0_north_io_transfers, s_1_north_io_transfers, s_2_north_io_transfers, s_3_north_io_transfers, s_4_north_io_transfers ],
["NorthIOCycles", s_0_north_io_cycles, s_1_north_io_cycles, s_2_north_io_cycles, s_3_north_io_cycles, s_4_north_io_cycles],
["NorthIOBytesTransfered", s_0_north_io_bytes_transfered, s_1_north_io_bytes_transfered, s_2_north_io_bytes_transfered, s_3_north_io_bytes_transfered, s_4_north_io_bytes_transfered],
["NorthRxQueueMaxDepth", s_0_north_rxqueue_max_depth, s_1_north_rxqueue_max_depth, s_2_north_rxqueue_max_depth, s_3_north_rxqueue_max_depth, s_4_north_rxqueue_max_depth],
["NorthRxQueueAveDepth", s_0_north_rxqueue_ave_depth, s_1_north_rxqueue_ave_depth, s_2_north_rxqueue_ave_depth, s_3_north_rxqueue_ave_depth, s_4_north_rxqueue_ave_depth],
["NorthTxQueueMaxDepth", s_0_north_txqueue_max_depth, s_1_north_txqueue_max_depth, s_2_north_txqueue_max_depth, s_3_north_txqueue_max_depth, s_4_north_txqueue_max_depth],
["NorthTxQueueAveDepth", s_0_north_txqueue_ave_depth, s_1_north_txqueue_ave_depth, s_2_north_txqueue_ave_depth, s_3_north_txqueue_ave_depth, s_4_north_txqueue_ave_depth],
["EastIOTransfers", s_0_east_io_transfers, s_1_east_io_transfers, s_2_east_io_transfers, s_3_east_io_transfers, s_4_east_io_transfers],
["EastIOCycles", s_0_east_io_cycles, s_1_east_io_cycles, s_2_east_io_cycles, s_3_east_io_cycles, s_4_east_io_cycles],
["EastIOBytesTransfered", s_0_east_io_bytes_transfered, s_1_east_io_bytes_transfered, s_2_east_io_bytes_transfered, s_3_east_io_bytes_transfered, s_4_east_io_bytes_transfered],
["EastRxQueueMaxDepth", s_0_east_rxqueue_max_depth, s_1_east_rxqueue_max_depth, s_2_east_rxqueue_max_depth, s_3_east_rxqueue_max_depth, s_4_east_rxqueue_max_depth],
["EastRxQueueAveDepth", s_0_east_rxqueue_ave_depth, s_1_east_rxqueue_ave_depth, s_2_east_rxqueue_ave_depth, s_3_east_rxqueue_ave_depth, s_4_east_rxqueue_ave_depth],
["EastTxQueueMaxDepth", s_0_east_txqueue_max_depth, s_1_east_txqueue_max_depth, s_2_east_txqueue_max_depth, s_3_east_txqueue_max_depth, s_4_east_txqueue_max_depth],
["EastTxQueueAveDepth", s_0_east_txqueue_ave_depth, s_1_east_txqueue_ave_depth, s_2_east_txqueue_ave_depth, s_3_east_txqueue_ave_depth, s_4_east_txqueue_ave_depth],
["SouthIOTransfers", s_0_south_io_transfers, s_1_south_io_transfers, s_2_south_io_transfers, s_3_south_io_transfers, s_4_south_io_transfers],
["SouthIOCycles", s_0_south_io_cycles, s_1_south_io_cycles, s_2_south_io_cycles, s_3_south_io_cycles, s_4_south_io_cycles],
["SouthIOBytesTransfered", s_0_south_io_bytes_transfered, s_1_south_io_bytes_transfered, s_3_south_io_bytes_transfered, s_4_south_io_bytes_transfered, s_4_south_io_bytes_transfered],
["SouthRxQueueMaxDepth", s_0_south_rxqueue_max_depth, s_1_south_rxqueue_max_depth, s_2_south_rxqueue_max_depth, s_3_south_rxqueue_max_depth, s_4_south_rxqueue_max_depth],
["SouthRxQueueAveDepth", s_0_south_rxqueue_ave_depth, s_1_south_rxqueue_ave_depth, s_2_south_rxqueue_ave_depth, s_3_south_rxqueue_ave_depth, s_4_south_rxqueue_ave_depth],
["SouthTxQueueMaxDepth", s_0_south_txqueue_max_depth, s_1_south_txqueue_max_depth, s_2_south_txqueue_max_depth, s_3_south_txqueue_max_depth, s_4_south_txqueue_max_depth],
["SouthTxQueueAveDepth", s_0_south_txqueue_ave_depth, s_1_south_txqueue_ave_depth, s_2_south_txqueue_ave_depth, s_3_south_txqueue_ave_depth, s_4_south_txqueue_ave_depth],
["WestIOTransfers", s_0_west_io_transfers, s_1_west_io_transfers, s_2_west_io_transfers, s_3_west_io_transfers, s_4_west_io_transfers],
["WestIOCycles", s_0_west_io_cycles, s_1_west_io_cycles, s_2_west_io_cycles, s_3_west_io_cycles, s_4_west_io_cycles],
["WestIOBytesTransfered", s_0_west_io_bytes_transfered, s_1_west_io_bytes_transfered, s_2_west_io_bytes_transfered, s_3_west_io_bytes_transfered, s_4_west_io_bytes_transfered],
["WestRxQueueMaxDepth", s_0_west_rxqueue_max_depth, s_1_west_rxqueue_max_depth, s_2_west_rxqueue_max_depth, s_3_west_rxqueue_max_depth, s_4_west_rxqueue_max_depth],
["WestRxQueueAveDepth", s_0_west_rxqueue_ave_depth, s_1_west_rxqueue_ave_depth, s_2_west_rxqueue_ave_depth, s_3_west_rxqueue_ave_depth, s_4_west_rxqueue_ave_depth],
["WestTxQueueMaxDepth", s_0_west_txqueue_max_depth, s_1_west_txqueue_max_depth, s_2_west_txqueue_max_depth, s_3_west_txqueue_max_depth, s_4_west_txqueue_max_depth],
["WestTxQueueAveDepth", s_0_west_txqueue_ave_depth, s_1_west_txqueue_ave_depth, s_2_west_txqueue_ave_depth, s_3_west_txqueue_ave_depth, s_4_west_txqueue_ave_depth],
]

print tabulate(table_switch_data, headers=["Stat Switch", "S0", "S1", "S2", "S3", "S4"], tablefmt="simple", numalign="right", floatfmt="16.4f")
