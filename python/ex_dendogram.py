import numpy as np
import matplotlib.pyplot as plt
from scipy.cluster.hierarchy import dendrogram
import tspg

def mergeOrderToScipyFormat(mergeOrder):
	maxi = len(mergeOrder) + 1
	translate={}
	Z = []
	for x1,x2,dist,sz in mergeOrder:
		x1=round(x1)
		x2=round(x2)
		sz=round(sz)
		a=x1;b=x2
		while x1 in translate:
			x1=translate[x1]
		while x2 in translate:
			x2=translate[x2]
		Z.append([x1, x2, dist, sz])
		translate[x1] = maxi
		translate[x2] = maxi
		maxi+=1
	return Z
	
	
# np.random.seed(23423)
# ds = np.random.rand(30, 2)
ds = np.genfromtxt('data/s1_small.txt')
labels,mergeOrder = tspg.tspg(ds.tolist(),1,distance="l2",num_tsp=5,dtype="vec")

mergeOrder_scipy = mergeOrderToScipyFormat(mergeOrder)

fig, axs = plt.subplots(1, 2, figsize=(12, 6), gridspec_kw={'width_ratios': [1, 1]})

plt1 = axs[0]; plt2 = axs[1]

# Create 2d plot showing the  merges
plt1.scatter(ds[:, 0], ds[:, 1], marker='o', color='b')
for pair in mergeOrder:
	plt1.plot([ds[pair[0],0], ds[pair[1],0]] , [ds[pair[0],1], ds[pair[1],1]], 'k-')
plt1.set_title('Merge Order')

# Plot the dendrogram
dendrogram(mergeOrder_scipy,ax=plt2,no_labels=True)
plt2.set_title('Hierarchical Clustering Dendrogram')
plt.show()
