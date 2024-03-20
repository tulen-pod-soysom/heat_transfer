#include "heat_renderer.h"
#include "heat_transfer_program.hpp"
#include <QCursor>

void heat_renderer::paintEvent(QPaintEvent *event)
{
    if (program == nullptr) return;

    world_rect = {0,0,program->v.r.right_bound(),program->v.z.right_bound()};

    QPainter painter(this);


    QRect outer_rect = rect();
    QRect rect = outer_rect; rect.adjust(0,60,-100,0);

    QTransform transform;
    transform.translate(rect.left(),rect.bottom());
    transform.scale(rect.width()/world_rect.width(),-rect.height()/world_rect.height());

    painter.setTransform(transform);




    auto& field = *program->v.temperature_field.rbegin();





    auto min_max = std::minmax_element(field.data().begin(),field.data().end());
    if (adaptive_temperature) {T_min = *min_max.first; T_max = *min_max.second;}

    for (int i = 0; i < program->v.r.size()-1; ++i) {
        for (int j = 0; j < program->v.z.size()-1; ++j) {





            //auto low_index = ceil(izolines_size*(T[0].T-T_min)/(T_max-T_min));
            //auto high_index = floor(izolines_size*(T[2].T-T_min)/(T_max-T_min));
            //if (low_index <= high_index)
            //{
            //
            //
            //    auto& i_ = low_index;
            //    auto T_izoline = T_min + i_* (T_max - T_min)/float(izolines_size);
            //
            //    auto& x = program->v.r;
            //    auto& y = program->v.z;
            //    auto x_izoline_1 = -(T[1].T - T_izoline)/(T_max - T_min) *(x[T[1].i] - x[T[0].i]) + x[T[1].i];
            //    auto y_izoline_1 = -(T[1].T - T_izoline)/(T_max - T_min) *(y[T[1].j] - y[T[0].j]) + y[T[1].j];
            //
            //    auto x_izoline_2 = -(T[2].T - T_izoline)/(T_max - T_min) *(x[T[2].i] - x[T[0].i]) + x[T[2].i];
            //    auto y_izoline_2 = -(T[2].T - T_izoline)/(T_max - T_min) *(y[T[2].j] - y[T[0].j]) + y[T[2].j];
            //
            //    QPen pen(QColor(0,0,0,255));
            //    pen.setCosmetic(true);
            //    painter.setPen(pen);
            //    painter.drawLine(x_izoline_1,y_izoline_1,x_izoline_2,y_izoline_2);
            //}
            //}

            QPen pen(QColor(0,0,0,10));
            pen.setCosmetic(true);
            painter.setPen(pen);
            QColor color = InterpolateColor(field.at_element(i,j));
            QBrush brush(color);
            painter.setBrush(brush);

            QPointF points[4];
            points[0] = {program->v.r[i],program->v.z[j]};
            points[1] = {program->v.r[i],program->v.z[j+1]};
            points[2] = {program->v.r[i+1],program->v.z[j+1]};
            points[3] = {program->v.r[i+1],program->v.z[j]};
            painter.drawPolygon(points,4);
        }
    }



    if (do_izolines)
    {
        for (int var = 0; var < izolines_size; ++var) {

            double T_izo = T_min + var * (T_max - T_min) / (double)izolines_size;

            for (int i = 0; i < program->v.r.size()-1; ++i) {
                for (int j = 0; j < program->v.z.size()-1; ++j) {

                    double T[3] = {
                        field.at_element(i,j),
                        field.at_element(i+1,j),
                        field.at_element(i,j+1)
                    };
                    auto& T_centr = T[0];
                    auto& T_right = T[1];
                    auto& T_upper = T[2];


                    auto [min,max] = std::minmax_element(T,T+3);
                    if ((T_izo >= *min) && (T_izo <= *max))
                    {
                        auto& x = program->v.r;
                        auto& y = program->v.z;

                        auto factor_x = (T_izo - T_centr)/(T_right - T_centr);
                        auto factor_y = (T_izo - T_centr)/(T_upper - T_centr);
                        auto x_izoline = factor_x * x.get_step() + x[i];
                        auto y_izoline = factor_y * y.get_step() + y[j];

                        QPointF p1(x[i],y_izoline);
                        QPointF p2(x_izoline,y[j]);

                        auto mux1 = (x[i]   - p1.x())/(p2.x() - p1.x());
                        auto mux2 = (x[i+1] - p1.x())/(p2.x() - p1.x());
                        auto muy1 = (y[j]   - p1.y())/(p2.y() - p1.y());
                        auto muy2 = (y[j+1] - p1.y())/(p2.y() - p1.y());

                        if (p2.x() - p1.x() < 0) std::swap(mux1,mux2);
                        if (p2.y() - p1.y() < 0) std::swap(muy1,muy2);



                        QPointF izopoint1(
                                    p1.x() + (p2.x() - p1.x()) * std::max(muy1,mux1),
                                    p1.y() + (p2.y() - p1.y()) * std::max(muy1,mux1)
                                    );
                        QPointF izopoint2(
                                    p1.x() + (p2.x() - p1.x()) * std::min(muy2,mux2),
                                    p1.y() + (p2.y() - p1.y()) * std::min(muy2,mux2)
                                    );

                        QPen pen(QColor(0,0,0,255));
                        pen.setCosmetic(true);
                        painter.setPen(pen);
                        painter.drawLine(izopoint1,izopoint2);
                    }
                }
            }
        }
    }

    QPen bound_pen(QColor(0,0,0));
    bound_pen.setCosmetic(true);
    painter.setPen(bound_pen);
    painter.setBrush(Qt::BrushStyle::NoBrush);
    QRectF heater = {
        0,
        program->p.height-program->p.wall_width-program->p.heater_height,
        program->p.heater_radius,
        program->p.heater_height
    };

    QRectF vessel = {
        0,
        program->p.wall_width,
        program->p.radius- program->p.wall_width,
        program->p.height - 2*program->p.wall_width
    };
    painter.drawRect(heater);
    painter.drawRect(vessel);





    if (do_hint)
    {
        QPointF pos = this->mapFromGlobal(QCursor::pos());
        if (this->rect().contains(pos.toPoint(),true))
        {
            QPen str_pen(QColor(0,0,0)); str_pen.setCosmetic(true);
            QBrush str_brush(QColor(140,140,140,140));

            painter.setPen(str_pen);
            painter.setBrush(str_brush);

            QRectF drawing_box(pos.x(),pos.y()-60,120,60);


            pos = painter.transform().inverted().map(pos);
            auto drawing_box_trans = painter.transform().inverted().mapRect(drawing_box);


            painter.drawRect(drawing_box_trans);

            size_t i = program->v.r.closest_index(pos.x());
            size_t j = program->v.z.closest_index(pos.y());


            QString material_str;
            auto& m = program->material_at_point(program->v.r[i],program->v.z[j]);
            if (std::addressof(m) == std::addressof(program->p.metal)) material_str = "metal";
            else if (std::addressof(m) == std::addressof(program->p.liquid)) material_str = "liquid";
            //else if (m == program->p.steel) material_str = "steel";
            QString str; str.sprintf("%.3f,%.3f:\r\n{%g}\r\n%s",
                                     program->v.r[i],program->v.z[j],
                                     program->v.temperature_field.rbegin()->at_element(i,j),
                                     material_str.toStdString().c_str());
            painter.resetTransform();
            painter.drawText(drawing_box,Qt::AlignCenter,str);
        }
    }
}

QColor heat_renderer::InterpolateColor(double T)
{
    QColor color1(0,0,255), color2(255,0,0);

    //double T_max{350.0}, T_min{250.0};

    double factor = (T - T_min)/(T_max - T_min);
    if (isnanf(factor)) return QColor(0,0,0);
    if (factor < 0) return color1;
    if (factor > 1) return color2;
    QColor color(
                color1.red() + (color2.red() - color1.red()) * factor,
                color1.green() + (color2.green() - color1.green()) * factor,
                color1.blue() + (color2.blue() - color1.blue()) * factor
                );
    return color;
}
