# 4-Way Set-Associative Cache Simulator

A C++ implementation of a 4-way set-associative cache simulator with PLRU (Pseudo-Least Recently Used) replacement policy. This project demonstrates cache behavior under various memory access patterns and provides detailed hit/miss statistics.

##  Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Cache Parameters](#cache-parameters)
- [Access Patterns](#access-patterns)
- [PLRU Replacement Policy](#plru-replacement-policy)
- [Building and Running](#building-and-running)
- [Output](#output)
- [Code Structure](#code-structure)
- [Technical Details](#technical-details)

##  Overview

This simulator models a 4-way set-associative cache with block-level organization, implementing the PLRU replacement algorithm. It tests cache performance across different memory access patterns including sequential, round-robin, random, and strided accesses.

##  Features

- **4-Way Set-Associative Cache**: Supports 4 cache lines per set
- **PLRU Replacement Policy**: Implements Pseudo-Least Recently Used algorithm using 3-bit state structure
- **Multiple Access Patterns**: Tests sequential, round-robin, random, and strided memory access patterns
- **Performance Metrics**: Tracks cache hits, misses, and calculates hit rates
- **Cold Miss Prevention**: Preloads cache with initial blocks to eliminate cold-start misses
- **Configurable Parameters**: Easily adjustable cache size, block size, and memory size

##  Architecture

The simulator consists of four main components:

1. **`main_memory`**: Simulates byte-addressable main memory (64 KB default)
2. **`cache_line`**: Represents a single cache line with valid bit, tag, and data
3. **`cache_set`**: Contains 4 cache lines and manages PLRU bits for replacement decisions
4. **`set_associative_cache`**: Main cache controller handling reads, replacements, and statistics

##  Cache Parameters

Default configuration:
- **Memory Size**: 65,536 bytes (64 KB)
- **Cache Size**: 8,192 bytes (8 KB)
- **Block Size**: 64 bytes
- **Associativity**: 4 ways
- **Number of Sets**: 32 sets

### Address Division

For an address, the cache extracts:
- **Tag**: Identifies the block within main memory
- **Index**: Identifies which set in the cache (set number)
- **Offset**: Identifies the byte within the block

##  Access Patterns

The simulator tests four distinct access patterns:

1. **Sequential Access**: Accesses addresses in order (0, 1, 2, 3, ...)
   - Generates 100 sequential accesses starting from address 0

2. **Round-Robin Access**: Cycles through a small set of addresses
   - Rotates through addresses [0, 64, 128, 192] for 20 accesses
   - Tests temporal locality and cache reuse

3. **Random Access**: Accesses randomly distributed addresses
   - Generates 50 random addresses across the memory space
   - Tests the cache under unpredictable access patterns

4. **Strided Access**: Accesses addresses with a fixed stride
   - Accesses addresses with stride of 16 bytes for 50 accesses
   - Tests spatial locality at various strides

##  PLRU Replacement Policy

The Pseudo-Least Recently Used algorithm uses a binary tree structure with 3 bits:

```
      bit[0]
     /       \
  bit[1]   bit[2]
 /     \  /     \
way0 way1 way2 way3
```

- **Update**: When a way is accessed, PLRU bits are updated to mark it as "most recently used"
- **Victim Selection**: The algorithm traverses the tree to find the least recently used way for eviction

This provides a good approximation of LRU with O(1) complexity using only 3 bits per set.

## Building and Running

### Prerequisites

- C++ compiler with C++11 or later support (g++, clang++, etc.)
- Standard Library support for `<vector>`, `<iostream>`, `<random>`, `<cmath>`, `<map>`

### Compilation

```bash
g++ -std=c++11 -o 4_way_cache 4_way_set_associative_cache.cpp
```

### Execution

```bash
./4_way_cache
```

##  Output

The program generates detailed statistics for each access pattern:

```
Cache Stats for Sequential Access: 
Hits: 99, Misses: 1, Hit Rate: 99%

Cache Stats for Round Robin Access: 
Hits: 16, Misses: 4, Hit Rate: 80%

Cache Stats for Random Access: 
Hits: 37, Misses: 13, Hit Rate: 74%

Cache Stats for Strided Access: 
Hits: 45, Misses: 5, Hit Rate: 90%

Overall Hit Rate: 85.5%
```

##  Code Structure

```
4_way_set_associative_cache.cpp
├── Class: main_memory
│   └── Simulates byte-addressable memory
├── Class: cache_line
│   └── Represents individual cache lines
├── Class: cache_set
│   ├── Manages 4 cache lines
│   ├── updatePLRU() - Updates PLRU bits on access
│   └── findPLRUVictim() - Selects victim for eviction
├── Class: set_associative_cache
│   ├── read_from_cache() - Main cache lookup
│   ├── load_block_from_memory() - Cache fill
│   ├── preload_cache() - Initialize cache
│   └── Helper functions for tag/index/offset extraction
└── Class: TestAccessPatterns
    └── Generates various access patterns
```

##  Technical Details

### Cache Organization

- **Set Index Calculation**: `(address >> log2(block_size)) % num_sets`
- **Tag Calculation**: `address / (block_size * num_sets)`
- **Block Offset**: `address % block_size`

### Replacement Strategy

1. **Hit Path**: On cache hit, data is returned and PLRU bits are updated
2. **Miss Path**: On cache miss:
   - Find an empty line if available
   - Otherwise, use PLRU to find victim
   - Load new block from main memory
   - Update PLRU bits

### Preloading

The cache is preloaded with 100 sequential blocks to eliminate cold misses and focus on conflict and capacity misses in the test patterns.

## License

This project is provided as educational material for understanding cache architecture and replacement policies.

## Educational Value

This simulator is ideal for:
- Understanding set-associative caches
- Learning PLRU replacement algorithms
- Analyzing cache behavior under different access patterns
- Visualizing cache performance metrics

---

**Note**: This is a simplified cache simulator for educational purposes and does not account for real-world factors like write policies, coherence protocols, or multi-level caches.
