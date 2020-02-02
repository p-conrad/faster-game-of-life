#ifndef GAME_OF_LIFE_FIELDBENCHMARK_H
#define GAME_OF_LIFE_FIELDBENCHMARK_H


#include <fstream>
#include "fields/GameField.h"

class FieldBenchmark {
    GameField field;
    std::string filename;
    FILE *outfile;

public:
    explicit FieldBenchmark(const GameField &field);
    void run(int generations);

    virtual ~FieldBenchmark();
};


#endif //GAME_OF_LIFE_FIELDBENCHMARK_H
