#ifndef GAME_OF_LIFE_GAMEFIELD_H
#define GAME_OF_LIFE_GAMEFIELD_H


#include <cstdint>
#include <cstdlib>
#include <vector>
#include "Pattern.h"

class GameField {
    int rows;
    int columns;
    int size;
    int currentGen = 1;
    std::vector<uint_fast8_t> frontField; // read only
    std::vector<uint_fast8_t> backField; // write only

    void increaseNeighbors(int row, int column);

    void decreaseNeighbors(int row, int column);

    void addToNeighbors(int row, int column, uint_fast8_t value);

public:
    GameField(int rows, int columns);

    int getRows() const;

    int getColumns() const;

    int getCurrentGen() const;

    int getIterations() const;

    uint_fast8_t cellState(int row, int column) const;

    void enable(int row, int column);

    void disable(int row, int column);

    void setCentered(const Pattern &pattern);

    int nextGeneration_raw();

    int nextGeneration();

    void print() const;

    void addToNeighbors_raw(int index, uint_fast8_t value);
};


#endif //GAME_OF_LIFE_GAMEFIELD_H
