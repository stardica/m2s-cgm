import subprocess
import sys
import time
import ConfigParser
import numpy as np
import matplotlib as mpl
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

#import evertyhing from the paths file
from paths import *

#import seaborn as sns
#np.random.seed(sum(map(ord, "aesthetics")))

# notes
# changes text to int with repr()
# get all sections from config file with Config.sections()
# keep upper case letters in an ini file memconfig.optionxform = str

#stat veriables
total_cycles = 0
ROBstalls = 0
busy_cycles = 0


#make the simulator
m2s_cgm_build = subprocess.Popen(["make clean all -C " + m2s_cgm_make], shell = True)
m2s_cgm_build.wait()

#run benchmarks
for i in benchmarks_to_run:
    
    #build the target benchmark
    benchmark_build = subprocess.Popen(["make clean all -C " + benchmark_make[i]], shell = True)
    benchmark_build.wait()
    
    #set the benchmark path and args
    bench_name = benchmark_name[i]
    bench = m2s_cgm + benchmark_path[i] + benchmark_args[i]
        
    #set benchmark name in simulator ini
    memconfig = ConfigParser.ConfigParser()
    memconfig.optionxform = str
    memconfig.read(m2s_cgm_memconfig)
    memconfig.set("Stats", "File_Name", benchmark_name[i])
    with open(m2s_cgm_memconfig, 'wb') as memconfigfile:
        memconfig.write(memconfigfile)
        
    cores = [1, 2, 4]
    for i in cores:
                       
        #set number of cores in simulator ini
        x86config = ConfigParser.ConfigParser()
        x86config.optionxform = str
        x86config.read(m2s_cgm_x86config)
        x86config.set("General", "Cores", i)
        with open(m2s_cgm_x86config, 'wb') as x86configfile:
            x86config.write(x86configfile)
            
        #run the benchmark(s)
        benchmark_run = subprocess.Popen([bench], shell = True)
        benchmark_run.wait()
        #check to make sure benchmark completed successfully.
        if benchmark_run.returncode == 1:
            sys.exit()

    continue

    for i in cores:
        #get the stats file path
        stats_file = m2s_cgm_home + "/" + bench_name +"_p" + str(i)
                       
        #read the stats file
        stats = ConfigParser.ConfigParser()
        stats.optionxform = str
        stats.read(stats_file)
             
        #get specific values
        total_cycles = stats.getint('CPU', 'TotalCycles')
        ROBstalls = stats.getint('CPU', 'ROBStalls')
        
        busy_cycles = total_cycles - ROBstalls 

        ROBstalls_array = (ROBstalls)
        busy_cycles_array = (busy_cycles)
        
        #set graph title
        plt.title(bench_name)

        #set ledgend array names
        LedgendArray = ('ROB Stalls', 'Busy')
        
        number_of_benchmarks = 1

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
