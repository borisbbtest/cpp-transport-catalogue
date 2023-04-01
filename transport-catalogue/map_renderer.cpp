#include "map_renderer.h"
#include <utility>

namespace transport_catalog::svgreader
{

    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }
    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }
    // Render Graphics
    RouteLine::RouteLine(std::vector<svg::Point> &points, const RenderSettings &config, size_t &color) : points_(points), config_(config), color_index_(color)
    {
    }
    void RouteLine::Draw(svg::ObjectContainer &container) const
    {
        auto polyline = std::make_unique<svg::Polyline>();
        polyline->SetStrokeWidth(config_.line_width);
        polyline->SetStrokeColor(*(config_.color_palette.begin() + color_index_));
        polyline->SetFillColor(svg::NoneColor);
        polyline->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for (auto x : points_)
        {
            polyline->AddPoint(x);
        }
        container.AddPtr(std::move(polyline));
    }

    MapRenderer::MapRenderer(const RenderSettings &config, const std::unordered_map<std::string_view, const domain::Bus *> &dictBus)
    {

        size_t color_index = 0;
        for (const auto &[name_bus, bus_link] : dictBus)
        {
            if (bus_link->stops.size() == 0)
            {
                continue;
            }
            std::vector<svg::Point> points;
            std::vector<geo::Coordinates> geo_coords;
            for (const auto &stop : bus_link->stops)
            {
                if (*stop != domain::Stop{})
                {
                    geo_coords.push_back(stop->coordinates);
                }
            }
            const SphereProjector proj{geo_coords.begin(), geo_coords.end(), config.width, config.height, config.padding};

            // Проецируем и выводим координаты
            for (const auto geo_coord : geo_coords)
            {
                const svg::Point screen_coord = proj(geo_coord);
                points.push_back(screen_coord);
                // std::cout << '(' << geo_coord.lat << ", " << geo_coord.lng << ") -> ";
                // std::cout << '(' << screen_coord.x << ", " << screen_coord.y << ')' << std::endl;
            }

            RouteLine drw(points, config, color_index);
            if (bus_link->stops.size() > 1)
            {
                color_index++;
            }
            if (color_index >= config.color_palette.size())
            {
                color_index = 0;
            }

            routeline_[name_bus].push_back(std::move(drw));
        }
    }
    svg::Document MapRenderer::RenderMap()
    {
        svg::Document doc;
        for (const auto &[k, v] : routeline_)
        {
            // std::cout << "name" << k << std::endl;
            for (const RouteLine &a : v)
            {
                a.Draw(doc);
            }
        }
        return doc;
    }
    // end
}