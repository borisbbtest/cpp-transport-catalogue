#pragma once
#include <string>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <set>
#include <vector>
#include "domain.h"
namespace transport_catalog
{

    using namespace transport_catalog::domain;
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
        std::vector<const Bus *> GetBusesVector() const;
        const std::unordered_map<std::string_view, const Stop *> &GetAllStop() const;
        const std::unordered_map<std::string_view, const Bus *> &GetAllBus() const;
        const std::unordered_map<std::string_view, std::set<std::string_view>> &StopToBus() const;
        const Stop *FindStop(std::string_view name) const;
        const Bus *FindBus(std::string_view name) const;

        BusOut GetBusInfo(std::string_view name) const;
        StopOut GetStopInfo(std::string_view name) const;
    };

}
