#include <iostream>
#include <cassert>
#include <omp.h>
#include "GameField.h"
#include "accessors.h"
#include "Pattern.h"

GameField::GameField(int rows, int columns) :
        rows(rows),
        columns(columns),
        size(rows * columns),
        frontField(size),
        backField(frontField) {}

int GameField::getRows() const {
    return rows;
}

int GameField::getColumns() const {
    return columns;
}

int GameField::getCurrentGen() const {
    return currentGen;
}

int GameField::getIterations() const {
    // a little convenience function to get us the count nextGeneration has been called
    return currentGen - 1;
}

uint_fast8_t GameField::cellState(int row, int column) const {
    return get(frontField, columns, row, column) & 1u;
}

void GameField::enable(int row, int column) {
    getMutable(backField, columns, row, column) |= 1u;
    increaseNeighbors(row, column);
}

void GameField::disable(int row, int column) {
    getMutable(backField, columns, row, column) &= ~1u;
    decreaseNeighbors(row, column);
}

void GameField::setCentered(const Pattern &pattern) {
    assert(columns >= pattern.rows);
    assert(rows >= pattern.columns);
    int row_start = rows / 2 - pattern.rows / 2;
    int column_start = columns / 2 - pattern.columns / 2;

    for (int row = 0; row < pattern.rows; row++) {
        for (int column = 0; column < pattern.columns; column++) {
            if (get(pattern.contents, pattern.columns, row, column)) {
                enable(row_start + row, column_start + column);
            }
        }
    }
    frontField = backField;
}

int GameField::nextGeneration_raw() {
#pragma omp parallel
  {
    const int num_threads = omp_get_num_threads();
    const int thread_id = omp_get_thread_num();

    int start, end;
    if (num_threads >= size) {
        start = thread_id;
        end = thread_id + 1;
    } else {
        const double cells_per_thread = static_cast<double>(size) / num_threads;
        start = static_cast<int>(cells_per_thread * thread_id);
        end = static_cast<int>(cells_per_thread * (thread_id + 1));
    }

    int index = start;
    int next_sum = start;
    const int cells_per_sum = 256 / sizeof(uint_fast8_t);

    while (index < end) {
        if (index == next_sum) {
            int vec_sum = 0;
            int sum_to = index + cells_per_sum;
            if (sum_to > size) {
                sum_to = size;
            }
#pragma omp simd reduction (+:vec_sum)
                for (int i = index; i < sum_to; i++) {
                    vec_sum += frontField[i];
                }

            if (vec_sum == 0) {
                // we can skip over all these cells and repeat the process
                index += cells_per_sum;
                next_sum = index;
                continue;
            }
        }

        uint_fast8_t value = frontField[index];
        if (!value) {
            index += 1;
            continue;
        }
        uint_fast8_t alive = value & 1u;
        int neighbors = value >> 1u;
        if (alive) {
            if (neighbors != 2 && neighbors != 3) {
                //disable and decrease neighbors
                backField[index] &= ~1u;
                addToNeighbors_raw(index, -2);
            }
        } else if (neighbors == 3) {
            //enable and increase neighbors
            backField[index] |= 1u;
            addToNeighbors_raw(index, 2);
        }

        // we found something, so we can try summing again
        index += 1;
        next_sum = index;
    }
  }

    frontField = backField;
    return ++currentGen;
}

int GameField::nextGeneration() {
#pragma omp parallel
  {
    const int num_threads = omp_get_num_threads();
    const int thread_id = omp_get_thread_num();

    int start, end;
    if (num_threads >= rows) {
        start = thread_id;
        end = thread_id + 1;
    } else {
        const double rows_per_thread = static_cast<double>(rows) / num_threads;
        start = static_cast<int>(rows_per_thread * thread_id);
        end = static_cast<int>(rows_per_thread * (thread_id + 1));
    }


    for (int row = start; row < end; row++) {
        for (int column = 0; column < columns; column++) {
            uint_fast8_t value = get(frontField, columns, row, column);
            if (!value) continue;
            uint_fast8_t alive = value & 1u;
            int neighbors = value >> 1u;
            if (alive) {
                if (neighbors != 2 && neighbors != 3) {
                    disable(row, column);
                }
            } else {
                if (neighbors == 3) {
                    enable(row, column);
                }
            }
        }
    }
  }
    frontField = backField;

    return ++currentGen;
}

void GameField::print() const {
    for (int row = 0; row < rows; row++) {
        for (int column = 0; column < columns; column++) {
            auto current = cellState(row, column) ? "O " : "_ ";
            std::cout << current;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void GameField::increaseNeighbors(int row, int column) {
    addToNeighbors(row, column, 2);
}

void GameField::decreaseNeighbors(int row, int column) {
    addToNeighbors(row, column, -2);
}

void GameField::addToNeighbors_raw(int index, uint_fast8_t value) {
    int dAbove, dBelow, dLeft, dRight;

    if (index < columns) {
        // we are on the first row
        dAbove = size - index - index;
    } else {
        dAbove = -columns;
    }
    if (index % columns == 0) {
        // we are on the left column
        dLeft = columns - 1;
    } else {
        dLeft = -1;
    }
    if (index >= (size - columns)) {
        // we are on the bottom row
        dBelow = -(size - index - index);
    } else {
        dBelow = columns;
    }
    if ((index + 1) % columns == 0) {
        // we are on the right column
        dRight = -(columns - 1);
    } else {
        dRight = 1;
    }

    backField[index + dAbove + dLeft] += value;
    backField[index + dAbove] += value;
    backField[index + dAbove + dRight] += value;
    backField[index + dLeft] += value;
    backField[index + dRight] += value;
    backField[index + dBelow + dLeft] += value;
    backField[index + dBelow] += value;
    backField[index + dBelow + dRight] += value;
}

void GameField::addToNeighbors(int row, int column, uint_fast8_t value) {
    // Adds a value to all fields around a given index
    // This is a generalized method which should only be called from increaseNeighbors
    // and decreaseNeighbors. The only valid inputs for value would probably be
    // 2 and -2 (which increases/decreases neighbors by one preserving the cell states)
    // We're using 'raw' indices to point into the field directly, which saves us
    // from having to check the bounds for each assignment and should gain us a
    // little speed advantage
    int index = rawIndex(columns, row, column);
    int diffAbove, diffBelow, diffLeft, diffRight;

    if (row == 0) {
        diffAbove = rawIndex(columns, rows - 1, column) - index;
    } else {
        diffAbove = rawIndex(columns, row - 1, column) - index;
    }
    if (column == 0) {
        diffLeft = columns - 1;
    } else {
        diffLeft = -1;
    }
    if (row == rows - 1) {
        diffBelow = rawIndex(columns, 0, column) - index;
    } else {
        diffBelow = rawIndex(columns, row + 1, column) - index;
    }
    if (column == columns - 1) {
        diffRight = -(columns - 1);
    } else {
        diffRight = 1;
    }

    backField[index + diffAbove + diffLeft] += value;
    backField[index + diffAbove] += value;
    backField[index + diffAbove + diffRight] += value;
    backField[index + diffLeft] += value;
    backField[index + diffRight] += value;
    backField[index + diffBelow + diffLeft] += value;
    backField[index + diffBelow] += value;
    backField[index + diffBelow + diffRight] += value;
}
