# dmaExperiments

C++17 experiments comparing communication strategies for a parallel economic (agent-based) simulation: **DMA** (shared-memory, direct memory access) versus **MPI** (message passing).

- `include/econDMAAgents.h` — agent simulation over shared memory with DMA
- `include/econMPIAgents.h` — the same workload over MPI
- `src/main.cpp` — driver
- `test/` — doctest-based tests

Requires C++17.

# install C++17 on CentOS7
```
sudo yum install centos-release-scl
sudo yum install devtoolset-9

scl enable devtoolset-9 bash
```

# Run tests
```make test```

# Run econ simulation
```make econSim
./econSim
```
