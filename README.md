## Inspiration
The Algorithm was taken from the paper "Multi-Commodity Flow-Based Spreading in a Commercial Analytic Placer" by Nima Karimpour Darav, Andrew Kennings, Kristofer Vorwerk, and Arun Kundu. In Proceedings of the 2019 ACM/SIGDA International Symposium on Field-Programmable Gate Arrays (FPGA '19).

## What it does
This project is divided into 2 parts, the placer and router for FPGA cells.The placer contains a branch-and-bound partitioning component as well as an analytical placer that determines the final position of the partitioned cells. The router will then operate on the placement results and determine the connections between each cells.

### The FPGA Architecture
The general FPGA architecture consists of three types of modules. They are I/O blocks or Pads, Switch Matrix/ Interconnection Wires and Configurable logic blocks (CLB). The basic FPGA architecture has two dimensional arrays of logic blocks with a means for a user to arrange the interconnection between the logic blocks.

In my project, for simplicity and proof of concept, I have generalized all types of blocks that could be present on an FPGA and treated them the same way, as generalized modules (referred to as "cells" in this post) that require connection with each other through switch matrix and interconnection wires. The FPGA is then simplified to an 2D array of cells surrounded by programmable routing components.

### The Placer

#### Branch-and-Bound Partitioning
The partitioning component has the following specification:

Inputs:
- Netlist which contains
    - All cells
    - Connections between each cells

Outputs:
- Bipartition of the input cells

Constraints:
- The cells contained in either partition do not appear in other partition
- The two partitions should contain all cells that are present in the netlist input
- Minimize the number of connections between the cells in each partition

The partitioning component will partition the given cells in the netlists into 2 partitions. This could be done into more partitions, but since my project is a proof-of-concept exercise, the results will only be partitioned into 2.

The partitioner will first generate a number of random partition and pick the best scoring result and move forward. The partitioning decision tree is then designed as a binary tree, and each node of the tree represents a partial solution. A bounding function is then designed to compute the lower bound on the cost of all solutions below a given node. The bounding function I used is cut size of the current partial solution + the minimum cut size increase by adding each un-visited cell to either side of the current partitions.

The partition results are also represented graphically as a inverted tree, with each level representing the partition decision for a specific cell. If the cell is put into the left partition, it will be represented as a node on the left, the same applies for the right. The optimal solution is highlighted in red.

![](https://d112y698adiu2z.cloudfront.net/photos/production/software_photos/003/187/005/datas/original.png)
*Partitioner Results*

#### Analytical Placer
The analytical placer then can operate on each of the partitions, and has the following specification:

Inputs:
- Partitioned netlist which contains
    - All cells
    - Connections between each cells

Outputs:
- Position of all cells

Constraints:
- Minimize the wire length between all cells

The analytical placer will solve the problem in two parts, first the placer will perform analytical placement (AP) to determine the initial placement using the clique model, then it will spread all cells following the multi-commodity based spreading algorithm to achieve legal placement.

##### Analytical Placement
This step will compute a placement of all cells in one shot by solving a quadratic placement problem such that it minimizes the cost of the placement. The cost function is written as the half perimeter wirelength (HPWL) between each pair of cells in the partition. By solving the quadratic cost function using derivitives, the results will be shown graphically. 
- Each gray box represents a cell.
- Each green box represents a anchor cell, for which the placement location cannot be changed.
- Each pink box represents a blockage, for which no cell can be placed on top.
- Each line connecting two cells represents a connection exists between them. (can be turned off for visual de-cluttering)

![](https://d112y698adiu2z.cloudfront.net/photos/production/software_photos/003/187/012/datas/original.png)

*Initial AP Result*

##### Multi-Commodity Based Spreading
This step will start from the AP result and iteratively apply the multi-commodity based spreading algorithm to move each cell into a legal placement position (no overlap and not on any blockage). For each iteration, the algorithm determines which bins are overfilled (overlap of cells) and determine the optimal path to move each cell in that bin. The algorithm finishes when no overfilled bin is present anymore.

![](https://d112y698adiu2z.cloudfront.net/photos/production/software_photos/003/187/013/datas/original.png)

*Spreading Result*

#### Lee-Moore Maze Router
The router for this project uses the Lee-Moore Maze routing algorithm, which is an extension on Dijkstra's algorithm. And has the following specification:

Inputs:
- Partitioned netlist which contains
    - All cells
    - Connections between each cells
- Position of each cell and their pins from the placer

Outputs:
- Paths for the connections so all connections in the netlist are made.

Constraints:
- Minimize the overall wire length
- No shorts between connections

The algorithm will first extract all connections to be routed, and for each of the connection it will perform a djikstra-like path finding algorithm starting from the sink to the source. The routing results will then be displayed graphically. In the below example, each group of 2 gray boxes represents a cell, this simulates the situation where there are more than one module in a cell that requires connection; each line segment around the cells represents an available wire segment. Each wire segment can only be used for one connection to avoid shorts.

![](https://d112y698adiu2z.cloudfront.net/photos/production/software_photos/003/186/156/datas/original.png)
*Routing result for a 10x14 cells design*


## How I built it
This project is written in C++ and using an open-source graphics library easygl developed by University of Toronto. This library has allowed easy demonstration of results in graphics form.

## Challenges I ran into
It is difficult to find and debug graph algorithms, especially when I have misunderstandings of the algorithms I am trying to implement.

## What I learned
The general flow of the back-end layout syntheses for FPGAs. This project has exposed me to general ASIC front-to-back design flow as well. This area is explored in another one of my posts "Low Power CMOS Image Sensor", where a CIS chip is developed from design to tapeout.

## What's next for Analytical Placer for FPGA Blocks
- The partitioning B&B algorithm although produces an optimal solution, is not scalable at O(2^n) complexity. It is better to have a heuristic algorithm such as Fiduccia–Mattheyses algorithm.
- Implement heuristic based placing algorithm such as simulated annealing to provide more options.
- Incorporate timing-driven routing and optimize for the longest connection path in circuit.
