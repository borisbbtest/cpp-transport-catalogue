#include "json_reader.h"
#include <algorithm>
#include <cassert>

namespace transport_catalog::json_reader
{
    JsonReader::JsonReader(std::istream &it) : document_json_(json::Load(it))
    {
        BaseRequest();
        StatRequest();
    }

    inline void JsonReader::RenderMap(json::Builder &buff_node)
    {
        const auto &mapdoc = RenderSVGRequest();
        std::ostringstream sstream;
        mapdoc.Render(sstream);
        buff_node.Key("map").Value(sstream.str());
    }
    inline void JsonReader::RenderStop(json::Builder &buff_node, const json::Node &value)
    {
        const auto &stopinfo = transport_catalog_.GetStopInfo(value.AsDict().at("name").AsString());
        if (stopinfo.isFound)
        {
            json::Array tmp;
            for (const std::string_view str : stopinfo.buses)
            {
                tmp.push_back(json::Node{std::string(str)});
            }
            buff_node.Key("buses").Value(tmp);
        }
        else
        {
            buff_node.Key("error_message").Value("not found");
        }
    }

    inline void JsonReader::RenderBus(json::Builder &buff_node, const json::Node &value)
    {
        // std::cout << value.AsMap().at("name").AsString() << std::endl;
        const auto &businfo = transport_catalog_.GetBusInfo(value.AsDict().at("name").AsString());
        if (businfo.isFound)
        {

            buff_node.Key("curvature").Value(businfo.curvature);
            buff_node.Key("route_length").Value( businfo.routeLength);
            buff_node.Key("stop_count").Value(static_cast<int>(businfo.coutStopOnRoute));
            buff_node.Key("unique_stop_count").Value(static_cast<int>(businfo.uniqStops));
        }
        else
        {
           buff_node.Key("error_message").Value("not found");
        }
    }

    inline void JsonReader::StatRequest()
    {
        try
        {
            const auto &root_map = document_json_.GetRoot().AsDict();
            if (root_map.count("stat_requests") > 0)
            {
                json::Builder BuildDoc = json::Builder();
                BuildDoc.StartArray();
                for (const auto &value : root_map.at("stat_requests").AsArray())
                {
                    BuildDoc.StartDict().Key("request_id").Value(value.AsDict().at("id").AsInt());
    
                    if (value.AsDict().at("type").AsString() == "Stop")
                    {
                        RenderStop(BuildDoc, value);
                    }

                    if (value.AsDict().at("type").AsString() == "Bus")
                    {
                       RenderBus(BuildDoc, value);
                    }

                    if (value.AsDict().at("type").AsString() == "Map")
                    {
                       RenderMap(BuildDoc);
                    }
                    BuildDoc.EndDict();
                }
                BuildDoc.EndArray();
                json::Print(json::Document{BuildDoc.Build()},std::cout);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    inline void JsonReader::ParsingRequestBus()
    {
        const auto &root_map = document_json_.GetRoot().AsDict();
        if (root_map.count("base_requests") > 0)
        {
            for (const auto &value : root_map.at("base_requests").AsArray())
            {
                if (value.AsDict().at("type").AsString() == "Bus")
                {
                    domain::Bus buff_bus;

                    buff_bus.name = value.AsDict().at("name").AsString();
                    buff_bus.type = value.AsDict().at("is_roundtrip").AsBool() ? domain::BusType::Ring : domain::BusType::Line;

                    for (const auto &v : value.AsDict().at("stops").AsArray())
                    {
                        auto stop = transport_catalog_.FindStop(v.AsString());
                        if (*stop != Stop{})
                        {
                            buff_bus.stops.push_back(stop);
                        }
                        // std::cout << "--" << v.AsString() << std::endl;
                    }

                    if (buff_bus.stops.front() != buff_bus.stops.back())
                    {

                        buff_bus.view = buff_bus.type;
                    }
                    else
                    {
                        buff_bus.view = domain::BusType::Ring;
                    }

                    if (buff_bus.type == transport_catalog::domain::BusType::Line)
                    {
                        std::vector<const transport_catalog::domain::Stop *> reverse_name;
                        std::for_each(std::next(buff_bus.stops.rbegin()), buff_bus.stops.rend(),
                                      [&reverse_name](const transport_catalog::domain::Stop *name)
                                      { reverse_name.push_back(name); });
                        buff_bus.stops.insert(buff_bus.stops.end(), reverse_name.begin(), reverse_name.end());
                    }

                    transport_catalog_.AddBus(buff_bus);
                }
            }
        }
    }

    inline void JsonReader::ParsingRequestStop()
    {
        const auto &root_map = document_json_.GetRoot().AsDict();
        if (root_map.count("base_requests") > 0)
        {

            std::unordered_map<std::string_view, const json::Dict *> buff_stops_dist;
            for (const auto &value : root_map.at("base_requests").AsArray())
            {
                if (value.AsDict().at("type").AsString() == "Stop")
                {
                    domain::Stop buff_stop;

                    buff_stop.name = value.AsDict().at("name").AsString();
                    buff_stop.coordinates.lat = value.AsDict().at("latitude").AsDouble();
                    buff_stop.coordinates.lng = value.AsDict().at("longitude").AsDouble();
                    transport_catalog_.AddStop(buff_stop);
                    buff_stops_dist[value.AsDict().at("name").AsString()] = &value.AsDict().at("road_distances").AsDict();
                }
            }

            for (const auto &[key, value] : buff_stops_dist)
            {
                for (const auto &[l, r] : *value)
                {
                    // std::cout << key << "--=-==-=" << l << "---" << r.AsInt() << std::endl;
                    transport_catalog_.AddDistances(key, l, r.AsInt());
                }
            }
        }
    }

    inline void JsonReader::BaseRequest()
    {
        ParsingRequestStop();
        ParsingRequestBus();
    }

    svg::Document JsonReader::RenderSVGRequest()
    {
        svg::Document doc;
        try
        {
            svgreader::RenderSettings redsetting;
            const auto &root_map = document_json_.GetRoot().AsDict().at("render_settings").AsDict();

            redsetting.width = root_map.at("width").AsDouble();
            redsetting.height = root_map.at("height").AsDouble();
            redsetting.padding = root_map.at("padding").AsDouble();
            redsetting.line_width = root_map.at("line_width").AsDouble();
            redsetting.stop_radius = root_map.at("stop_radius").AsDouble();
            redsetting.bus_label_font_size = root_map.at("bus_label_font_size").AsInt();
            redsetting.bus_label_offset = Offset(root_map.at("bus_label_offset").AsArray());
            redsetting.stop_label_font_size = root_map.at("stop_label_font_size").AsInt();
            redsetting.stop_label_offset = Offset(root_map.at("stop_label_offset").AsArray());
            redsetting.underlayer_color = SetColor(root_map.at("underlayer_color"));
            redsetting.underlayer_width = root_map.at("underlayer_width").AsDouble();
            redsetting.color_palette = SetColorPalette(root_map.at("color_palette").AsArray());

            auto buses = transport_catalog_.GetBusesVector();
            svgreader::MapRenderer maprend(redsetting, buses);
            doc = maprend.RenderMap();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error -" << e.what() << '\n';
        }
        return doc;
    }

    svg::Point JsonReader::Offset(const json::Array &offset)
    {
        return svg::Point(offset[0].AsDouble(), offset[1].AsDouble());
    }

    std::vector<svg::Color> JsonReader::SetColorPalette(const json::Array &palette)
    {
        std::vector<svg::Color> colors;
        colors.reserve(palette.size());
        for (const auto &color : palette)
        {
            colors.push_back(SetColor(color));
        }
        return colors;
    }

    svg::Color JsonReader::SetColor(const json::Node &color)
    {
        if (color.IsString())
        {
            return color.AsString();
        }
        else if (color.IsArray())
        {
            const auto &color_array = color.AsArray();
            if (color_array.size() == 3)
            {
                return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
            }
            else if (color_array.size() == 4)
            {
                return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
            }
        }
        assert(false);
    }
    JsonReader::~JsonReader()
    {
    }
}
