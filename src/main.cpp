#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Display.H>
#include "Pattern.h"
#include "GameWidget.h"
#include "FieldBenchmark.h"

int main(int argc, char **argv) {
    const int rows = 500;
    const int columns = 500;
    const bool output_graphics = true;
    const int FIELD_H = 1000;
    const int FIELD_W = 1000;
    const int TEXTFIELD_H = 30;
    const int TEXTFIELD_W = FIELD_W;

    GameField field(rows, columns);
    field.setCentered(PRESET_EVE);
    if (output_graphics) {
        Fl_Double_Window win(FIELD_W, FIELD_H + TEXTFIELD_H, "Game Of Life");
        GameWidget game(0, 0, FIELD_W, FIELD_H, nullptr, field);
        auto box = new Fl_Box(0, FIELD_H, FIELD_W, TEXTFIELD_H, game.getGenerationStr());
        box->labelcolor(FL_BLACK);
        box->color(FL_WHITE);
        win.end();
        win.show();
        return(Fl::run());
    } else {
        // assume we're benchmarking
        const int max_generation = 30000;
        FieldBenchmark benchmark(field);
        benchmark.run(max_generation);
    }
}