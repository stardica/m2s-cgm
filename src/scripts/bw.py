
import matplotlib
import matplotlib.pyplot as plt
plt.style.use('ggplot')
import pandas as pd
import numpy as np

f = open('bandwidth_log_file.out', 'r')

num_active_cores = int(f.readline().strip())
num_cycles = int(f.readline().strip())
max_bandwidth = int(f.readline().strip())
num_epochs = int(f.readline().strip())
epoch_size = int(f.readline().strip())
pct_epochs = float(f.readline())

columns = []

for line in f:
    line = line.strip()
    columns.append(map(int, line.split()))

f.close()

df = pd.DataFrame(columns, columns=['Core_0_tx', 'Core_0_rx'])
axes = df.plot(kind='line', linewidth=0.5, colormap='seismic', title="Backprop P1 32768", rot=0)

x_major_ticks = np.arange(0, (num_epochs), (num_epochs*.10))
axes.set_xticks(x_major_ticks)

x_ticks = [
	0, 
	((num_epochs*0.1)/num_epochs)*100, 
	((num_epochs*0.2)/num_epochs)*100, 
	((num_epochs*0.3)/num_epochs)*100,
	((num_epochs*0.4)/num_epochs)*100,
	((num_epochs*0.5)/num_epochs)*100,
	((num_epochs*0.6)/num_epochs)*100, 
	((num_epochs*0.7)/num_epochs)*100, 
	((num_epochs*0.8)/num_epochs)*100,
	((num_epochs*0.9)/num_epochs)*100,
	((num_epochs*1.0)/num_epochs)*100
	]


y_major_ticks = np.arange(0, (max_bandwidth), (max_bandwidth*.10))
axes.set_yticks(y_major_ticks)

y_ticks = [
	0, 
	((max_bandwidth*0.1)/max_bandwidth)*100, 
	((max_bandwidth*0.2)/max_bandwidth)*100, 
	((max_bandwidth*0.3)/max_bandwidth)*100,
	((max_bandwidth*0.4)/max_bandwidth)*100,
	((max_bandwidth*0.5)/max_bandwidth)*100,
	((max_bandwidth*0.6)/max_bandwidth)*100, 
	((max_bandwidth*0.7)/max_bandwidth)*100, 
	((max_bandwidth*0.8)/max_bandwidth)*100,
	((max_bandwidth*0.9)/max_bandwidth)*100,
	((max_bandwidth*1.0)/max_bandwidth)*100
	]

axes.set(
	xlabel="Pct cycles",
	xticklabels=[
		'{:0.0f}%'.format(x_ticks[0]), 
		'{:0.0f}%'.format(x_ticks[1]), 
		'{:0.0f}%'.format(x_ticks[2]), 
		'{:0.0f}%'.format(x_ticks[3]), 
		'{:0.0f}%'.format(x_ticks[4]), 
		'{:0.0f}%'.format(x_ticks[5]), 
		'{:0.0f}%'.format(x_ticks[6]), 
		'{:0.0f}%'.format(x_ticks[7]), 
		'{:0.0f}%'.format(x_ticks[8]), 
		'{:0.0f}%'.format(x_ticks[9])
		],
	ylabel="Pct Max BW", 
	yticklabels=[
		'{:0.0f}%'.format(y_ticks[0]), 
		'{:0.0f}%'.format(y_ticks[1]), 
		'{:0.0f}%'.format(y_ticks[2]), 
		'{:0.0f}%'.format(y_ticks[3]), 
		'{:0.0f}%'.format(y_ticks[4]), 
		'{:0.0f}%'.format(y_ticks[5]), 
		'{:0.0f}%'.format(y_ticks[6]), 
		'{:0.0f}%'.format(y_ticks[7]), 
		'{:0.0f}%'.format(y_ticks[8]), 
		'{:0.0f}%'.format(y_ticks[9])
		]
	)

axes.grid(b=True, which='major', color='black')
#axes.grid(b=True, which='minor', color='black', linestyle='-')

plt.show()


