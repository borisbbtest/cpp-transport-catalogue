#include "stat_reader.h"
#include <iostream>
#include <string_view>
#include <string>
#include <iomanip>
#include <iostream>

namespace transport_catalog::reader
{
    namespace stat
    {
        StatReader::StatReader(const transport_catalog::TransportCatalogue &transport_catalogue, std::ostream &output)
            : transport_catalogue_(transport_catalogue), output_(output)
        {
        }

        void StatReader::StatToBus() const
        {
            for (const std::string_view name : requests_bus)
            {
                OutputBusInfo(name);
            }
        }

        void StatReader::Stat() const
        {
            for (const auto &[k, v] : requests_list)
            {
                if (k == TypeStat::Stop)
                {
                    OutputStopInfo(v);
                }
                if (k == TypeStat::Bus)
                {
                    OutputBusInfo(v);
                }
            }
        }

        void StatReader::StatToStop() const
        {
            for (const std::string_view name : requests_stop)
            {
                OutputStopInfo(name);
            }
        }

        void StatReader::OutputBusInfo(std::string_view name) const
        {
            BusOut bus = transport_catalogue_.GetBusInfo(name);

            if (!bus.is_found)
            {
                output_ << "Bus " << bus.name << ": not found\n";
                return;
            }

            output_ << "Bus " << bus.name << ": "
                    << bus.coutStopOnRoute << " stops on route, "
                    << bus.uniqStops << " unique stops, "
                    << bus.routeLength << " route length, "
                    << std::setprecision(6)
                    << bus.curvature << " curvature"
                    <<std::endl;
        }

        void StatReader::OutputStopInfo(std::string_view name) const
        {
            StopOut stop = transport_catalogue_.GetStopInfo(name);

            output_ << "Stop " << stop.name << ": ";
            if (!stop.is_found)
            {
                output_ << "not found\n";
            }

            else if (stop.buses.size() == 0)
            {
                output_ << "no buses\n";
            }

            else
            {
                output_ << "buses";
                for (std::string_view n : stop.buses)
                {
                    output_ << " " << n;
                }
                output_ << "\n";
            }
        }
    }
}
