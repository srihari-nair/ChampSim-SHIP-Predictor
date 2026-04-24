# Custom Signature-Based Hit Predictor (SHIP) for ChampSim

This repository contains a custom C++ implementation of the **Signature-based Hit Predictor (SHIP)** Last-Level Cache (LLC) replacement policy, built from scratch for the [ChampSim](https://github.com/ChampSim/ChampSim) cycle-accurate simulator.

This project was developed as part of the Advanced Computer Architecture coursework (M.E. Computer Science) at BITS Pilani.

## Project Overview

Traditional cache replacement policies like Least Recently Used (LRU) are highly vulnerable to cache thrashing during memory-intensive workloads (like large array scans), as they treat "dead-on-arrival" data with high priority. 

This custom SHIP implementation solves this by tracking the **Program Counter (PC)** signature of instructions that load data. By learning which instructions historically load useless data, the predictor aggressively assigns them the highest eviction priority (RRPV = 3), protecting critical data in the Last-Level Cache.

### Key Architectural Optimizations
1. **PC-Shift + Prime Modulo Hashing:** Dropped aligned zeros (`pc >> 2`) and applied a prime number modulo (`16381`) to drastically reduce hash collisions in the 16K-entry Signature History Counter Table (SHCT).
2. **Anti-Saturation (Reward Once) Logic:** Implemented a single-reward mechanism per cache-block lifetime to prevent the SHCT counters from artificially saturating and losing adaptability during program phase changes.
3. **Neutral Initialization:** Initialized the SHCT counters to give unseen Program Counters the "benefit of the doubt," preventing aggressive premature evictions during cache warmup.

## Performance Results

The custom policy was evaluated against ChampSim's baseline LRU policy using the SPEC CPU 2006 `bzip2` compression trace (20 million instructions).

| Policy | IPC | LLC Hit Rate | LLC Avg Miss Latency | Critical Load Misses |
| :--- | :--- | :--- | :--- | :--- |
| **LRU (Baseline)** | 1.673 | 46.61% | 197.6 cycles | 15,856 |
| **Custom SHIP** | **1.868** | 36.67% | **163.6 cycles** | **9,855** |

**Conclusion:** The optimized SHIP policy achieved an **11.6% IPC improvement** over LRU. While the overall hit rate decreased, this was an intentional architectural trade-off: the policy sacrificed less-important Write/RFO blocks to aggressively protect critical pipeline-stalling LOAD blocks (achieving a **37.8% reduction in critical load misses**).

## Repository Structure
* `ship.cc`: The core C++ replacement policy logic (Insertion, Hit Rewarding, Eviction Penalization).
* `ship.h`: Header file containing the 1D shadow cache state, custom metadata struct, and the SHCT predictor table.
* `Advanced_Computer_Architecture_Project_Final_Report.pdf`: The full academic write-up detailing the design space exploration, methodology, and bottleneck analysis.

## How to Run

1. Clone the official [ChampSim repository](https://github.com/ChampSim/ChampSim).
2. Place `ship.cc` and `ship.h` into the `replacement/ship/` directory (replacing the default files).
3. Ensure your `champsim_config.json` sets the LLC replacement policy to `"ship"`.
4. Compile and run:
```bash
./config.sh champsim_config.json
make -j
./bin/champsim --warmup_instructions 10000000 --simulation_instructions 20000000 traces/bzip2.trace.xz
