#ifndef GAME_OF_LIFE_GAMEWIDGET_H
#define GAME_OF_LIFE_GAMEWIDGET_H

#include <FL/Fl_Box.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <utility>
#include "GameField.h"

class GameWidget : public Fl_Box {
    GameField field;
    bool running = false;

    void drawNext() {
        if (!running) return;
        field.nextGeneration_raw();
        redraw();
    }

    void draw() override {
        Fl_Box::draw();

        int cell_pixel_width = w() / field.getColumns();
        int cell_pixel_height = h() / field.getRows();

        for (int row = 0; row < field.getRows(); row++) {
            for (int column = 0; column < field.getColumns(); column++) {
                if (field.cellState(row, column)) {
                    fl_draw_box(FL_FLAT_BOX,
                                column * cell_pixel_width, row * cell_pixel_height,
                                cell_pixel_width, cell_pixel_height,
                                FL_GREEN);
                }
            }
        }
    }

    int handle(int e) override {
        int ret = Fl_Box::handle(e);
        if (e == FL_FOCUS || e == FL_UNFOCUS) {
            return 1;
        } else if (e == FL_KEYDOWN && Fl::event_key() == 32) {
            running = !running;
            return 1;
        }
        return ret;
    }

    static void Timer_CB(void *userdata) {
        auto *gw = (GameWidget*) userdata;
        gw->drawNext();
        Fl::repeat_timeout(1.0 / 30.0, Timer_CB, userdata);
    }

public:
    GameWidget(int x, int y, int w, int h, const char *l, GameField field) :
            Fl_Box(x, y, w, h, l),
            field(std::move(field)) {
        box(FL_FLAT_BOX);
        color(FL_BLACK);
        redraw();
        Fl::add_timeout(1.0 / 30.0, Timer_CB, (void*) this);
    }
};

#endif //GAME_OF_LIFE_GAMEWIDGET_H
