#ifndef QTPLOT_H
#define QTPLOT_H

#include <QWidget>
#include <QPainter>
#include <vector>
#include <math.h>
#include <float.h>

namespace my_graphics
{
    class QtPlot : public QWidget
    {
    public:
        std::vector<std::vector<QPointF>> points;

        std::vector<QFont> fonts;

        QPen grid_pen;
        QPen main_grid_pen;

        std::vector<QPen> plot_pens;

        QTransform transform_matrix;
        void render(QPainter& painter, QRect rect);

        virtual void render_grid(QPainter& painter);
        virtual void render_main(QPainter& painter, void* program = nullptr);
        virtual void render_main_grid(QPainter& painter);
        virtual void render_after_main(QPainter& painter, QRect rect);

        virtual void render_captions(QPainter& painter, QRect rect);
        virtual void render_title(QPainter& painter, QRect rect);

        enum plot_bounds_detect
        {
            log,
            linear,
            max_min
        };

        struct accommodate_settings
        {
            int create_matrix : 1;
            int accommodate_plot_bounds : 1;
            int accommodate_grid_scale : 1;
            int x_axis_symmetry : 1;
            int y_axis_symmetry : 1;

            plot_bounds_detect bounds_detection_x, bounds_detection_y;
        } acc_settings;

        void accommodate(QRect outerrect);
        void accommodate(accommodate_settings settings, QRect outerrect);

        template	<typename InputIt>
        void add_points(InputIt begin, InputIt end, float width = 1, float left = 0);

        template	<typename InputIt>
        void add_points(std::initializer_list<std::pair<InputIt, InputIt>> list, float width = 1, float left = 0);

        template	<typename InputIt>
        void set_points(InputIt begin, InputIt end, float width = 1, float left = 0);

        template	<typename InputIt>
        void set_points(std::initializer_list<std::pair<InputIt, InputIt>> list, float width = 1, float left = 0);


        float offsetX = 30;
        float offsetY = 30;

        QRectF worldRect = { 0,-1,2,2 };

        float grid_step_x = 1.0;
        float grid_step_y = 1.0;

        unsigned grid_count_x = 10;
        unsigned grid_count_y = 10;

        QString title;
        QString captionX;
        QString captionY;

        void* program = nullptr;


        QtPlot(QWidget* p = nullptr);

        void save_as_file(std::wstring file_path, unsigned width, unsigned height );

        // QWidget interface
    protected:
        void paintEvent(QPaintEvent *event);
    };


    // Useful methods
    template<typename T>
    static T CeilMantissa(T value)
    {
        int sign = value > 0 ? 1.0 : -1.0;
        value = abs(value);
        if (value < FLT_EPSILON) return 0.0;
        return sign * pow(10, floor(log10(value))) * ceil(pow(10, log10(value) - floor(log10(value))));
    }
    template<typename T>
    static T FloorMantissa(T value)
    {
        int sign = value > 0 ? 1.0 : -1.0;
        value = abs(value);
        if (value < FLT_EPSILON) return 0.0;
        return sign * pow(10, floor(log10(value))) * floor(pow(10, log10(value) - floor(log10(value))));
    }
    template<typename T>
    static T GetMantissa(T value)
    {
        int sign = value > 0 ? 1.0 : -1.0;
        value = abs(value);
        if (value < FLT_EPSILON) return 0.0;
        return sign * pow(10, log10(value) - floor(log10(value)));
    }
    template <typename T>
    static T Get10thPower(T value)
    {
        value = abs(value);
        if (value < FLT_EPSILON) return 0.0;
        return  pow(10, floor(log10(value)));
    }

    template<typename InputIt, typename T = typename InputIt::value_type>
    static T CeilMantissa(InputIt begin, InputIt end)
    {
        T fmax = *(std::max_element(begin, end));
        T fmin = *(std::min_element(begin, end));
        T fextr = (abs(fmax) > abs(fmin)) ? fmax : fmin;
        return CeilMantissa(fextr);
    }

    template<typename InputIt, typename T = typename InputIt::value_type>
    static T FloorMantissa(InputIt begin, InputIt end)
    {
        T fmax = *(std::max_element(begin, end));
        T fmin = *(std::min_element(begin, end));
        T fextr = (abs(fmax) > abs(fmin)) ? fmax : fmin;
        return FloorMantissa(fextr);
    }


    template<typename InputIt>
    inline void QtPlot::add_points(InputIt begin, InputIt end, float width, float left)
    {
        std::vector<QPointF> p(end - begin);
        for (auto it = begin; it != end; it++)
            p[it - begin] = { left + (it - begin) * width / (end - begin),(float)*it };

        points.emplace_back(p);
    }
    template<typename InputIt>
    inline void QtPlot::add_points(std::initializer_list<std::pair<InputIt, InputIt>> list, float width, float left)
    {
        for (auto& i : list) add_points(i.first, i.second, width, left);

    }
    template<typename InputIt>
    inline void QtPlot::set_points(InputIt begin, InputIt end, float width, float left)
    {
        points.clear(); points.resize(1);
        points[0].resize(end-begin);

        for (auto it = begin; it != end; it++)
            points[0][it - begin] = { left + (it - begin) * width / (end - begin),(float)*it };
    }
    template<typename InputIt>
    inline void QtPlot::set_points(std::initializer_list<std::pair<InputIt, InputIt>> list, float width, float left)
    {
        if (list.size() > 0) set_points(list.begin()->first, list.begin()->second, width, left);
        if (list.size() > 1)
            for (auto it = list.begin() + 1; it != list.end(); it++)
                add_points(it->first, it->second, width, left);
    }
}
#endif // QTPLOT_H
