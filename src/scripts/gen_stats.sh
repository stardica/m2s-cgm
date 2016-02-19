#!/usr/bin/env bash
python get_cache_stats.py 
python get_switch_stats.py
python get_sa_stats.py


cat ./fetch_lat_log_file.out | /home/stardica/Desktop/m2s-cgm/src/scripts/histogram.py --agg-key-value --buckets 100 --percent &> fetch_lat_hist.txt
cat ./load_lat_log_file.out | /home/stardica/Desktop/m2s-cgm/src/scripts/histogram.py --agg-key-value --buckets 100 --percent &> load_lat_hist.txt
cat ./store_lat_log_file.out | /home/stardica/Desktop/m2s-cgm/src/scripts/histogram.py --agg-key-value --buckets 100 --percent &> store_lat_hist.txt

