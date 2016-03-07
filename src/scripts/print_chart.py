#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser




def plot_cpu_stats(options):


	




	return


parser = OptionParser()
parser.usage = "%prog -c numcores -i inputfile -o outputfile"
parser.add_option("-c", "--numcores", dest="NumCores", default="", help="Specifiy the number of cores.")
parser.add_option("-i", "--infile", dest="InFileName", default="", help="Specifiy the stats file and path to parse.")
parser.add_option("-o", "--outfile", dest="OutFileName", default="sim_stats.txt", help="Specifiy the outputfile name and path.")
(options, args) = parser.parse_args()

if not options.NumCores:
	parser.print_usage()
	exit(0)

if not options.InFileName:
	parser.print_usage()
	exit(0)

plot_cpu_stats(options)
