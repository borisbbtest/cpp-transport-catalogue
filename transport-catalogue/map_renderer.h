#pragma once
#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <utility>
#include <memory>
#include <map>
#include <vector>

namespace transport_catalog::svgreader
{
    struct RenderSettings
    {
        double width = 0;
        double height = 0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset;
        int stop_label_font_size = 0;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;
    };

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector
    {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end)
            {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs)
                { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs)
                { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_))
            {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat))
            {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom)
            {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom)
            {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom)
            {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class RouteLine : public svg::Drawable
    {

    public:
        RouteLine(std::vector<svg::Point> &points, const RenderSettings &config, size_t &color);
        void Draw(svg::ObjectContainer &container) const override;

    private:
        std::vector<svg::Point> points_;
        const RenderSettings &config_;
        size_t color_index_;
    };

    class TextMapBus : public svg::Drawable
    {

    public:
        TextMapBus(svg::Point &point, std::string_view text, const RenderSettings &config, size_t &color);
        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point point_;
        std::string_view text_;
        const RenderSettings &config_;
        size_t color_index_;
    };

    class TextMapStop : public svg::Drawable
    {

    public:
        TextMapStop(const svg::Point &point, std::string_view text, const RenderSettings &config);
        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point point_;
        std::string_view text_;
        const RenderSettings &config_;
    };

    class PointMap : public svg::Drawable
    {

    public:
        PointMap(const svg::Point &point, const RenderSettings &config);
        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point point_;
        const RenderSettings &config_;
    };
    class MapRenderer
    {
        using LabelCoordinates = std::pair<std::string_view, geo::Coordinates*>;
        struct StopComparator
        {
            bool operator()(const domain::Stop *lhs, const domain::Stop *rhs) const
            {
                return lhs->name < rhs->name;
            }
        };
        struct PointComparator
        {
            bool operator()(const LabelCoordinates* lhs, const LabelCoordinates *rhs) const
            {
                return lhs->first < rhs->first;
            }
        };
        using ArrayCoordinates = std::set<const LabelCoordinates ,MapRenderer::PointComparator>;
        using StopSet = std::set<const domain::Stop *, MapRenderer::StopComparator>;

    public:
        MapRenderer(const RenderSettings &config, std::vector<const domain::Bus *> &dictBus);
        StopSet ConvertBuses(std::vector<const domain::Bus *> &dictBus);
        svg::Document RenderMap();
        SphereProjector GetSphere(const MapRenderer::StopSet &stops, const RenderSettings &config) const;
        inline void InitRenderSettingsRoute();
        inline void InitRenderSettingsRouteText(const std::string_view bus_name, std::vector<svg::Point> &points, domain::BusType typebus, size_t color_index);
        // Data members
    private:
        // Data members
        std::map<std::string_view, std::vector<RouteLine>> routeline_;
        std::map<std::string_view, std::vector<TextMapBus>> routelineText_;
        std::vector<PointMap> routelinePoint_;
        std::vector<TextMapStop> routelinePointText_;
        
        RenderSettings renderSettings_;
        std::vector<const domain::Bus *> &dictBus_;
        StopSet stops_ ;
        SphereProjector sphereProjector_;
    };
}