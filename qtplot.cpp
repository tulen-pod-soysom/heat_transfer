#include "qtplot.h"
#include <math.h>

my_graphics::QtPlot::QtPlot(QWidget* parent) : QWidget(parent)
{

    fonts = std::vector<QFont>{
        QFont("Arial",9),
        QFont("Arial",11),
        QFont("Arial",11,QFont::DemiBold,true)
        };
    plot_pens = std::vector<QPen>{
        QPen(QColor(255,0,0),0.001),
        QPen(QColor(0,0,255),0.001),
        QPen(QColor(0,255,0),0.001),
        QPen(QColor(255,255,0),0.001),
        QPen(QColor(0x1f, 0x3a, 0x3d),0.001),
        };

    grid_pen = QPen(QColor(128,128,128),1);
    main_grid_pen = QPen(QColor(0,0,0),1);

    acc_settings = {};
    acc_settings.create_matrix = true;
    acc_settings.accommodate_grid_scale = true;
    acc_settings.accommodate_plot_bounds = true;
    acc_settings.x_axis_symmetry = false;
    acc_settings.y_axis_symmetry = false;
    acc_settings.bounds_detection_x = log;
    acc_settings.bounds_detection_y = log;


}

void my_graphics::QtPlot::save_as_file(std::wstring file_path, unsigned width, unsigned height)
{

}

void my_graphics::QtPlot::paintEvent(QPaintEvent *event)
{
    auto r = rect();
    accommodate(r);
    QPainter painter(this);
    render(painter,r);
}

void my_graphics::QtPlot::render(QPainter &painter, QRect rect)
{
    painter.setRenderHint(painter.HighQualityAntialiasing);
    painter.fillRect(rect,QBrush(QColor(255,255,255)));

    render_grid(painter);
    painter.setWorldTransform(transform_matrix);
    render_main(painter,program);
    painter.resetTransform();

    render_main_grid(painter);
    render_title(painter, rect);
    render_captions(painter, rect);
    render_after_main(painter, rect);
}

void my_graphics::QtPlot::render_grid(QPainter &painter)
{
    QBrush brush(QColor(0,0,0));
    painter.setPen(grid_pen);
    painter.setBrush(brush);
    painter.setFont(fonts[0]);

    for (size_t i = 0; i < worldRect.width() / grid_step_x; i++)
    {
        QPointF p1 = { worldRect.left() + i * grid_step_x,worldRect.top() };
        QPointF p2 = { worldRect.left() + i * grid_step_x,worldRect.bottom() };

        QString caption; caption.sprintf("%g", p1.x());

        p1 = transform_matrix.map(p1);
        p2 = transform_matrix.map(p2);

        painter.drawLine(p1, p2);

        QRectF r { QPointF{ p1.x() - offsetX, p1.y() }, QSizeF{ 2 * offsetX, offsetY } };
        painter.drawText(r,Qt::AlignCenter,caption);
    }

    for (size_t i = 0; i < worldRect.height() / grid_step_y; i++)
    {
        QPointF p1 = { worldRect.left() , worldRect.top() + i * grid_step_y };
        QPointF p2 = { worldRect.right(), worldRect.top() + i * grid_step_y };
        QString caption; caption.sprintf("%g", p1.y());

        p1 = transform_matrix.map(p1);
        p2 = transform_matrix.map(p2);

        painter.drawLine(p1, p2);

        QRectF r = { QPointF{ 0, p1.y() - 30 }, QSizeF{ offsetX, 30 * 2 } };
        painter.drawText(r,Qt::AlignCenter,caption);
    }
}

void my_graphics::QtPlot::render_main(QPainter &painter, void *program)
{
    if (!points.empty())
        for (size_t var = 0; var < points.size(); ++var)
        {
            if (points[var].size() == 0) continue;
            if (var < plot_pens.size()) painter.setPen(plot_pens[var]);

            painter.drawPolyline(points[var].data(),points[var].size());
        }
}

void my_graphics::QtPlot::render_main_grid(QPainter &painter)
{
    QPointF zero = transform_matrix.map(QPointF{0,0});
    auto screen_wr = transform_matrix.mapRect(worldRect);
    painter.setPen(main_grid_pen);

    painter.drawLine(screen_wr.left(),zero.y(), screen_wr.right(),zero.y());
    painter.drawLine(zero.x(),screen_wr.bottom(),zero.x(), screen_wr.top());
}

void my_graphics::QtPlot::render_after_main(QPainter &painter, QRect rect)
{

}

void my_graphics::QtPlot::render_captions(QPainter &painter, QRect rect)
{
    painter.setBrush(QBrush(QColor(0,0,0)));
    painter.setFont(fonts[1]);
    //!!!!!!!!
    painter.translate(offsetX,offsetY);
    painter.drawText(QPointF(0,0),captionY);
    painter.drawText(QPointF(rect.width(),rect.height() / 2.0f),captionX);
    painter.resetTransform();
}

void my_graphics::QtPlot::render_title(QPainter &painter, QRect rect)
{
    painter.setBrush(QBrush(QColor(0,0,0)));
    painter.setFont(fonts[2]);
    QRectF r (QPointF(0,0),QSizeF(rect.width(),offsetY));
    painter.drawText(r,Qt::AlignCenter,title);
}

void my_graphics::QtPlot::accommodate(QRect outerrect)
{
    accommodate(acc_settings, outerrect);
}

void my_graphics::QtPlot::accommodate(accommodate_settings settings, QRect outerrect)
{
    QRect rect = outerrect;
    rect.adjust(offsetX,offsetY,-2*offsetX,-2*offsetY);


    if (points.empty()) {
        worldRect = { -1,-1,2,2 };
    }
    else if (settings.accommodate_plot_bounds)
    {
        std::vector<float> max_top(points.size());
        std::vector<float> min_bottom(points.size());
        std::vector<float> min_lefts(points.size());
        std::vector<float> max_rights(points.size());

        for (size_t i = 0; i < points.size(); i++)
        {
            auto a = std::minmax_element(points[i].begin(), points[i].end(), [](auto& a, auto& b) {return a.y() < b.y(); });
            min_bottom[i] = a.first->y();
            max_top[i] = a.second->y();

            //!!! Dangerous place
            min_lefts[i] = points[i].begin()->x();
            max_rights[i] = points[i].rbegin()->x();
        }
        float min_y = *std::max_element(min_bottom.begin(), min_bottom.end(), [](auto& a, auto& b) {return abs(a) < abs(b); });
        float max_y = *std::max_element(max_top.begin(), max_top.end(), [](auto& a, auto& b) {return abs(a) < abs(b); });

        float min_x = *std::max_element(min_lefts.begin(), min_lefts.end(), [](auto& a, auto& b) {return abs(a) < abs(b); });
        float max_x = *std::max_element(max_rights.begin(), max_rights.end(), [](auto& a, auto& b) {return abs(a) < abs(b); });

        switch (settings.bounds_detection_x)
        {
        case linear:
            min_x = floor(min_x);
            max_x = ceil(max_x);
            break;
        case log:
            min_x = (min_x > 0) ? FloorMantissa(min_x) : CeilMantissa(min_x);
            max_x = (max_x > 0) ? CeilMantissa(max_x) : FloorMantissa(max_x);
            break;
        case max_min:
            break;
        }
        switch (settings.bounds_detection_y)
        {
        case linear:
            min_y = floor(min_y);
            max_y = ceil(max_y);
            break;
        case log:
            min_y = (min_y > 0) ? FloorMantissa(min_y) : CeilMantissa(min_y);
            max_y = (max_y > 0) ? CeilMantissa(max_y) : FloorMantissa(max_y);
            break;
        case max_min:
            break;
        }

        if (settings.x_axis_symmetry) { max_x = std::max(abs(max_x), abs(min_x)); min_x = -max_x; }
        if (settings.y_axis_symmetry) { max_y = std::max(abs(max_y), abs(min_y)); min_y = -max_y; }

        worldRect = {
            min_x,
            min_y,
            max_x - min_x,
            max_y - min_y
        };
    }

    if (settings.accommodate_grid_scale)
    {
        grid_step_x = Get10thPower(std::abs(worldRect.right() - worldRect.left()));
        grid_step_y = Get10thPower(std::abs(worldRect.bottom() - worldRect.top()));

        if (abs(grid_step_x - 1) <= FLT_EPSILON) grid_step_x /= 10.f;
        if (abs(grid_step_y - 1) <= FLT_EPSILON) grid_step_y /= 10.f;
    }
    else
    {
        grid_step_x = worldRect.width() / grid_count_x;
        grid_step_y = worldRect.height() / grid_count_y;
    }

    if (settings.create_matrix)
    {
        transform_matrix.reset();
        auto c = outerrect.center();
        auto z = worldRect.center();
        transform_matrix.translate(c.x(),c.y());
        transform_matrix.scale(rect.width()/worldRect.width(),-rect.height()/worldRect.height());
        transform_matrix.translate(-z.x(),-z.y());

        //transform_matrix.translate(0.5 * outerrect.width(), 0.5 * outerrect.height());
        //transform_matrix.Scale(rect.width() / worldRect.width(), -(REAL)rect.height() / worldRect.height());
        //transform_matrix.Translate(-(worldRect.x() + worldRect.width() / 2.0), -(worldRect.y() + worldRect.height() / 2.0));
    }
}
