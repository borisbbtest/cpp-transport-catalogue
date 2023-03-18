#include "transport_catalogue.h"
#include <iostream>
#include <unordered_set>
namespace transport_catalog
{
    void TransportCatalogue::AddStop(Stop &stop)
    {
        stops_.push_back(std::move(stop));
        dictStops_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddBus(Bus &bus)
    {
        buses_.push_back(std::move(bus));
        auto &buff = buses_.back();
        dictBuses_[buff.name] = &buff;
        for (const auto *tmp : buff.stops)
        {
            stopToBuses_[tmp->name].insert(buff.name);
        }
    }

    const Stop *TransportCatalogue::FindStop(std::string_view name) const
    {
        const auto stop = dictStops_.find(name);

        if (stop == dictStops_.end())
        {
            static Stop empty;
            return &empty;
        }
        return stop->second;
    }

    const Bus *TransportCatalogue::FindBus(std::string_view name) const
    {
        const auto finded_bus = dictBuses_.find(name);
        if (finded_bus == dictBuses_.end())
        {
            static Bus empty;
            return &empty;
        }
        return finded_bus->second;
    }

    bool TransportCatalogue::AddDistances(std::string_view stop, std::string_view to_stop, size_t ste_meter)
    {
        const Stop *fstop = FindStop(stop), *sstop = FindStop(to_stop);
        if (*fstop == Stop{} || *sstop == Stop{})
        {
            return false;
        }
        RelationStopToStop x{fstop, sstop};
        distancesToStops_.insert_or_assign(x, ste_meter);
        return true;
    }

    StopOut TransportCatalogue::GetStopInfo(std::string_view name) const
    {
        const Stop *stop = FindStop(name);
        if (*stop == Stop{})
        {
            StopOut stop_info;
            stop_info.name = name;
            stop_info.isFound = false;
            return stop_info;
        }
        StopOut stop_info;
        stop_info.name = stop->name;
        stop_info.buses = stopToBuses_.find(name) == stopToBuses_.end() ? std::set<std::string_view>{} : stopToBuses_.at(name);
        return stop_info;
    }

    BusOut TransportCatalogue::GetBusInfo(std::string_view name) const
    {
        const Bus *bus = FindBus(name);
        if (*bus == Bus{})
        {
            BusOut bus_info;
            bus_info.name = name;
            bus_info.isFound = false;
            return bus_info;
        }
        std::unordered_set<const Stop *> uniq_stops = {*bus->stops.begin()};

        size_t route_len = 0;
        double straight_way = 0.0;

        Coordinates coordinate_from = (*bus->stops.begin())->coordinates;
        Coordinates coordinate_to;
        const Stop *stop_from = *bus->stops.begin();
        const Stop *stop_to;

        for (auto iter = std::next(bus->stops.begin()); iter != bus->stops.end(); ++iter)
        {
            coordinate_to = (*iter)->coordinates;
            straight_way += ComputeDistance(coordinate_from, coordinate_to);
            coordinate_from = coordinate_to;

            stop_to = *iter;
            if (distancesToStops_.find(RelationStopToStop{stop_from, stop_to}) != distancesToStops_.end())
                route_len += distancesToStops_.at(RelationStopToStop{stop_from, stop_to});
            else
                route_len += distancesToStops_.at(RelationStopToStop{stop_to, stop_from});
            stop_from = stop_to;

            uniq_stops.insert(*iter);
        }

        BusOut bus_info;

        bus_info.name = bus->name;
        bus_info.coutStopOnRoute = bus->stops.size();
        bus_info.uniqStops = uniq_stops.size();
        bus_info.routeLength = route_len;
        bus_info.curvature = route_len / straight_way;

        return bus_info;
    }

    const std::unordered_map<std::string_view, const Stop *> &TransportCatalogue::GetAllStop() const
    {
        return dictStops_;
    }

    const std::unordered_map<std::string_view, const Bus *> &TransportCatalogue::GetAllBus() const
    {
        return dictBuses_;
    }
    const std::unordered_map<std::string_view, std::set<std::string_view>> &TransportCatalogue::StopToBus() const
    {
        return stopToBuses_;
    }
}