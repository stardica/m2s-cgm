#!/usr/bin/env bash
#put the cache stats into a table.
python get_cache_data.py &> cache_stats.out

cat ./fetch_lat_log_file.out | /usr/local/bin/histogram.py --agg-key-value --buckets 100 --percent &> fetch_lat_hist.out
cat ./load_lat_log_file.out | /usr/local/bin/histogram.py --agg-key-value --buckets 100 --percent &> load_lat_hist.out
cat ./store_lat_log_file.out | /usr/local/bin/histogram.py --agg-key-value --buckets 100 --percent &> store_lat_hist.out
