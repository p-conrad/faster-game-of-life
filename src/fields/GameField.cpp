#include <iostream>
#include "GameField.h"

GameField::GameField(int rows, int columns) :
        rows(rows),
        columns(columns),
        frontField(rows, columns),
        backField(rows, columns) {}

int GameField::getRows() const {
    return rows;
}

int GameField::getColumns() const {
    return columns;
}

int GameField::getCurrentGen() const {
    return current_gen;
}

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

void GameField::setCentered(const SimpleMatrix<bool>& values) {
    assert(getColumns() >= values.getColumns());
    assert(getRows() >= values.getRows());
    int y_start = getRows() / 2 - values.getRows() / 2;
    int x_start = getColumns() / 2 - values.getColumns() / 2;

    for (int y = 0; y < values.getRows(); y++) {
        for (int x = 0; x < values.getColumns(); x++) {
            setElementAt(y_start + y, x_start + x, values.getElementAt(y, x));
        }
    }
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

    // classic 23/3 rules
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

void GameField::print() const {
    for (int row = 0; row < getRows(); row++) {
        for (int column = 0; column < getColumns(); column++) {
            auto current = getElementAt(row, column) ? "O " : "_ ";
            std::cout << current;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
