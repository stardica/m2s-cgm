#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser
import os
import matplotlib
import matplotlib.pyplot as plt
plt.style.use('ggplot')
import pandas as pd
import numpy as np


def print_histograms(options):

	
	

	os.system("histogram.py 1"

	
	return


parser = OptionParser()
parser.usage = "%prog -f fetchlogfile -l loadlogfile -s storelogfile"
parser.add_option("-f", "--fetchlog", dest="fetchlog", default="", help="Specifiy the fetch log file.")
parser.add_option("-l", "--loadlog", dest="loadlog", default="", help="Specifiy the load log file.")
parser.add_option("-s", "--storelog", dest="storelog", default="", help="Specifiy the store log file.")
(options, args) = parser.parse_args()

if not(options.fetchlog or options.loadlog or options.storelog):
	print "no inputs given"
	parser.print_usage()
	exit(0)

print "using Matplotlib version " + matplotlib.__version__
print "using Pandas version " + pd.__version__
print "using Numpy version " + np.__version__

print_histograms(options)
