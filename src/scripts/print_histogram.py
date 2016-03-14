#!/usr/bin/env python
import ConfigParser
from optparse import OptionParser
import matplotlib
import matplotlib.pyplot as plt
plt.style.use('ggplot')
import pandas as pd
import numpy as np

store_list = []
with open("store_lat_log_file.out") as stat_file:
    for line in stat_file:
        key, value = line.partition(" ")[::2]
        store_list.append(int(value.rstrip()))

#x,y = zip(*store_list)

#print store_list


#np.asarray(store_list)

#print store_list

#print np.random.randint(1, 100, 1000)

ds = pd.Series(store_list)


df4 = pd.DataFrame({'stores' : ds}, columns=['stores'])



#y_major_ticks = np.arange(0, (total_cycles*1.4), (total_cycles*.20))
#axes.set_yticks(y_major_ticks)
#y_ticks = [0, total_cycles*0.2, total_cycles*0.4, total_cycles*0.6, total_cycles*0.8, total_cycles*1.0, total_cycles*1.2, total_cycles*1.4]

#axes.set(xlabel="Core", ylabel="Percent of Cycles", yticklabels=['{:0.2f}'.format(y_ticks[0]/total_cycles), '{:0.2f}'.format(y_ticks[1]/total_cycles), '{:0.2f}'.format(y_ticks[2]/total_cycles), '{:0.2f}'.format(y_ticks[3]/total_cycles), '{:0.2f}'.format(y_ticks[4]/total_cycles), '{:0.2f}'.format(y_ticks[5]/total_cycles), '{:0.2f}'.format(y_ticks[6]/total_cycles), '{:0.2f}'.format(y_ticks[7]/total_cycles)])
#axes.grid(b=True, which='major', color='black', linestyle='--')
#axes.grid(b=True, which='minor', color='black', linestyle='--')
#axes.legend(loc='upper right', ncol=4)




axes = df4.plot(kind='hist', bins=100, cumulative='true')
axes.grid(b=True, which='major', color='black', linestyle='--')
axes.grid(b=True, which='minor', color='black', linestyle='--')
axes.legend(loc='upper right', ncol=4)

plt.show()
