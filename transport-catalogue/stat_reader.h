#pragma once
#include <vector>
#include <deque>
#include <string>
#include "transport_catalogue.h"
namespace transport_catalog::reader
{
    namespace stat
    {
        enum TypeStat
        {
            Bus,
            Stop,
        };
        class StatReader
        {
        public:
            StatReader(const transport_catalog::TransportCatalogue &transport_catalogue, std::ostream &output);
            StatReader() = default;
            void StatToBus() const;
            void StatToStop() const;
            void Stat() const;
            std::vector<std::string> requests_stop;
            std::vector<std::string> requests_bus;
            std::deque<std::pair<TypeStat, std::string>> requests_list;

        private:
            const transport_catalog::TransportCatalogue &transport_catalogue_;
            std::ostream &output_;

            void OutputBusInfo(std::string_view name) const;
            void OutputStopInfo(std::string_view name) const;
        };
    }

}