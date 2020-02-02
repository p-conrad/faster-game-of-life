#ifndef GAME_OF_LIFE_GAMEFIELD_H
#define GAME_OF_LIFE_GAMEFIELD_H


#include <cstdlib>
#include <vector>
#include "SimpleMatrix.h"

class GameField {
public:
    GameField(int rows, int columns);

    int getRows() const;

    int getColumns() const;

    int getCurrentGen() const;

    bool getElementAt(int row, int column) const;

    void setElementAt(int row, int column, bool value);

    void setCentered(const SimpleMatrix<bool>& values);

    bool nextCellState(int row, int column) const;

    int nextGeneration();

    void print() const;

private:
    int rows;
    int columns;
    int current_gen = 1;
    SimpleMatrix<bool> frontField;
    SimpleMatrix<bool> backField;

    int neighborCount(int row, int column) const;
};


#endif //GAME_OF_LIFE_GAMEFIELD_H
