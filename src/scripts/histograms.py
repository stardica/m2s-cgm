#!/usr/bin/env python
import ConfigParser
import data_hacks

array_size = 2000
fetch_hist_data=[0] * array_size
load_hist_data=[0] * array_size
store_hist_data=[0] * array_size

sim_data = ConfigParser.RawConfigParser()
sim_data.read('/home/stardica/Desktop/m2s-cgm/Release/m2s-cgm.out')

i = 0
fetch_hist_total = 0
load_hist_total = 0
store_hist_total = 0

for number in fetch_hist_data:
	fetch_hist_data[i] = sim_data.getint("MemSystemFetchLat", str(i))
	fetch_hist_total += fetch_hist_data[i]
	i+=1

#for number in load_hist_data:
#	load_hist_data[i] = sim_data.getint("MemSystemStoreLat", str(i))
#	load_hist_total += load_hist_data[i]
#	i+=1

#for number in store_hist_data:
#	store_hist_data[i] = sim_data.getint("MemSystemStoreLat", str(i))
#	store_hist_total += store_hist_data[i]
#	i+=1


