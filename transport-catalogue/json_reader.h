#pragma once
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <sstream>
#include <iostream>

namespace transport_catalog::json_reader
{

    class JsonReader
    {
    private:
        /* data */
        json::Document document_json_;
        transport_catalog::TransportCatalogue transport_catalog_;
        inline void StatRequest();
        inline void BaseRequest();
        svg::Document RenderSVGRequest();
        svg::Color SetColor(const json::Node &color);
        std::vector<svg::Color> SetColorPalette(const json::Array &palette);
        svg::Point Offset(const json::Array &offset);
        inline void RenderMap(json::Builder &buff_node);
        inline void RenderStop(json::Builder &buff_node, const json::Node &value);
        inline void RenderBus(json::Builder &buff_node, const json::Node &value);
        inline void ParsingRequestBus();
        inline void ParsingRequestStop();

    public:
        JsonReader(std::istream &it);
        ~JsonReader();
    };

}
