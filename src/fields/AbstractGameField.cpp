#include <iostream>
#include "AbstractGameField.h"

AbstractGameField::AbstractGameField(int rows, int columns) : rows(rows), columns(columns) {}

int AbstractGameField::getRows() const {
    return rows;
}

int AbstractGameField::getColumns() const {
    return columns;
}

int AbstractGameField::getCurrentGen() const {
    return current_gen;
}

bool AbstractGameField::nextCellState(int row, int column) {
    int neighbors = neighborCount(row, column);
    bool isAlive = getElementAt(row, column);

    // classic 23/3 rules
    return (!isAlive && neighbors == 3) || (isAlive && (neighbors == 2 || neighbors == 3));
}

void AbstractGameField::setCentered(const SimpleMatrix<bool> &values) {
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

void AbstractGameField::print() const {
    for (int row = 0; row < getRows(); row++) {
        for (int column = 0; column < getColumns(); column++) {
            auto current = getElementAt(row, column) ? "O " : "_ ";
            std::cout << current;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
