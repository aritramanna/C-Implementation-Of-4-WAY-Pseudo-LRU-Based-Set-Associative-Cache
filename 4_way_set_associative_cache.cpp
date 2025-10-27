#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <cmath>
#include <map>

using namespace std;

#define NUM_WAYS 4

// Main memory class simulating a simple byte-addressable memory
class main_memory {
public:
    size_t size;
    vector<uint8_t> memory_array;

    main_memory(size_t size) {
        this->size = size;
        memory_array.resize(size);
        for (size_t i = 0; i < size; i++) {
            memory_array[i] = i;
        }
    }
};

// Represents a single cache line in a set-associative cache
class cache_line {
public:
    bool valid;
    size_t tag;
    vector<uint8_t> cache_data;
    
    cache_line(size_t block_size) {
        valid = false;
        tag = 0;
        cache_data.resize(block_size, 0);
    }
};

// Represents a single set in a set-associative cache
class cache_set {
public:
    vector<cache_line> lines; // Stores multiple cache lines
    vector<bool> plru_bits;   // 3-bit PLRU structure for 4-way associativity

    cache_set(size_t block_size) {
        plru_bits.resize(3, false);
        lines.resize(NUM_WAYS, cache_line(block_size));
    }

    // Updates PLRU bits based on the accessed way
    void updatePLRU(int accessedWay) {
        plru_bits[0] = accessedWay >= 2;
        plru_bits[1] = accessedWay % 2;
        plru_bits[2] = (accessedWay / 2) % 2;
    }

    // Determines which cache way should be replaced using PLRU policy
    int findPLRUVictim() {
        return (plru_bits[0] ? (plru_bits[2] ? 3 : 2) : (plru_bits[1] ? 1 : 0));
    }
};

// Implements a 4-way set-associative cache with PLRU replacement policy
class set_associative_cache {
public:
    size_t num_sets, block_size;
    vector<cache_set> sets;
    int cache_hits, cache_misses, total_accesses;
    map<string, double> hit_rates;
    main_memory& memory; // Reference to main memory
    
    set_associative_cache(size_t block_size, size_t cache_size, main_memory& main_mem)
        : memory(main_mem) {
        this->block_size = block_size;
        this->num_sets = cache_size / (NUM_WAYS * block_size);
        this->cache_hits = 0;
        this->cache_misses = 0;
        this->total_accesses = 0;
        this->sets.resize(num_sets, cache_set(block_size));
    }

    void reset_cache_stats() {
        cache_hits = 0;
        cache_misses = 0;
    }

    // Extracts the tag from the given memory address
    size_t extract_tag(size_t address) {
        return address / (block_size * num_sets);
    }

    // Extracts the index (set number) from the given memory address
    size_t extract_index(size_t address) {
        return (address >> (size_t)log2(block_size)) % num_sets;
    }

    // Extracts the block offset from the given memory address
    size_t extract_block_offset(size_t address) {
        return address % block_size;
    }

    // Loads a memory block into the cache
    void load_block_from_memory(size_t address, int way) {
        size_t set_idx = extract_index(address);
        size_t tag = extract_tag(address);
        size_t block_start = (address / block_size) * block_size;
        
        sets[set_idx].lines[way].valid = true;
        sets[set_idx].lines[way].tag = tag;
        for (size_t i = 0; i < block_size; i++) {
            sets[set_idx].lines[way].cache_data[i] = memory.memory_array[block_start + i];
        }
    }

    // Preloads blocks into the cache to prevent cold misses
    void preload_cache(size_t start_address, size_t num_blocks) {
        for (size_t i = 0; i < num_blocks; ++i) {
            size_t address = start_address + i * block_size;
            size_t set_idx = extract_index(address);
            int evictWay = -1;

            // Check for an empty line to preload
            for (int j = 0; j < NUM_WAYS; ++j) {
                if (!sets[set_idx].lines[j].valid) {
                    evictWay = j;
                    break;
                }
            }

            // If no empty line, find a victim using PLRU
            if (evictWay == -1) {
                evictWay = sets[set_idx].findPLRUVictim();
            }

            load_block_from_memory(address, evictWay);
            sets[set_idx].updatePLRU(evictWay);
        }
    }

    // Reads data from the cache and applies PLRU replacement if needed
    uint8_t read_from_cache(size_t address) {
        size_t set_idx = extract_index(address);
        size_t tag = extract_tag(address);
        size_t block_offset = extract_block_offset(address);

        total_accesses++;
        for (int i = 0; i < NUM_WAYS; ++i) {
            if (sets[set_idx].lines[i].valid && sets[set_idx].lines[i].tag == tag) {
                cache_hits++;
                sets[set_idx].updatePLRU(i);
                return sets[set_idx].lines[i].cache_data[block_offset];
            }
        }

        // Cache miss: find a victim and load block from memory
        cache_misses++;
        int evictWay = -1;
        for (int i = 0; i < NUM_WAYS; ++i) {
            if (!sets[set_idx].lines[i].valid) {
                evictWay = i;
                break;
            }
        }
        if (evictWay == -1) {
            evictWay = sets[set_idx].findPLRUVictim();
        }
        load_block_from_memory(address, evictWay);
        sets[set_idx].updatePLRU(evictWay);
        return sets[set_idx].lines[evictWay].cache_data[block_offset];
    }

    // Prints cache performance statistics
    void print_cache_stats(const string& pattern) {
        double hit_rate = (cache_hits * 100.0) / (cache_hits + cache_misses);
        hit_rates[pattern] = hit_rate;
        cout << "\nCache Stats for " << pattern << ": "
             << "Hits: " << cache_hits << ", Misses: " << cache_misses
             << ", Hit Rate: " << hit_rate << "%\n";
    }
};

// Class to generate different memory access patterns
class TestAccessPatterns {
public:
    // Function to generate sequential access pattern
    static vector<size_t> generate_sequential_access(size_t start, size_t count) {
        vector<size_t> addresses;
        for (size_t i = 0; i < count; ++i) {
            addresses.push_back(start + i);
        }
        return addresses;
    }

    // Function to generate round robin access pattern
    static vector<size_t> generate_round_robin_access(const vector<size_t>& base_addresses, size_t repetitions) {
        vector<size_t> addresses;
        for (size_t i = 0; i < repetitions; ++i) {
            addresses.push_back(base_addresses[i % base_addresses.size()]);
        }
        return addresses;
    }

    // Function to generate random access pattern
    static vector<size_t> generate_random_access(size_t count, size_t memory_size) {
        vector<size_t> addresses;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<size_t> dist(0, memory_size - 1);
        for (size_t i = 0; i < count; ++i) {
            addresses.push_back(dist(gen));
        }
        return addresses;
    }

    // Function to generate strided access pattern
    static vector<size_t> generate_strided_access(size_t start, size_t stride, size_t count) {
        vector<size_t> addresses;
        for (size_t i = 0; i < count; ++i) {
            addresses.push_back(start + i * stride);
        }
        return addresses;
    }
};

int main() {
    size_t memory_size = 65536, cache_size = 8192, block_size = 64;
    main_memory memory(memory_size);
    set_associative_cache cache(block_size, cache_size, memory);
    
    // Preload cache with sequential addresses to prevent cold misses
    cache.preload_cache(0, 100); // Preload the first 100 blocks

    // Variables to track overall hits and misses
    int overall_hits = 0;
    int overall_misses = 0;

    // Test access patterns
    vector<size_t> sequential_addresses = TestAccessPatterns::generate_sequential_access(0, 100);
    vector<size_t> round_robin_addresses = TestAccessPatterns::generate_round_robin_access({0, 64, 128, 192}, 20);
    vector<size_t> random_addresses = TestAccessPatterns::generate_random_access(50, memory_size);
    vector<size_t> strided_addresses = TestAccessPatterns::generate_strided_access(0, 16, 50);

    // Run tests for each access pattern
    cache.reset_cache_stats();
    for (size_t addr : sequential_addresses) {
        cache.read_from_cache(addr);
    }
    cache.print_cache_stats("Sequential Access");
    overall_hits += cache.cache_hits;
    overall_misses += cache.cache_misses;

    cache.reset_cache_stats();
    for (size_t addr : round_robin_addresses) {
        cache.read_from_cache(addr);
    }
    cache.print_cache_stats("Round Robin Access");
    overall_hits += cache.cache_hits;
    overall_misses += cache.cache_misses;

    cache.reset_cache_stats();
    for (size_t addr : random_addresses) {
        cache.read_from_cache(addr);
    }
    cache.print_cache_stats("Random Access");
    overall_hits += cache.cache_hits;
    overall_misses += cache.cache_misses;

    cache.reset_cache_stats();
    for (size_t addr : strided_addresses) {
        cache.read_from_cache(addr);
    }
    cache.print_cache_stats("Strided Access");
    overall_hits += cache.cache_hits;
    overall_misses += cache.cache_misses;

    // Calculate and print overall hit rate
    double overall_hit_rate = (overall_hits * 100.0) / (overall_hits + overall_misses);
    cout << "\nOverall Hit Rate: " << overall_hit_rate << "%\n";

    return 0;
}