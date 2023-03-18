#pragma once
#include <string>
#include "geo.h"
#include <deque>
#include <string_view>
#include <unordered_map>
#include <set>
#include <vector>
namespace transport_catalog
{
    enum BusType
    {
        Ring,
        Line,
    };
    struct Stop
    {
        std::string name;
        Coordinates coordinates;
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

    class TransportCatalogue
    {
    private:
        // Store buses
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, const Bus *> dictBuses_;
        // Store stop
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Stop *> dictStops_;
        // Store routes to buses
        std::unordered_map<std::string_view, std::set<std::string_view>> stopToBuses_;
        // Calculated distance stop to stop
        std::unordered_map<RelationStopToStop, size_t, RelationStopToStopHasher> distancesToStops_;

    public:
        void AddStop(Stop &stop);
        bool AddDistances(std::string_view stop, std::string_view to_stop, size_t ste_meter);
        void AddBus(Bus &bus);
        const std::unordered_map<std::string_view, const Stop *> &GetAllStop() const;
        const std::unordered_map<std::string_view, const Bus *> &GetAllBus() const;
        const std::unordered_map<std::string_view, std::set<std::string_view>> &StopToBus() const;
        const Stop *FindStop(std::string_view name) const;
        const Bus *FindBus(std::string_view name) const;

        BusOut GetBusInfo(std::string_view name) const;
        StopOut GetStopInfo(std::string_view name) const;
    };

}
