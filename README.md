# Optimised Conway's Game of Life

This project is a heavily optimised C++ implementation of the classic [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) using the default 23/3 ruleset. By applying several stages of algorithm engineering, it achieves a significant performance increase over naive implementations. It was developed as a class project, with considerable inspiration taken from Michael Abrash's legendary [Graphics Programming Black Book](https://web.archive.org/web/20190706123029/http://www.drdobbs.com/parallel/graphics-programming-black-book/184404919). Graphics output is handled via the [FLTK library](https://www.fltk.org/).

As a project focused on algorithmic optimisation, it features a benchmarking facility for custom patterns, storing averaged computation times in CSV format for analysis. Patterns can be provided in [RLE format](https://www.conwaylife.com/wiki/Run_Length_Encoded).

A few sample patterns, taken from [LifeWiki](https://conwaylife.com/wiki), are provided with this project.

## But Why *Another* Game of Life?

Conway's Game of Life is a staple project for many developers. However, most implementations rarely move beyond a straightforward, cell-by-cell neighbour count. While this approach is intuitive—much like a recursive Fibonacci function—it is also highly inefficient.

In researching this project, I encountered versions of the algorithm modified for High-Performance Computing (HPC) clusters that, surprisingly, still used the same wasteful core logic. Instead of refining the underlying algorithm, these projects attempted to overcome inefficiency by simply "throwing more hardware at it." The result was often a paradoxical situation where a massively parallel implementation on a cluster performed worse than an optimised version running on a single, consumer-grade CPU.

This project takes a different path. Rather than scaling a slow algorithm, it focuses on **mechanical sympathy** and **algorithmic efficiency**. In the following sections, I will walk through the optimisation steps that eventually led to a version operating **66 times faster** than the initial baseline.

## Optimisation Steps in Detail

### Implementation Choices and Benchmarking Methodology

In its pure version, Conway's Game of Life takes place on a grid of infinite size. This exceeds the capabilities of computing equipment and requires a trade-off. A common choice is to limit the grid size to a fixed number and treat any cells beyond that limit as dead. The approach taken in this project is different: To simulate an infinite grid, it uses a **toroidal array** (wrapping around the edges). Benchmarks are conducted on a 500x500 field using the *[Eve](https://conwaylife.com/wiki/Eve)* pattern, a "methuselah" known for its longevity (30,046 generations). This provides a consistent and challenging workload.

![the Eve pattern in its initial and final state](doc/eve.png)

The program is compiled with `-Ofast` and `-march=native`. Benchmarks were performed on a Framework Laptop 13 (AMD Ryzen 7 7840U, Fedora Kinoite 41). As performance metric, the *generations per second* (GPS) value is used.

### The Starting Point: Our First Version

Our baseline follows the standard algorithm: counting eight neighbours for every cell in every generation. Even this "naive" version uses some basic optimisations, such as storing the field in a contiguous memory block to ensure good **data locality**.

This is a good point to outline some details, so we can get a better understanding of the program's inner workings and have a clearer picture on how and why the subsequent optimisations improve the execution speed.

First, let's take a look at what is required to make the Game of Life algorithm and our benchmark work:

1. A *data type* suitable for storing the state of a cell in the field.
2. A *data structure* suitable for keeping the two-dimensional field's state in our chosen type, allowing random access and modification at any index. This needs to be allocated on the heap, unless we hard-code the dimensions of our field.
3. Of that data structure, we need to have *two*: One holding the field's *current* state, and a second one storing the *next state*, i.e. the state of the following generation. In this project, we call them *front field* and *back field*, resembling double-buffering techniques used in game development.
4. Some logic iterating over our front field, determining the alive neighbours of each cell, applying the game's rules, writing the result to the back field, flipping both fields at the end of the process.
5. Benchmarking code to measure the timing of said logic, printing performance data into a file.
6. Some way to put the game's current state on the screen.

With these requirements known, here is how each of them is implemented for our baseline version:

1. In our first version, we use `bool` for storing a cell's state. As a cell can only ever be in one of two states (alive/dead), this seems the natural choice. This will change later.
2. For storing large amounts of data, an array is probably the first structure to come to mind. As dealing with "raw" arrays in C++ can be cumbersome, we choose `std::vector<bool>` instead for convenience (which is not quite optimal, as will be shown later).
   (By the way: Why use a `std::vector` if the field size isn't going to change beyond initial allocation? After all, the overhead of using a dynamic container type would seem wasteful in that case. The answer is simple: *It doesn't matter.* Internally, indexing a `std::vector` is exactly the same as indexing a raw pointer or a `std::array`, as can be verified by looking at the resulting assembly—so it's mostly a matter of taste.)
3. Both of our fields are kept in a `GameField` class. Besides holding the two fields and the field's size, this class is also responsible for determining the next state of the field (i.e., the game's logic), keeping track of the current generation number.
4. Counting the neighbours and setting each cell's next state is done using the simple approach, the logic being split up into multiple methods:
   ```cpp
   bool GameField::getElementAt(int row, int column) const {
       if (row < 0) row = getRows() - 1;
       if (row >= getRows()) row = 0;
       if (column < 0) column = getColumns() - 1;
       if (column >= getColumns()) column = 0;
       return frontField(row, column);
   }

   int GameField::neighborCount(int row, int column) const {
       int sum = 0;
       for (int y = row - 1; y <= row + 1; y++) {
           for (int x = column - 1; x <= column + 1; x++) {
               sum += getElementAt(y, x);
           }
       }
       return sum - getElementAt(row, column);
   }

   bool GameField::nextCellState(int row, int column) const {
       int neighbors = neighborCount(row, column);
       bool isAlive = getElementAt(row, column);
       return (!isAlive && neighbors == 3) || (isAlive && (neighbors == 2 || neighbors == 3));
   }

   int GameField::nextGeneration() {
       for (int i = 0; i < getRows(); i++) {
           for (int j = 0; j < getColumns(); j++) {
               bool nextState = nextCellState(i, j);
               backField.setElementAt(i, j, nextState);
           }
       }
       auto tempField = frontField;
       frontField = backField;
       backField = tempField;

       return ++current_gen;
   }
   ```
   To count a cell's neighbours, we simply use two nested `for` loops to sum up *all* surrounding cells including the cell itself, then undo the superfluous addition by subtracting the cell's value from the result. This saves us from having unroll the loop manually. The GCC compiler is actually smart enough to figure this out, emitting the same assembly code as if the additions were written out by hand.
5. For performance measurements, we make use of [OpenMP](https://www.openmp.org/), a parallel programming specification and library which we're going use again at later optimisation stages. OpenMP provides us a handy function, `omp_get_wtime()`, returning the current *wall clock time* in seconds. Wrapping our `nextGeneration()` in two calls gives the computing time of a single generation:
   ```cpp
   auto start_time = omp_get_wtime();
   field.nextGeneration();
   auto run_time = omp_get_wtime() - start_time;
   ```
   These run times are averaged over a total of 50 generations, then written to a CSV file along with the GPS value (which is `1.0 / avg`). All that functionality is kept in another class, `FieldBenchmark`, which also handles opening and finalising the output file, and printing a simple progress bar. The resulting CSV file can be opened in a data analysis framework of your choice, but for simply retrieving the overall GPS value, AWK is also sufficient:
   ```
   $ awk -F ',' '{ total += $3 } END {print total/NR}' benchmark_500x500.csv
   ```
6. Graphics output is quite simple. All that is required is to draw a bunch of colored boxes on a black background. We choose the FLTK framework to avoid having to pull in an unnecessarily complex UI library. Though it can feel a litte arcane by today's standards, it gets the job done in as little as 66 lines of code. As a handy addition, pressing the space bar allows us to start/stop the game anytime.

These are the benchmarking results for our initial version:

| Version           | Naive |
|-------------------|-------|
| Performance (g/s) | 339   |
| rel. speed-up     | 1.0   |
| abs. speed-up     | 1.0   |

### Optimisation 1: Using a Better Data Type

A common pitfall in C++ is the use of `std::vector<bool>`. As noted in the [C++ reference](https://en.cppreference.com/w/cpp/container/vector_bool.html), this is a space-optimised specialisation that stores bits. While memory-efficient, the bit manipulation required for every access is a significant performance bottleneck.

This can be seen by comparing the following two functions which simply return the element at a given index:

```cpp
int elem_at(std::vector<int> &vec, int i) {
    return vec[i];
}

bool elem_at(std::vector<bool> &vec, int i) {
    return vec[i];
}
```

This is the resulting assembly (using GCC, with optimisation enabled):
```asm
elem_at(std::vector<int, std::allocator<int>>&, int):
    mov     rax, QWORD PTR [rdi]
    movsx   rsi, esi
    mov     eax, DWORD PTR [rax+rsi*4]
    ret

elem_at(std::vector<bool, std::allocator<bool>>&, int):
    mov     rdi, QWORD PTR [rdi]
    movsx   rdx, esi
    mov     ecx, esi
    mov     eax, 1
    shr     rdx, 6
    sal     rax, cl
    and     rax, QWORD PTR [rdi+rdx*8]
    setne   al
    ret
```

By switching to `uint_fast8_t`—a type guaranteed to be the fastest 8-bit integer on the architecture—and avoiding the overhead of bit-packing, we achieved a massive leap:

| Version           | Naive | uint_fast8_t |
|-------------------|-------|--------------|
| Performance (g/s) | 339   | 1241         |
| rel. speed-up     | 1.0   | 3.66         |
| abs. speed-up     | 1.0   | 3.66         |

The result is a **3.66x** increase in speed just by swapping the underlying data type. This remains one of the most significant returns on investment in this project.


### Optimisation 2: Adding Padding to the Field

Profiling revealed that 62% of execution time was spent in index checking (handling the toroidal wrapping). By adding a "padding" layer around the field that mirrors the opposite edges, we can eliminate these conditional checks within the inner loops. This idea was originally taken from the book, and is explained in more detail there.

| Version           | Naive | uint_fast8_t | Padding |
|-------------------|-------|--------------|---------|
| Performance (g/s) | 339   | 1241         | 1543    |
| rel. speed-up     | 1.0   | 3.66         | 1.24    |
| abs. speed-up     | 1.0   | 3.66         | 4.55    |

That's a 24% gain in performance. Why not more? Well, the CPU does a tremendously good
job at [predicting the branches](https://en.wikipedia.org/wiki/Branch_predictor) for these
conditionals, keeping the computational costs low.

### Optimisation 3: Re-evaluating Abstractions

In software engineering, there is often a strong emphasis on deep class hierarchies and strict encapsulation. While beneficial for general software design, these "best practices" can introduce overhead in performance-critical paths due to function call costs and hindered compiler optimisations.

In this step, we shifted toward a more **data-oriented approach**. We removed the `SimpleMatrix` abstraction and utilised inlined accessor functions, allowing the compiler to better optimise memory access.

| Version           | Naive | uint_fast8_t | Padding | OO-- |
|-------------------|-------|--------------|---------|------|
| Performance (g/s) | 339   | 1241         | 1543    | 1665 |
| rel. speed-up     | 1.0   | 3.66         | 1.24    | 1.08 |
| abs. speed-up     | 1.0   | 3.66         | 4.55    | 4.91 |

### Optimisation 4: Changing the Approach

The most significant gain came from reconsidering the problem itself. Instead of counting neighbours for every cell from scratch, we can store the neighbour count *within* the cell bytes using the remaining bits.

![our new cell encoding, using one bit for the state, and another four to keep track of the alive neighbours](doc/cell_encoding.png)

By propagating state changes to neighbours only when a cell actually flips, we drastically reduce the workload. This approach exploits the fact that large parts of the grid are often static or empty. This idea is taken from the book as well.

The accessing logic needs to be changed like this:

1. Reading a cell's state is a bitwise AND of the value and 1: `get(frontField, columns, row, column) & 1u`.
2. Enabling a cell is a bitwise OR of the value and 1 (`get_mutable` is a newly introduced accessor
   returning a mutable reference to a cell in the field): `get_mutable(backField, columns, row, column) |= 1u`, followed by a propagation to its neighbours.
3. Disabling a cell is a bitwise AND with of the value and a bitwise "NOT 1": `get_mutable(backField, columns, row, column) &= ~1u`, again followed by propagation.
4. "Counting" the neighbours of a cell is done by shifting the value one bit to the right: `get(frontField, columns, row, column) > 1u`.
5. Increasing the neighbour count of a cell is done by adding *2* (1 shifted one bit to the left) to its value.

(Side note: Since this is essentially a different algorithm now, the padding around our field is no longer required. There is no need to visit neighbouring fields that frequently, and in the cases we do, checking for the edges works just fine. While using an adjusted form of padding might help speeding things up a bit, the impact is expected to be rather small.)

These the results:

| Version           | Naive | uint_fast8_t | Padding | OO-- | Encode |
|-------------------|-------|--------------|---------|------|--------|
| Performance (g/s) | 339   | 1241         | 1543    | 1665 | 7905   |
| rel. speed-up     | 1.0   | 3.66         | 1.24    | 1.08 | 4.75   |
| abs. speed-up     | 1.0   | 3.66         | 4.55    | 4.91 | 23.32  |

### Optimisation 5: Parallelisation

Finally, we utilised OpenMP to distribute the workload across multiple CPU cores, making sure to distribute the work evenly to prevent false sharing effects:
```cpp
#pragma omp parallel
  {
    const int num_threads = omp_get_num_threads();
    const int thread_id = omp_get_thread_num();
    const double rows_per_thread = rows / num_threads;

    const int start = static_cast<int>(rows_per_thread * thread_id);
    const int end = static_cast<int>(rows_per_thread * (thread_id + 1));

    for (int row = start; row < end; row++) {
        // (the algorithm, as usual)
    }
  }
```

The results:

| Version           | Naive | uint_fast8_t | Padding | OO-- | Encode | Threading |
|-------------------|-------|--------------|---------|------|--------|-----------|
| Performance (g/s) | 339   | 1241         | 1543    | 1665 | 7905   | 22.544    |
| rel. speed-up     | 1.0   | 3.66         | 1.24    | 1.08 | 4.75   | 2.85      |
| abs. speed-up     | 1.0   | 3.66         | 4.55    | 4.91 | 23.32  | 66.5      |

## Outlooks

While a 66-fold speedup is substantial, there is still significant headroom. The parallel efficiency (2.85x on 16 threads) suggests a **load imbalance**, as the *Eve* pattern is sparse and asymmetrical. Future iterations could implement dynamic work scheduling or active region tracking to ensure all cores are utilised effectively.

Furthermore, exploring **SIMD vectorisation** could further reduce per-cell overhead, and implementing **HashLife**—a quadtree-based approach that memoises repeating patterns—could lead to astronomical speedups for stable configurations.

Most importantly, now that the single-node performance has been pushed to a solid, highly efficient baseline, the logical next step is to transition these optimisations back to a true **HPC environment**. Having addressed the algorithmic inefficiencies at the core level, scaling this across a cluster would finally allow for a meaningful comparison of how distributed computing can push the boundaries of such complex simulations.

## Usage

This project uses CMake. Ensure FLTK and OpenMP are installed.

```bash
$ cmake -D CMAKE_BUILD_TYPE=Release -S . -B build
$ ./build/game_of_life --infile patterns/eve.rle --benchmark

```

The following parameters are supported:

* `-i`, `--infile`: The pattern file to use. This parameter is required. Some standard patterns are available in the _patterns_ directory.
* `-f`, `--fieldsize`: The size of the field in _WxH_ format. If not provided, a size of 500x500 will be used.
* `-w`, `--winsize`: The size of the window in _WxH_ format. Should be greater than or equal to the field dimensions (multiples work best). The game will try to scale the field to fit the window as good as possible. If not provided, the field size will be used.
* `-b`, `--benchmark`: Do a benchmark of the algorithm. This will disable graphics output to maximize speed. A CSV file containing the results will be generated. Be sure to also specify the `-g` and `-l` parameters below.
* `-g`, `--generations`: How many generations to run. Currently only works for benchmarking. If not provided, a default value of 30000 will be used.
* `-l`, `--logfrequency`: How many generations to average the benchmark measurements over. A value of N means the next generation is calculated N times, logging the average run time over those N iterations.  For N=1, the run time for each generation will be stored explicitly. If not provided, a default value of 50 will be used.

## References and Acknowledgements

These tools and resources were indispensable for implementing this project:

* Michael Abrash's
  [Graphics Programming Black Book](https://web.archive.org/web/20190706123029/http://www.drdobbs.com/parallel/graphics-programming-black-book/184404919)
  served as the primary inspiration for this project. Some optimisations (padding, cell encoding)
  were originally detailed there and have been adapted to modern programming environments and
  computing equipment. While a bit dated, it remains an excellent resource on software optimisation
  and is an entertaining read. If you’re interested in these topics, I encourage you to check it out!
* [Golly](https://golly.sourceforge.io/) is likely the most advanced Life simulation software
  available. It includes numerous features (a better GUI, various rule sets, scripting support,
  a pattern library, the HashLife algorithm, etc.) not found in this project. If you’re curious
  about Conway's Game of Life and want to explore it further, this is a fantastic tool.
* [LifeWiki](https://conwaylife.com/wiki/) is a comprehensive resource on all things related to the
  Game of Life. The *Eve* pattern used in this project is sourced from there, and its detailed
  description of the RLE format provided the foundation of the RLE reader implemented in this
  project.
* Matt Godbolt's [Compiler Explorer](https://godbolt.org/) is invaluable for understanding
  a compiler's behavior under varying flags and circumstances. This site simplifies the process of
  seeing how your code transforms into machine code, highlighting corresponding regions and
  providing helpful explanations. The `elem_at` example above was copied from this site.

I would also like to express my gratitude to my lecturer,
[Mark Blacher](https://www.fmi.uni-jena.de/en/18315/mark-blacher), for his guidance throughout this
project. His valuable advice helped me refine my programming style significantly, not only in terms
of program speed but also regarding best practices in general. I gained a lot from his course and
this project.

This is my second project in the field of algorithm engineering. If you're interested, I recommend
checking out my [first project](https://git.sr.ht/~p-conrad/algorithm-engineering). Unlike this
final assignment, it was developed continuously throughout the course, featuring a variety of
algorithms for selected problems. It takes a more structured, analytical approach, testing
algorithms across multiple problem types and sizes, and discussing the results in detail. It may
also serve as a library of sorts, providing basic tools for those interested in optimising
algorithms themselves.

Thank you for reading this far! I hope you find value in this project as I did. If you have any
questions, feel free to reach out!


## License

The contents of this repository are licensed under the conditions of the [MIT License](LICENSE.md).
