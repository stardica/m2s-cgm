import ConfigParser
import numpy as np
import matplotlib as mpl
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
#import seaborn as sns
np.random.seed(sum(map(ord, "aesthetics")))

# notes
# changes text to int with repr()
# get all sections from config file with Config.sections()

#variables
stats_file_path = "/home/stardica/Desktop/m2s-cgm/Release/cgm_stats.out"
benchmark_path = ""
total_cycles = 0
ROBstalls = 0

#Math functions
def sinplot(flip=1):
    x = np.linspace(0, 14, 100)
    for i in range(1, 7):
        plt.plot(x, np.sin(x + i * .5) * (7 - i) * flip)

#read the stats file
Config = ConfigParser.ConfigParser()
Config.read(stats_file_path)

#get specific values
benchmark_path = Config.get('General', 'Benchmark')
total_cycles = Config.getint('CPU', 'TotalCycles')
ROBstalls = Config.getint('CPU', 'ROBStalls')

#printing values
#print(benchmark_path)
#print("Total cycles " + repr(total_cycles))
#total_cycles = total_cycles + 5
#print("Total cycles blah blah " + repr(total_cycles))

#set the number of benchmark results
number_of_benchmarks = 2

bench_total_cycles = (total_cycles, 900000)
ROBstalls_array = (ROBstalls, 450000)

# the x locations for the groups
index = np.arange(number_of_benchmarks)

# the width of the bars: can also be len(x) sequence
width = 0.125       

opacity = 1

p1 = plt.bar(index, bench_total_cycles, width, alpha=opacity,  color = "w", hatch = 'x',)
p2 = plt.bar(index + width, ROBstalls_array, width, alpha=opacity, color = 'w', hatch = '/')

#place the data
plt.xticks(index + width, ('OpenCL 64x64 MM', 'OpenCL 32x32 MM'))
plt.xlim([0,2])
plt.yticks(np.arange(0, total_cycles, 500000))


#write the labels and ledgend
plt.ylabel('Cycles')
plt.xlabel('Benchmark')
plt.title('Test Plot')
plt.legend((p1[0], p2[0]), ('Total Cycles', 'ROB Stalls'))


#show the plot
plt.show()