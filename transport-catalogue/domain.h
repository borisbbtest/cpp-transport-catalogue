#pragma once
#include "geo.h"
#include <string>
#include <vector>
#include <set>

namespace transport_catalog::domain
{
    enum BusType
    {
        Ring,
        Line,
    };

    struct Stop
    {
        std::string name;
        geo::Coordinates coordinates;
        bool operator==(const Stop &in) const
        {
            return name == in.name;
        }
        bool operator!=(const Stop &in) const
        {
            return !(*this == in);
        }
    };

    struct Bus
    {
        std::string name;
        BusType type;
        BusType  view;
        std::vector<const Stop *> stops;

        bool operator==(const Bus &in) const
        {
            return name == in.name;
        }
        bool operator!=(const Bus &in) const
        {
            return !(*this == in);
        }
    };

    struct BusOut
    {
        std::string_view name;
        size_t coutStopOnRoute;
        size_t uniqStops;
        double routeLength;
        double curvature;
        bool isFound = true;
    };

    struct StopOut
    {
        std::string_view name;
        std::set<std::string_view> buses;
        bool isFound = true;
    };

    using RelationStopToStop = std::pair<const Stop *, const Stop *>;

    struct RelationStopToStopHasher
    {
        size_t operator()(const RelationStopToStop &tmp) const
        {
            size_t res = 0;
            size_t left = stop_hasher_(tmp.first);
            size_t right = stop_hasher_(tmp.second);
            res = left + right * 37;
            return res;
        }
        std::hash<const Stop *> stop_hasher_;
    };
}