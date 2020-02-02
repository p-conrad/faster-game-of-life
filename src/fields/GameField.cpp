#include "GameField.h"

GameField::GameField(int rows, int columns) :
        AbstractGameField(rows, columns),
        frontField(rows, columns),
        backField(rows, columns) {}

bool GameField::getElementAt(int row, int column) const {
    if (row < 0) row = getRows() - 1;
    if (row >= getRows()) row = 0;
    if (column < 0) column = getColumns() - 1;
    if (column >= getColumns()) column = 0;
    return frontField(row, column);
}

void GameField::setElementAt(int row, int column, bool value) {
    frontField.setElementAt(row, column, value);
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
