#ifndef GAME_OF_LIFE_SIMPLEMATRIX_H
#define GAME_OF_LIFE_SIMPLEMATRIX_H


#include <cassert>
#include <cstdlib>
#include <vector>

template<typename T>
class SimpleMatrix {
public:
    SimpleMatrix(int rows, int columns) :
            rows(rows),
            columns(columns),
            elements(rows * columns) {}

    SimpleMatrix(int rows, int columns, T value) :
            rows(rows),
            columns(columns),
            elements(rows * columns, value) {}

    SimpleMatrix(int rows, int columns, std::initializer_list<T> values) :
            rows(rows),
            columns(columns) {
        assert(rows * columns == (int)values.size());
        elements = std::vector<T>(values);
    }

    T operator()(int y, int x) const {
        return getElementAt(y, x);
    }

    int getRows() const {
        return rows;
    }

    int getColumns() const {
        return columns;
    }

    T getElementAt(int y, int x) const {
        return elements[(y * columns) + x];
    }

    void setElementAt(int y, int x, T value) {
        elements[(y * columns) + x] = value;
    }

private:
    int rows;
    int columns;
    std::vector<T> elements;
};


#endif //GAME_OF_LIFE_SIMPLEMATRIX_H
