import os
import numpy as np
import matplotlib.pyplot as plt
from typing import Iterable, List, Tuple
from scipy.cluster.hierarchy import dendrogram
import tspg

# This code demonstrates the calculation of multiple clusterings of different
# numebrs of clusters using the merge order

def mergeOrderToScipyFormat(mergeOrder):
  # In input mergeorder node id's are reused in a merge
  # Scipy requires merged node to have different id than child nodes
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
	
def labels_from_merges(children: Iterable[Tuple[int, int]], n_clusters: int) -> np.ndarray:
    """
    Convert an agglomerative merge order into flat cluster labels.

    Parameters
    ----------
    children : array-like of shape (n_samples - 1, 2+)
        Merge order. Row i gives the pair of node IDs that were merged to
        create node (n_samples + i). Leaf nodes are 0..n_samples-1, and
        merged (internal) nodes are n_samples..(2*n_samples-2).
        (This matches scipy.cluster.hierarchy/AgglomerativeClustering.children_.)
    n_clusters : int
        Desired number of flat clusters to cut the tree into (1..n_samples).

    Returns
    -------
    labels : np.ndarray of shape (n_samples,)
        Integer labels 0..(n_clusters-1).
    """
    children = np.asarray(children, dtype=int)
    if children.ndim < 2 or not (children.shape[1] >= 2):
        raise ValueError("children must be array-like with shape (n_samples-1, 2)")
    n = children.shape[0] + 1
    if not (1 <= n_clusters <= n):
        raise ValueError(f"n_clusters must be in [1, {n}]")

    # Map internal node id -> its two children
    node_children = {n + i: (int(children[i, 0]), int(children[i, 1]))
                     for i in range(n - 1)}

    # Start from the root (last formed node) and split until we have k clusters.
    root = 2 * n - 2
    clusters: List[int] = [root]  # a list of current cluster node IDs

    while len(clusters) < n_clusters:
        cid = max(clusters)
        if cid < n:
            # We have only leaves but still need more clusters -> impossible
            raise ValueError("Cannot split further to reach requested n_clusters.")
        left, right = node_children[cid]
        # Replace the chosen composite node with its two children
        idx = clusters.index(cid)
        clusters[idx:idx+1] = [left, right]

    # Gather leaves for each final cluster and assign labels
    labels = np.empty(n, dtype=int)

    def assign_leaf_nodes(node_id: int, label: int):
        # Iterative DFS to avoid recursion depth concerns
        stack = [node_id]
        while stack:
            u = stack.pop()
            if u < n:            # leaf
                labels[u] = label
            else:                # internal
                stack.extend(node_children[u])

    for label, node_id in enumerate(clusters):
        assign_leaf_nodes(node_id, label)

    return labels
	
def show_clusters_2d(x,labels,numclu):
	colormap = plt.cm.gist_ncar
	colorst = [colormap(i) for i in np.linspace(0, 0.9,numclu)]
	# print(colorst)
	u_labels = np.unique(labels)
	for i in u_labels:
		plt.scatter(x[labels == i , 0] , x[labels == i , 1] , label = i, color = colorst[i-1])
	plt.show()
	
# np.random.seed(23423)
# ds = np.random.rand(100, 2)
ds = np.genfromtxt(os.path.join(os.path.dirname(__file__), '../data/s1_small.txt'))
labels,mergeOrder = tspg.tspg(ds,1,distance="l2",num_tsp=5,dtype="vec")

mergeOrder_scipy = mergeOrderToScipyFormat(mergeOrder)

# Get labels with three clusters using the mergeorder
numclu=3
labels=labels_from_merges(mergeOrder_scipy, n_clusters=numclu) 
show_clusters_2d(ds,labels,numclu)


# Get labels with 15 clusters using the mergeorder
numclu=15
merge2 = mergeOrder_scipy
labels=labels_from_merges(mergeOrder_scipy, n_clusters=numclu) 
show_clusters_2d(ds,labels,numclu)

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
