import numpy as np

#simulator ini file paths
m2s_cgm_x86config = "/home/stardica/Desktop/m2s-cgm/src/config/Intel-i7-4790k-CPU-Config.ini"
m2s_cgm_siconfig = "/home/stardica/Desktop/m2s-cgm/src/config/Radeon-HD-7870-GPU-Config.ini"
m2s_cgm_memconfig = "/home/stardica/Desktop/m2s-cgm/src/config/cgm-config.ini"
m2s_cgm_make = "/home/stardica/Desktop/m2s-cgm/Release"
m2s_cgm_home = "/home/stardica/Desktop/m2s-cgm/Release"

#simulator paths
m2s_cgm_path = "/home/stardica/Desktop/m2s-cgm/Release/m2s-cgm"
m2s_cgm_x86args = " --x86-sim detailed --x86-config /home/stardica/Desktop/m2s-cgm/src/config/Intel-i7-4790k-CPU-Config.ini "
m2s_cgm_siargs = "--si-sim detailed --si-config /home/stardica/Desktop/m2s-cgm/src/config/Radeon-HD-7870-GPU-Config.ini " 
m2s_cgm_memargs ="--mem-config /home/stardica/Desktop/m2s-cgm/src/config/cgm-config.ini "
m2s_cgm = m2s_cgm_path + m2s_cgm_x86args + m2s_cgm_siargs + m2s_cgm_memargs

#the total number of benchmarks
num_benchmarks = 3

#the benchmarks we want to run out of all of our benchmarks
benchmarks_to_run = [0, 1, 2]

benchmark_name = np.empty(num_benchmarks, dtype=object)
benchmark_path = np.empty(num_benchmarks, dtype=object)
benchmark_args = np.empty(num_benchmarks, dtype=object)
benchmark_make = np.empty(num_benchmarks, dtype=object)

#benchmark paths

#Matrix Multiply CL
benchmark_name[0] = "MatrixMultiplyCL"
benchmark_path[0] = "/home/stardica/Desktop/MatrixMultiply/Release/MatrixMultiply "
benchmark_args[0] = ""
benchmark_make[0] = "/home/stardica/Desktop/MatrixMultiply/Release"

#Breadth-First Search OpenCL
benchmark_name[1] = "BreadthFirstSearchCL"
benchmark_path[1] = "/home/stardica/Dropbox/CDA7919DoctoralResearch/Rodinia_Benchmarks/OpenCL/bfs/bfs "
benchmark_args[1] = "/home/stardica/Dropbox/CDA7919DoctoralResearch/Rodinia_Benchmarks/data/bfs/graph128.txt"
benchmark_make[1] = "/home/stardica/Dropbox/CDA7919DoctoralResearch/Rodinia_Benchmarks/OpenCL/bfs"

#Kmeans CL
benchmark_name[2] = "KmeansCL"
benchmark_path[2] = "/home/stardica/Dropbox/CDA7919DoctoralResearch/Rodinia_Benchmarks/OpenCL/kmeans/kmeans "
benchmark_args[2] = "-o -i /home/stardica/Dropbox/CDA7919DoctoralResearch/Rodinia_Benchmarks/data/kmeans/100"
benchmark_make[2] = "/home/stardica/Dropbox/CDA7919DoctoralResearch/Rodinia_Benchmarks/OpenCL/kmeans"