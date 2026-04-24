#ifndef REPLACEMENT_SHIP_H
#define REPLACEMENT_SHIP_H

#include "cache.h"
#include "modules.h"
#include <vector>

// Inherit from the modern ChampSim modules namespace
class ship : public champsim::modules::replacement {
private:
    long NUM_SET, NUM_WAY;

    // Metadata for each cache block
    struct ship_metadata {
        uint32_t signature;
        bool outcome;
        uint8_t rrpv;
    };

    // 1D vector to shadow the 2D cache
    std::vector<ship_metadata> ship_state;
    
    // The Signature History Counter Table (SHCT)
    int SHCT[16384];

    // Simple hash function declaration
    uint32_t hash_pc(uint64_t pc);

public:
    // Constructor
    explicit ship(CACHE* cache);

    // ChampSim API (No 'override' keywords here!)
    long find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip, champsim::address full_addr, access_type type);
    
    void replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr, access_type type);
    
    void update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr, access_type type, uint8_t hit);
};

#endif

