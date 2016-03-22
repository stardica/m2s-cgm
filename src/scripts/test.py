import matplotlib
import matplotlib.pyplot as plt
plt.style.use('ggplot')
import pandas as pd
import numpy as np

print "using Matplotlib version " + matplotlib.__version__
print "using Pandas version " + pd.__version__

#df = pd.DataFrame(np.random.randn(1000, 4), index=pd.date_range('1/1/2000', periods=1000), columns=list('ABCD'))
df = pd.DataFrame(np.random.rand(10, 4), columns=['a', 'b', 'c', 'd'])

#df = df.cumsum()
#df.plot()
df.plot(kind='bar', stacked=True)
plt.show()
