#pragma once
#include "json.h"
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
        TransportCatalogue transport_catalog_;
        inline void StatRequest();
        inline void BaseRequest();
        inline void RenderSVGRequest();
        svg::Color Color(const json::Node &color);
        std::vector<svg::Color> ColorPalette(const json::Array &palette);
        svg::Point Offset(const json::Array &offset);

    public:
        JsonReader(std::istream &it);
        ~JsonReader();
    };

}
