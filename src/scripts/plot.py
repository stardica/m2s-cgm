#!/usr/bin/env python
# a stacked bar plot with errorbars
import numpy as np
import matplotlib.pyplot as plt

N = 7
menMeans = (20, 35, 30, 0, 35, 27, 32)
womenMeans = (25, 32, 34, 0, 20, 25, 32)
transMeans = (75, 75, 75, 0, 75, 75, 75)
ind = np.arange(N)    # the x locations for the groups
width = 1       # the width of the bars: can also be len(x) sequence

p3 = plt.bar(ind, transMeans, width, color='g', bottom=womenMeans)
p2 = plt.bar(ind, womenMeans, width, color='y', bottom=menMeans)
p1 = plt.bar(ind, menMeans, width, color='r')

plt.ylabel('Scores')
plt.title('Scores by group and gender')
plt.xticks(ind + width/2., ('KmeansA', 'KmeansB', 'Kmeansc'), rotation=90)
plt.yticks(np.arange(0, 150, 10))
plt.legend((p1[0], p2[0], p3[0]), ('Busy Time', 'Stall Time', 'mem lat'))

plt.show()
