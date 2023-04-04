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

    TextMapBus::TextMapBus(svg::Point &point, std::string_view text, const RenderSettings &config, size_t &color) : point_(point), text_(text), config_(config), color_index_(color)
    {
    }

    TextMapStop::TextMapStop(const svg::Point &point, std::string_view text, const RenderSettings &config) : point_(point), text_(text), config_(config)
    {
    }

    PointMap::PointMap(const svg::Point &point, const RenderSettings &config) : point_(point), config_(config)
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

    void PointMap::Draw(svg::ObjectContainer &container) const
    {
        auto circle = std::make_unique<svg::Circle>();
        circle->SetCenter(point_);
        circle->SetRadius(config_.stop_radius);
        circle->SetFillColor("white");
        container.AddPtr(std::move(circle));
    }

    void TextMapBus::Draw(svg::ObjectContainer &container) const
    {

        const auto make_text = [this]()
        {
            auto text = std::make_unique<svg::Text>();
            text->SetPosition(point_);
            text->SetOffset(config_.bus_label_offset);
            text->SetFontSize(config_.bus_label_font_size);
            text->SetFontFamily("Verdana");
            text->SetFontWeight("bold");
            text->SetData(std::string(text_));
            return text;
        };
        auto layer = make_text();
        layer->SetFillColor(config_.underlayer_color);
        layer->SetStrokeColor(config_.underlayer_color);
        layer->SetStrokeWidth(config_.underlayer_width);
        layer->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        layer->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        container.AddPtr(std::move(layer));

        auto text = make_text();
        text->SetFillColor(*(config_.color_palette.begin() + color_index_));
        container.AddPtr(std::move(text));
    }

    void TextMapStop::Draw(svg::ObjectContainer &container) const
    {

        const auto make_text = [this]()
        {
            auto text = std::make_unique<svg::Text>();
            text->SetPosition(point_);
            text->SetOffset(config_.stop_label_offset);
            text->SetFontSize(config_.stop_label_font_size);
            text->SetFontFamily("Verdana");
            text->SetData(std::string(text_));
            return text;
        };

        auto layer = make_text();
        layer->SetFillColor(config_.underlayer_color);
        layer->SetStrokeColor(config_.underlayer_color);
        layer->SetStrokeWidth(config_.underlayer_width);
        layer->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        layer->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        container.AddPtr(std::move(layer));

        auto text = make_text();
        text->SetFillColor("black");
        container.AddPtr(std::move(text));
    }

    MapRenderer::StopSet MapRenderer::ConvertBuses(std::vector<const domain::Bus *> &dictBus)
    {
        StopSet stops;
        for (const auto bus_link : dictBus)
        {
            for (const auto stop : bus_link->stops)
            {
                stops.insert(stop);
            }
        }
        return stops;
    }

    SphereProjector MapRenderer::GetSphere(const MapRenderer::StopSet &stops, const RenderSettings &config) const
    {
        std::vector<geo::Coordinates> coordinates;
        coordinates.reserve(stops.size());
        for (auto stop : stops)
        {
            coordinates.push_back(stop->coordinates);
        }
        return SphereProjector(coordinates.begin(), coordinates.end(), config.width, config.height, config.padding);
    }

    inline void MapRenderer::InitRenderSettingsRouteText(const std::string_view bus_name, std::vector<svg::Point> &points, domain::BusType typebus, size_t color_index)
    {
        int size_points = points.size();
        if (size_points <= 1)
        {
            return;
        }
        TextMapBus drw1(*points.begin(), bus_name, renderSettings_, color_index);
        routelineText_[bus_name].push_back(std::move(drw1));

        if (typebus == domain::BusType::Line)
        {
            double x = size_points / 2.0;
            // std::cout<<(int)(x + 0.5)<<std::endl;
            const auto &labelend = points.begin() + ((int)(x - 0.5));
            TextMapBus drw(*labelend, bus_name, renderSettings_, color_index);
            routelineText_[bus_name].push_back(std::move(drw));
        }
    }

    inline void MapRenderer::InitRenderSettingsRoute()
    {

        size_t color_index = 0;
        for (const auto bus_link : dictBus_)
        {
            if (bus_link->stops.size() == 0)
            {
                continue;
            }

            std::vector<svg::Point> points;
            std::vector<std::pair<std::string_view, geo::Coordinates>> geo_coords;

            for (const auto &stop : bus_link->stops)
            {
                if (*stop != domain::Stop{})
                {
                    geo_coords.push_back({stop->name, stop->coordinates});
                }
            }

            // Проецируем и выводим координаты
            for (const auto geo_coord : geo_coords)
            {
                const svg::Point screen_coord = sphereProjector_(geo_coord.second);
                points.push_back(screen_coord);
            }

            domain::BusType typeLine = bus_link->view != bus_link->type && bus_link->stops.front() == bus_link->stops.back() ? domain::BusType::Ring : bus_link->type;
            InitRenderSettingsRouteText(bus_link->name, points, typeLine, color_index);

            RouteLine drw(points, renderSettings_, color_index);
            if (bus_link->stops.size() >= 1)
            {
                color_index++;
            }
            if (color_index >= renderSettings_.color_palette.size())
            {
                color_index = 0;
            }

            routeline_[bus_link->name].push_back(std::move(drw));
        }
        for (const auto &buff : stops_)
        {
            const svg::Point screen_coord = sphereProjector_(buff->coordinates);
            PointMap drw(screen_coord, renderSettings_);
            routelinePoint_.push_back(std::move(drw));

            TextMapStop drw2(screen_coord, buff->name, renderSettings_);
            routelinePointText_.push_back(std::move(drw2));
        }
    }

    MapRenderer::MapRenderer(const RenderSettings &config, std::vector<const domain::Bus *> &dictBus) : renderSettings_(config),
                                                                                                        dictBus_(dictBus), stops_(ConvertBuses(dictBus)), sphereProjector_(GetSphere(stops_, config))
    {

        std::sort(dictBus_.begin(), dictBus_.end(), [](const domain::Bus *a, const domain::Bus *b)
                  { return a->name < b->name; });
        InitRenderSettingsRoute();
    }
    svg::Document MapRenderer::RenderMap()
    {
        svg::Document doc;

        for (const auto &[busname,pline ] : routeline_)
        {
            // std::cout << "name" << k << std::endl;

            for (const RouteLine &a : pline)
            {
                a.Draw(doc);
            }
        }

        for (const auto &[busname, text] : routelineText_)
        {
            // std::cout << "name" << k << std::endl;

            for (const TextMapBus &a : text)
            {
                a.Draw(doc);
            }
        }

        for (const auto &Point : routelinePoint_)
        {
            Point.Draw(doc);
        }

        for (const auto &Point : routelinePointText_)
        {
            Point.Draw(doc);
        }

        return doc;
    }
    // end
}