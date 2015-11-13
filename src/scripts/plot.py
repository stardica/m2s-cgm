import ConfigParser
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
#import seaborn as sns
np.random.seed(sum(map(ord, "aesthetics")))

#variables
stats_file_path = "/home/stardica/Desktop/m2s-cgm/Release/cgm_stats.out"
benchmark_path = ""
total_cycles = 0

#Math functions
def sinplot(flip=1):
    x = np.linspace(0, 14, 100)
    for i in range(1, 7):
        plt.plot(x, np.sin(x + i * .5) * (7 - i) * flip)

#read the stats file
Config = ConfigParser.ConfigParser()
Config.read(stats_file_path)
#Config.sections()
benchmark_path = Config.get('General', 'Benchmark')
#benchmark_path = ConfigSectionMap("General")['benchmark']
total_cycles = Config.getint('CPU', 'TotalCycles')

print(benchmark_path)
print("Total cycles " + repr(total_cycles))
total_cycles = total_cycles + 5
print("Total cycles blah blah " + repr(total_cycles))


#with sns.axes_style("darkgrid"):
#    plt.subplot(211)
#    sinplot()
#plt.subplot(212)
#sinplot(-1)

#main()
sinplot()
plt.show()