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
busy_cycles = total_cycles - ROBstalls 


#printing values
#print(benchmark_path)
#print("Total cycles " + repr(total_cycles))
#total_cycles = total_cycles + 5
#print("Total cycles blah blah " + repr(total_cycles))

#set the number of benchmark results
number_of_benchmarks = 1

#ROBstalls_array = (ROBstalls, 450000)
#busy_cycles = (busy_cycles, 900000)

ROBstalls_array = (ROBstalls)
busy_cycles_array = (busy_cycles)

#set graph title
plt.title('Test Plot')

#set ledgend array names
LedgendArray = ('ROB Stalls', 'Busy')

# the x locations for the groups
index = np.arange(number_of_benchmarks)

left = 0.375
width = 0.25       
opacity = 1



p1 = plt.bar(left, ROBstalls_array, width, bottom = busy_cycles, alpha=opacity, color = 'w', edgecolor = 'r', hatch= "/")
p2 = plt.bar(left, busy_cycles_array, width, bottom = 0, alpha=opacity, color = "w", edgecolor = 'black', hatch= "x")

#place the data
#plt.xticks(index + width/2.0, ('OpenCL 64x64 MM', 'OpenCL 32x32 MM'))
#plt.xticks('OpenCL 64x64 MM')

#x labels
plt.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='off')
plt.xlim([0,1])
plt.xlabel('OpenCL 64x64 MM')

plt.plot([0, busy_cycles], [0, busy_cycles], 'k-')

#y labels
y_labels = ('0', '0.2', '0.4', '0.6', '0.8', '1.0', '1.2', '1.4', '1.6', '1.8', '2.0')
plt.yticks(np.arange(0, (total_cycles * 2), (total_cycles-1)/5), y_labels)
plt.ylabel('Normalized Runtime')



#set 
plt.legend((p1[0], p2[0]), LedgendArray)

#show the plot
plt.show()