# Unrolled Linked List
A STL-compatible container for C++.

## Overview

## Key features
* **Caching:** what standart std:list suffers from is memory fragmentation and "pointer chasing", i.e. if you make a list of `int`, than one node contains 1 `int` and 2 `Node*` so there is only 20% of data itself. `unrolled_list` solves this problem by having multiple elements in one node. It also helps for caching as 1 node can be fully cahced by 1 cache line.
* **Strong exception guarantees**: All operations support strong exception rules
* **Value categories support:** Container supports both l-value and r-value. 
