#ifndef GAME_OF_LIFE_ABSTRACTGAMEFIELD_H
#define GAME_OF_LIFE_ABSTRACTGAMEFIELD_H


#include "SimpleMatrix.h"

class AbstractGameField {
protected:
    int rows, columns;
    int current_gen = 1;

    virtual int neighborCount(int row, int column) const = 0;

public:
    AbstractGameField(int rows, int columns);

    int getRows() const;

    int getColumns() const;

    int getCurrentGen() const;

    virtual bool getElementAt(int row, int column) const = 0;

    virtual void setElementAt(int row, int column, bool value) = 0;

    void setCentered(const SimpleMatrix<bool>& values);

    bool nextCellState(int row, int column);

    virtual int nextGeneration() = 0;

    void print() const;
};


#endif //GAME_OF_LIFE_ABSTRACTGAMEFIELD_H
