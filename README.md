# Unrolled Linked List

![C++ Standard](https://img.shields.io/badge/C%2B%2B-20%2F23-blue.svg) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A high-performance, cache-friendly and STL-compatible container for C++.

## Overview

The standart `std::list` suffers from *pointer_chasing* and memory fragmentation, as each element is stored in its own node and every node have 2 pointers, so metadata can occupy up to 80% of memory.

`unrolled_list` deals with this problem by storing multiple elements in one node. This improves data-to-metadata ratio and also allows to contain more elements in one CPU cache-line.

## Key features

* **Caching:** by grouping elements the container minimizes cache misses. A single node can be tuned to match one CPU cache-line (usually 64 bytes).
* **Strong exception guarantees**: All operations support strong exception rules. If an exception is thrown, container remains in its previous valid state.
* **Value categories support:** Container supports both l-value and r-value.
* **Customization of node size:** A number of elements in one node is template parametrized.
* **Partial STL Compatibility:**
  * Supports `bidirectional_iterator`.
  * Compatible with `<algorithm>`.
  * Provides all standat methods, except: `emplace`, `emplace_front`, `emplace_back`, `prepend_range`.

## Usage

Since library is header-only, you just have to `include` header.

Example:

```C++
#include <unrolled_list.h>
#include <vector>
#include <list>

int main(int argc, char** argv) {
    unrolled_list<int, 10> unrolled_lst = {1, 2, 3, 4, 5, 6, 7, 8};
    std::list<int> std_list = {1, 2, 3, 4, 5, 6, 7, 8};

    auto it1 = unrolled_lst.begin();
    auto it2 = std_list.begin();

    std::advance(it1, unrolled_lst.size() / 2);
    std::advance(it2, std_list.size() / 2);
    unrolled_lst.insert(it1, {2, 3, 4, 5, 6});
    std_list.insert(it2, {2, 3, 4, 5, 6});
}

```

## Few words about iterators

As far as the container is to be cache-friendly, the elements can sometimes be shifted, or the node can be splitted into two nodes.
In this case there is the possibility for iterator invalidation:
**Iterator invalidation rules:**

* `push/pop front/back`: May invalidate iterators if the operation causes a node to split or elements to shift within a head/tail node.
* `insert/erase`:

  * **Physically**: All iterators pointing to a node that gets split or deleted are invalidated (dangling pointers).
  * **Logically**: Iterators pointing to elements within the modified node (but not invalidated physically) may now point to different elements due to internal shifting.

## Memory

Container uses `allocator_traits`, so you can use your custome allocator to reach even better results.
