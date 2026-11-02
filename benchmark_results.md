# Benchmark Results

## Methodology

I compared my custom DPLL solver against CaDiCaL using six Sudoku puzzles converted into CNF formulas.

Each solver was run 100 times per puzzle. A new assignment vector was created for each DPLL run, while a new solver instance was created for each CaDiCaL run.

I measured two execution times:

- **Solver call time:** The time taken to run `dpll()` or `solver.solve()`.
- **End to end time:** The time taken to prepare the solver and solve the CNF formula.

CNF generation was excluded from both measurements. I calculated the average and median times in milliseconds (ms).

The benchmark was compiled using g++ with C++17 and the `-O2` optimisation flag.

## Results

### Puzzle 1 — Original Sudoku (11775 clauses)

| Measurement | Custom DPLL | CaDiCaL |
|---|---:|---:|
| Solver call average | 0.2779ms | 0.0349ms |
| Solver call median | 0.2650ms | 0.0306ms |
| End to end average | 0.2781ms | 1.0387ms |
| End to end median | 0.2650ms | 1.0141ms |

### Puzzle 2 — Hard Sudoku (11766 clauses)

| Measurement | Custom DPLL | CaDiCaL |
|---|---:|---:|
| Solver call average | 25.3759ms | 8.6469ms |
| Solver call median | 24.8885ms | 8.5271ms |
| End to end average | 25.3761ms | 9.5962ms |
| End to end median | 24.8889ms | 9.4667ms |

### Puzzle 3 — Third Sudoku (11781 clauses)

| Measurement | Custom DPLL | CaDiCaL |
|---|---:|---:|
| Solver call average | 0.1641ms | 0.0332ms |
| Solver call median | 0.1582ms | 0.0285ms |
| End to end average | 0.1642ms | 0.9543ms |
| End to end median | 0.1583ms | 0.9175ms |

### Puzzle 4 — Easy Sudoku (11775 clauses)

| Measurement | Custom DPLL | CaDiCaL |
|---|---:|---:|
| Solver call average | 0.2487ms | 0.0318ms |
| Solver call median | 0.2371ms | 0.0275ms |
| End to end average | 0.2488ms | 0.9255ms |
| End to end median | 0.2372ms | 0.8886ms |

### Puzzle 5 — Medium Sudoku (11781 clauses)

| Measurement | Custom DPLL | CaDiCaL |
|---|---:|---:|
| Solver call average | 0.1370ms | 0.0274ms |
| Solver call median | 0.1358ms | 0.0262ms |
| End to end average | 0.1371ms | 0.8745ms |
| End to end median | 0.1359ms | 0.8538ms |

### Puzzle 6 — Hard Sudoku 2 (11766 clauses)

| Measurement | Custom DPLL | CaDiCaL |
|---|---:|---:|
| Solver call average | 23.2174ms | 8.1952ms |
| Solver call median | 22.9851ms | 8.0732ms |
| End to end average | 23.2176ms | 9.0932ms |
| End to end median | 22.9852ms | 8.9547ms |

## Analysis

### Solver call Performance

CaDiCaL had faster solver call times on all six puzzles. For example, on Puzzle 1, CaDiCaL took a median of 0.0306ms compared to 0.2650ms for DPLL.

The difference was also noticeable on harder puzzles. For Puzzle 2, DPLL took 24.8885ms, whilst CaDiCaL took 8.5271ms.

This will likely be because CaDiCaL uses more advanced SAT solving techniques, including Conflict-Driven Clause Learning (CDCL), efficient unit propagation and better variable selection. My DPLL implementation instead uses rather basic unit propagation and will always select the lowest numbered unassigned variable, which usually causes more backtracking.

### End to end Performance

Despite CaDiCaL having faster solver call times, my DPLL implementation had lower end to end times on four of the six puzzles.

For instance, on the third puzzle, My custom DPLL took 0.1583ms compared to 0.9175ms for CaDiCaL, making it approximately 5.8 times faster overall.

This is mainly due to the difference in setup overhead. My DPLL solver works directly with the CNF formula stored in a vector and only needs an assignment vector before solving. CaDiCaL needs to initialise a solver instance and load the clauses into its internal data structures.

However, on the harder puzzles, CaDiCaL's faster solving performance outweighed this overhead. On Puzzle 6, CaDiCaL was approximately 2.57 times faster End to end.

### Implementation vs Algorithm Performance

An important distinction is that these benchmarks compare the performance of two implementations, rather than directly comparing the DPLL and CDCL algorithms.

Execution time is affected by the algorithm, but also by data structures, code optimisation and solver setup.

My DPLL implementation benefits from being lightweight, which makes it faster overall on puzzles requiring little searching. CaDiCaL has more setup overhead, but its advanced solving techniques appear to provide an advantage on harder puzzles.

Therefore, the results do not mean that DPLL is algorithmically faster than CDCL. They instead show that a simpler implementation can sometimes complete a small task faster when setup time is included.

## Limitations

There are a few limitations to these benchmarks.

CaDiCaL may perform preprocessing while clauses are being loaded, meaning the solver call times are not a completely direct comparison of algorithm performance.

The end to end measurements also include different preparation steps for each implementation, so the results depend on how the solvers are being used.

Although I tested six puzzles with 100 runs each, more puzzles would provide a better comparison. The results may also be affected by the computer being used and other programs running in the background.

## Conclusion

Overall, my DPLL implementation had faster end to end times on four of the six puzzles, while CaDiCaL was faster on the two harder puzzles.

CaDiCaL consistently had faster solver call times, but its additional setup overhead affected its overall performance on the easier puzzles.

The results highlight the difference between algorithm performance and practical implementation speed. Although CaDiCaL uses more advanced solving techniques, my simpler implementation can be faster overall for certain Sudoku instances.

If I were to improve the project further, I would investigate better variable selection methods and measure the number of decisions and backtracks made by my DPLL solver.