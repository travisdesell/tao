#!/usr/bin/env python

import sys

import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Circle
import numpy as np


node_set = set()
edge_set = set()

hidden_layers = None
nodes_per_layer = None
min_weight = None
max_weight = None

def is_output_node(n):
    if (n[0] == hidden_layers * 2 + 1):
        return 1
    else:
        return 0

def is_recurrent_node(n):
    if (not is_output_node(n) and n[0] % 2 == 1):
        return 1
    else:
        return 0

def convert_pos(x, y):
    if is_recurrent_node((x, y)):
        return (x+1, y+nodes_per_layer)
    elif is_output_node((x, y)):
        return (x+1, y)
    else:
        return (x, y)



with open(sys.argv[1]) as nn_file:
    n = 0
    src_layer = None
    src_node = None
    for line in nn_file:
        n += 1
        if (n == 1):
            vals = line[:-1].split(' ')
            hidden_layers = int(vals[0])
            nodes_per_layer = int(vals[1])
            min_weight = float(vals[2])
            max_weight = float(vals[3])

            continue

        if (n == 2):
            continue

        vals = line[:-1].split(' ')

        if (line[0] == '#'):
            src_layer = int(vals[1])
            src_node = int(vals[2])
            #print "new source: %d %d"%(src_layer, src_node)

        else:
            dst_layer = int(vals[2])
            dst_node = int(vals[3])
            weight = float(vals[4])

            node_set.add( convert_pos(src_layer,src_node) )
            node_set.add( convert_pos(dst_layer,dst_node) )

            edge_set.add( (convert_pos(src_layer,src_node),convert_pos(dst_layer,dst_node), weight) )

#print nodes
#print edges


nodes = []
edges = []

i = 0
for node in node_set:
    #print "node %d:"%i, node
    i += 1
    nodes.append(node)

i = 0
for edge in edge_set:
    #print "edge %d:"%i, edge
    i += 1
    edges.append(edge)


U = nx.MultiDiGraph()

for i in range(0, len(nodes)):
    #print "node %d:"%i, nodes[i]
    U.add_node(i, pos=nodes[i])

for i in range(0, len(edges)):
    #print "edge %d:"%i, edges[i]
    #print "index of first node in edge: ", nodes.index(edges[i][0])
    #print "index of first node in edge: ", nodes.index(edges[i][1])
    #print "weight of edge: ", edges[i][2]
    U.add_weighted_edges_from( [(nodes.index(edges[i][0]), nodes.index(edges[i][1]), edges[i][2])] )


print("graph has %d nodes with %d edges"%(nx.number_of_nodes(U), nx.number_of_edges(U)))


def draw_network(G,pos,ax,sg=None):

    for n in G:
        #print nodes[n]

        c = None
        if (nodes[n][0] == hidden_layers * 2 + 2):  #this is the output node
            c = Circle(pos[n],radius=0.25,alpha=0.5, color='#540351')
        elif (nodes[n][1] >= nodes_per_layer):
            c = Circle(pos[n],radius=0.25,alpha=0.5, color='#B53BB1')
        elif (nodes[n][0] == 0):
            c = Circle(pos[n],radius=0.25,alpha=0.5, color='#1420CD')
        else:
            c = Circle(pos[n],radius=0.25,alpha=0.5, color='#339540')

        ax.add_patch(c)
        G.node[n]['patch']=c
        x,y=pos[n]
    seen={}

    for (u,v,d) in G.edges(data=True):
        #print "node:",u,v,d
        #print "node1: ", nodes[u]
        #print "node2: ", nodes[v]

        n1=G.node[u]['patch']
        n2=G.node[v]['patch']
        rad=0.1
        if (u,v) in seen:
            rad=seen.get((u,v))
            rad=(rad+np.sign(rad)*0.1)*-1
        #alpha = 0.5
        alpha = abs( (float(d['weight']) - min_weight) / (max_weight - min_weight) )
        color='k'

        e = None
        if (nodes[u][1] >= nodes_per_layer):
            #print("drawing curved line from recurrent!")
            e = FancyArrowPatch(n1.center,n2.center,patchA=n1,patchB=n2,
                                arrowstyle='-|>',
                                connectionstyle='arc3,rad=%s'%+rad,
                                mutation_scale=10.0,
                                lw=1,
                                alpha=alpha,
                                color=color)

        elif (nodes[v][1] >= nodes_per_layer):
            #print("drawing curved line to recurrent!")
            e = FancyArrowPatch(n1.center,n2.center,patchA=n1,patchB=n2,
                                arrowstyle='-|>',
                                connectionstyle='arc3,rad=%s'%+rad,
                                mutation_scale=10.0,
                                lw=1,
                                alpha=alpha,
                                color=color)

        else:
            #print("drawing straight line!")
            e = FancyArrowPatch(n1.center,n2.center,patchA=n1,patchB=n2,
                                arrowstyle='-|>',
                                connectionstyle='arc3,rad=0',
                                mutation_scale=10.0,
                                lw=1,
                                alpha=alpha,
                                color=color)

        seen[(u,v)]=rad
        ax.add_patch(e)
    return e

plt.figure(1,figsize=(10,10))

pos=nx.get_node_attributes(U, 'pos')


#U=nx.MultiDiGraph([(1,2),(1,2),(2,3),(3,4),(2,4),
#                (1,2),(1,2),(1,2),(2,3),(3,4),(2,4)]
#                )

#pos=nx.spring_layout(U)
ax=plt.gca()
draw_network(U,pos,ax)
ax.autoscale()
plt.axis('equal')
plt.axis('off')
plt.savefig(sys.argv[2])
#plt.show()
