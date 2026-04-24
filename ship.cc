#include "ship.h"

// 1. Upgraded Hash Function (PC Shift + Prime Modulo)
uint32_t ship::hash_pc(uint64_t pc) {
    uint64_t shifted_pc = pc >> 2; 
    return (uint32_t)((shifted_pc ^ (shifted_pc >> 4) ^ (shifted_pc >> 8)) % 16381);
}

// 2. Constructor
ship::ship(CACHE* cache) : champsim::modules::replacement(cache), NUM_SET(cache->NUM_SET), NUM_WAY(cache->NUM_WAY), ship_state(cache->NUM_SET * cache->NUM_WAY) {
    for (int i = 0; i < 16384; i++) {
        SHCT[i] = 0;
    }
    for (long i = 0; i < NUM_SET * NUM_WAY; i++) {
        ship_state[i].signature = 0;
        ship_state[i].outcome = false;
        ship_state[i].rrpv = 3;
    }
}

// 3. Victim Selection (Eviction)
long ship::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip, champsim::address full_addr, access_type type) {
    long base_idx = set * NUM_WAY;

    while (true) {
        // Look for first block with RRPV == 3
        for (long way = 0; way < NUM_WAY; way++) {
            if (ship_state[base_idx + way].rrpv == 3) {
                return way;
            }
        }
        
        // Age all blocks if no block has RRPV == 3
        for (long way = 0; way < NUM_WAY; way++) {
            if (ship_state[base_idx + way].rrpv < 3) {
                ship_state[base_idx + way].rrpv++;
            }
        }
    }
    return 0; // Fallback
}

// 4. Miss & Insertion Logic
void ship::replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr, access_type type) {
    long idx = set * NUM_WAY + way;

    // A) Penalize the outgoing victim if it was useless
    uint32_t victim_sig = ship_state[idx].signature;
    bool victim_outcome = ship_state[idx].outcome;

    if (!victim_outcome && SHCT[victim_sig] > 0) {
        SHCT[victim_sig]--;
    }

    // B) Insert the new block
    uint64_t pc = ip.to<uint64_t>(); 
    uint32_t sig = hash_pc(pc);
    
    ship_state[idx].signature = sig;
    ship_state[idx].outcome = false;

    // C) Aggressive SHIP Prediction
    if (SHCT[sig] <= 1) {  // Punish both 0 and 1
        ship_state[idx].rrpv = 3; // "Dead on arrival" -> High eviction priority
    } else {
        ship_state[idx].rrpv = 2; // Intermediate re-reference
    }
}

// 5. Hit Logic
void ship::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr, access_type type, uint8_t hit) {
    if (hit) {
        long idx = set * NUM_WAY + way;

        // Reward logic: Block was reused
        ship_state[idx].outcome = true;
        ship_state[idx].rrpv = 0; // Promote to highest priority
        
        // Increment the SHCT counter
        uint32_t block_sig = ship_state[idx].signature;
        if (SHCT[block_sig] < 7) {
            SHCT[block_sig]++;
        }
    }
}

