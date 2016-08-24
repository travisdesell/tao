#!/usr/bin/env python

import sys
import json
from pprint import pprint

import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Circle
import numpy as np


json_data = open(sys.argv[1])

neural_network = json.load(json_data)

#pprint(neural_network)

json_data.close()


node_set = set()
edge_set = set()
node_identifiers = dict()

hidden_layers = int(neural_network['n_hidden_layers'])
nodes_per_layer = int(neural_network['n_hidden_nodes'])
recurrent_depth = int(neural_network['recurrent_depth'])
n_nodes = len(neural_network['nodes'])
n_edges = len(neural_network['edges'])
n_recurrent_edges = len(neural_network['recurrent_edges'])

print "hidden_layers:", hidden_layers
print "nodes_per_layer:", nodes_per_layer
print "recurrent_depth: ", recurrent_depth

with open(sys.argv[1]) as nn_file:
    for node in neural_network['nodes']:
        depth = node['depth']
        layer = node['layer'] * 50

        n = node['node']
        if (layer == (hidden_layers + 1) * 50):
            n *= 76;

        identifier = node['identifier']

        node_identifiers[ (layer, (depth * nodes_per_layer) + n) ] = identifier
        node_set.add( (layer, (depth * nodes_per_layer) + n) )


    for edge in neural_network['edges']:
        src_depth = edge['src_depth']
        src_layer = edge['src_layer'] * 50
        src_node = edge['src_node']

        dst_depth = edge['dst_depth']
        dst_layer = edge['dst_layer'] * 50
        dst_node = edge['dst_node']

        if (dst_layer == (hidden_layers + 1) * 50):
            dst_node *= 76;

        weight = edge['weight']

        edge_set.add( ((src_layer, (src_depth * nodes_per_layer) + src_node), (dst_layer, (dst_depth * nodes_per_layer) + dst_node), weight) )

    for edge in neural_network['recurrent_edges']:
        src_depth = edge['src_depth']
        src_layer = edge['src_layer'] * 50
        src_node = edge['src_node']

        dst_depth = edge['dst_depth']
        dst_layer = edge['dst_layer'] * 50
        dst_node = edge['dst_node']

        if (dst_layer == (hidden_layers + 1) * 50):
            dst_node *= 76;

        weight = edge['weight']

        edge_set.add( ((src_layer, (src_depth * nodes_per_layer) + src_node), (dst_layer, (dst_depth * nodes_per_layer) + dst_node), weight) )



#print nodes
#print edges

#print "hidden layers: %d"%hidden_layers
#print "nodes per layer: %d"%nodes_per_layer

nodes = []
edges = []

i = 0
for node in node_set:
    print "node %d:"%i, node
    i += 1
    nodes.append(node)

i = 0
for edge in edge_set:
    print "edge %d:"%i, edge
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


#print("graph has %d nodes with %d edges"%(nx.number_of_nodes(U), nx.number_of_edges(U)))

def label(xy, text, direction):
    x = xy[0] - (0.8 * direction)   #shift x value
    y = xy[1] - 0.05

    if (direction >= 0):
        plt.text(x, y, text, ha="right", family='sans-serif', size=12)
    else:
        plt.text(x, y, text, ha="left", family='sans-serif', size=12)

def draw_network(G, pos, ax, sg=None):

    for n in G:
        print nodes[n]

        c = None
        if (nodes[n][0] == (hidden_layers + 1) * 50):  #this is the output node
            c = Circle(pos[n],radius=0.25,alpha=0.5, color='#540351')
        elif (nodes[n][1] >= nodes_per_layer):      #this is a recurrent node
            c = Circle(pos[n],radius=0.25,alpha=0.25 * (nodes[n][1] // nodes_per_layer), color='#B53BB1')
        elif (nodes[n][0] == 0):                    #this is a input node
            c = Circle(pos[n],radius=0.25,alpha=0.5, color='#1420CD')
        else:                                       #this is a hidden node
            c = Circle(pos[n],radius=0.25,alpha=0.5, color='#339540')

        if (nodes[n][0] == hidden_layers + 1):
            label(pos[n], node_identifiers[ nodes[n] ], -1)
        elif (nodes[n][0] == 0):
            label(pos[n], node_identifiers[ nodes[n] ],  1)

        ax.add_patch(c)
        G.node[n]['patch'] = c
        x,y = pos[n]

    seen={}

    for (u,v,d) in G.edges(data=True):
        print "iterating over edges, node:",u,v,d
        #print "node1: ", nodes[u]
        #print "node2: ", nodes[v]

        n1=G.node[u]['patch']
        n2=G.node[v]['patch']
        rad=0.3
        if (u,v) in seen:
            rad=seen.get((u,v))
            rad=(rad+np.sign(rad)*0.1)*-1
        alpha = 0.5
        #alpha = abs( d['weight'] )
        color='k'

        e = None

        if (nodes[v][0] == nodes[u][0]):
            #print("drawing curved line to recurrent!")
            e = FancyArrowPatch(n1.center,n2.center,patchA=n1,patchB=n2,
                                arrowstyle='-|>',
                                connectionstyle='arc3,rad=%s'%-rad,
                                mutation_scale=10.0,
                                lw=2,
                                alpha=alpha,
                                color=color)

        else:
            #print("drawing straight line!")
            e = FancyArrowPatch(n1.center,n2.center,patchA=n1,patchB=n2,
                                arrowstyle='-|>',
                                connectionstyle='arc3,rad=0',
                                mutation_scale=10.0,
                                lw=2,
                                alpha=alpha,
                                color=color)

        seen[(u,v)]=rad
        ax.add_patch(e)


    print "recurrent_depth: %d"%(recurrent_depth)

    for i in range(1, recurrent_depth):
        identifier = str("recurrent layer %d"%(i))
        label( (0, (i * nodes_per_layer) + (nodes_per_layer - 1) / 2.0), identifier, 1)

    plt.text(-0.5, -1, "nodes: %d, edges: %d, recurrent edges: %d"%(n_nodes, n_edges, n_recurrent_edges), ha="left", family='sans-serif', size=12)


    return e

print "setting figure size"

fig = plt.figure(1,figsize=((hidden_layers + 2) * 100, recurrent_depth * nodes_per_layer) )
print fig.get_size_inches()

print "getting pos"
pos = nx.get_node_attributes(U, 'pos')


#U=nx.MultiDiGraph([(1,2),(1,2),(2,3),(3,4),(2,4),
#                (1,2),(1,2),(1,2),(2,3),(3,4),(2,4)]
#                )

#pos=nx.spring_layout(U)

print "getting ax"
ax=plt.gca()

print "drawing network"
draw_network(U,pos,ax)



print "autoscaling"
ax.autoscale()
#plt.axis('equal')
plt.axis('off')
plt.savefig(sys.argv[2], dpi=10)
#plt.show()
