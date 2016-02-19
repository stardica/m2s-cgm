#!/usr/bin/env bash
cat ./fetch_lat_log_file.out | /home/stardica/Desktop/m2s-cgm/src/scripts/histogram.py --agg-key-value --buckets 100 --percent --no-mvsd &> fetch_lat_hist.txt
cat ./load_lat_log_file.out | /home/stardica/Desktop/m2s-cgm/src/scripts/histogram.py --agg-key-value --buckets 100 --percent --no-mvsd &> load_lat_hist.txt
cat ./store_lat_log_file.out | /home/stardica/Desktop/m2s-cgm/src/scripts/histogram.py --agg-key-value --buckets 100 --percent --no-mvsd &> store_lat_hist.txt

