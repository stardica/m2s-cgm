#!/usr/bin/env bash

python get_cache_stats.py &> cache_stats.out

cat ./fetch_lat_log_file.out | histogram.py --agg-key-value --buckets 100 --percent &> fetch_lat_hist.out
cat ./load_lat_log_file.out | histogram.py --agg-key-value --buckets 100 --percent &> load_lat_hist.out
cat ./store_lat_log_file.out | histogram.py --agg-key-value --buckets 100 --percent &> store_lat_hist.out

python get_switch_stats.py &> switch_stats.out
python get_sa_stats.py &> sa_stats.out
