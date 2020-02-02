#ifndef GAME_OF_LIFE_GAMEFIELD_H
#define GAME_OF_LIFE_GAMEFIELD_H


#include <cstdlib>
#include <vector>
#include "SimpleMatrix.h"
#include "AbstractGameField.h"

class GameField : public AbstractGameField {
protected:
    SimpleMatrix<bool> frontField;
    SimpleMatrix<bool> backField;

    int neighborCount(int row, int column) const override;

public:
    GameField(int rows, int columns);

    bool getElementAt(int row, int column) const override;

    void setElementAt(int row, int column, bool value) override;

    int nextGeneration() override;
};


#endif //GAME_OF_LIFE_GAMEFIELD_H
