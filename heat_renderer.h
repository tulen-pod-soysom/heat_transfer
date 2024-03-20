#ifndef HEAT_RENDERER_H
#define HEAT_RENDERER_H

#include <QKeyEvent>
#include <QWidget>
#include <QPainter>
//#include "heat_transfer_program.hpp"
struct heat_transfer_program;

class heat_renderer : public QWidget
{
public:
    heat_renderer(QWidget* parent = nullptr) : QWidget(parent){setFocusPolicy(Qt::ClickFocus);};
    QRectF world_rect;

    heat_transfer_program* program = nullptr;

    bool adaptive_temperature = true;
    unsigned izolines_size = 10;

    double T_max = 350;
    double T_min = 250;
    bool do_hint = false;
    bool do_izolines = false;

    // QWidget interface
protected:



    void paintEvent(QPaintEvent *event);
    QColor InterpolateColor(double T);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key::Key_Shift)
            do_hint = true;
    }
    void keyReleaseEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key::Key_Shift)
            do_hint = false;
    }
};

#endif // HEAT_RENDERER_H
