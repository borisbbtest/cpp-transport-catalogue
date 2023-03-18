#include <iostream>
#include <string_view>
#include <string>
#include <iomanip>
#include <iostream>
#include "stat_reader.h"
namespace transport_catalog::reader
{
    namespace stat
    {
        StatReader::StatReader(const transport_catalog::TransportCatalogue &transport_catalogue, std::ostream &output)
            : transport_catalogue_(transport_catalogue), output_(output)
        {
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

        void StatReader::OutputBusInfo(std::string_view name) const
        {
            BusOut bus = transport_catalogue_.GetBusInfo(name);

            if (!bus.isFound)
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
                    << std::endl;
        }

        void StatReader::OutputStopInfo(std::string_view name) const
        {
            StopOut stop = transport_catalogue_.GetStopInfo(name);

            output_ << "Stop " << stop.name << ": ";
            if (!stop.isFound)
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
    //
    namespace utils
    {

        void LoadOutStreamFlowData(stat::StatReader &res, std::istream &input)
        {
            size_t count = 0;
            input >> count;
            std::string str;
            std::getline(input, str);
            for (int i = 0; i < count && std::getline(input, str); i++)
            {
                auto req = ParsingRequestTypeIn(str);
                if (req.type == RequestTypeInput::Bus)
                {
                    // res.requests_bus.push_back(move(req.name));
                    // auto *tmp = &res.requests_bus.back();
                    res.requests_list.push_back({stat::TypeStat::Bus, move(req.name)});
                }
                if (req.type == RequestTypeInput::Stop)
                {
                    // res.requests_stop.push_back(move(req.name));
                    // auto *tmp = &res.requests_stop.back();
                    res.requests_list.push_back({stat::TypeStat::Stop, move(req.name)});
                }
            }
        }

    }
}
