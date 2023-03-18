#include <iostream>
#include <string_view>
#include <string>
#include "input_reader.h"

namespace transport_catalog::reader
{
    namespace input
    {
        // Parsing Data to Bus
        transport_catalog::Bus InputReader::parsingDataToBusAndRoute(Request &req)
        {
            transport_catalog::Bus buff;
            buff.name = req.name;
            char delim;
            if (req.value.find('>') != std::string::npos)
            {
                delim = '>';
                buff.type = BusType::Ring;
            }
            else
            {
                delim = '-';
                buff.type = BusType::Line;
            }
            auto words = utils::SplitIntoWords(req.value, delim);
            if (words.size() >= 1 and req.name.size() > 0)
            {
                for (std::string_view w : words)
                {
                    auto t = transport_catalog_.FindStop(w);
                    buff.stops.push_back(t);
                }
            }
            if (buff.type == BusType::Line)
            {
                std::vector<const transport_catalog::Stop *> reverse_name;
                std::for_each(std::next(buff.stops.rbegin()), buff.stops.rend(),
                              [&reverse_name](const transport_catalog::Stop * name)
                              { reverse_name.push_back(name); });
                buff.stops.insert(buff.stops.end(), reverse_name.begin(), reverse_name.end());
            }
            return buff;
        }

        // Parsing Data to Stops
        transport_catalog::Stop InputReader::parsingDataToBusStopAndRoutes(Request &req)
        {
            auto words = utils::SplitIntoWords(req.value, ',');
            transport_catalog::Stop buff;
            if (words.size() >= 2 and req.name.size() > 0)
            {
                buff.coordinates.lat = stod(std::string(words.at(0)));
                buff.coordinates.lng = stod(std::string(words.at(1)));
                buff.name = req.name;
            }
            return buff;
        }

        std::vector<Request> &InputReader::GetRequests(RequestTypeInput type)
        {
            if (type == RequestTypeInput::Bus)
            {
                return requests_bus;
            }
            return requests_stop;
        }

        TransportCatalogue &InputReader::GetTransportCatalogues()
        {
            return transport_catalog_;
        }

        void InputReader::RequestStoreCurvature()
        {
            for (const auto &req : requests_stop)
            {
                auto words = utils::SplitIntoWords(req.value, ',');
                for (auto it = words.begin() + 2; it != words.end(); ++it)
                {
                    std::string_view metr = *it, stop_to;
                    stop_to = utils::trim(metr.substr(metr.find('o') + 1));
                    size_t m = stod(std::string(utils::trim(metr.substr(0, metr.find('m')))));
                    transport_catalog_.AddDistances(req.name, stop_to, m);
                    //  std::cout<<req.name<<"--"<<stop_to<<"--"<< m <<std::endl;
                }
            }
        }

        void InputReader::RequestStoreDB(Request &req, RequestTypeInput type, bool store_req = true)
        {
            if (req.type == type)
            {
                if (req.type == RequestTypeInput::Stop)
                {
                    transport_catalog::Stop x = parsingDataToBusStopAndRoutes(req);
                    transport_catalog_.AddStop(x);
                }
                if (req.type == RequestTypeInput::Bus)
                {
                    transport_catalog::Bus x = parsingDataToBusAndRoute(req);
                    transport_catalog_.AddBus(x);
                }
            }
            if (store_req == true)
            {
                if (req.type == RequestTypeInput::Stop)
                {
                    requests_stop.push_back(std::move(req));
                }
                if (req.type == RequestTypeInput::Bus)
                {
                    requests_bus.push_back(std::move(req));
                }
            }
        }
    }

    // Helper functions for reading from various streams. You can tell the producer.
    // Creates a queue or you can submit the queue itself as an input.
    namespace utils
    {
        // Loading data
        void LoadStreamFlowData(InputReader &input_reader, std::istream &input)
        {
            size_t count = 0;
            input >> count;
            std::string str;
            std::getline(input, str);
            for (int i = 0; i < count && std::getline(input, str); ++i)
            {
                auto req = ParsingRequest(str);
                input_reader.RequestStoreDB(req, RequestTypeInput::Stop);
            }
            //
            input_reader.RequestStoreCurvature();

            for (auto &buff : input_reader.GetRequests(RequestTypeInput::Bus))
            {
                input_reader.RequestStoreDB(buff, RequestTypeInput::Bus, false);
            }
        }
        //
        void LoadOutStreamFlowData(StatReader &res, std::istream &input)
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
                    res.requests_list.push_back({TypeStat::Bus, move(req.name)});
                }
                if (req.type == RequestTypeInput::Stop)
                {
                    // res.requests_stop.push_back(move(req.name));
                    // auto *tmp = &res.requests_stop.back();
                    res.requests_list.push_back({TypeStat::Stop, move(req.name)});
                }
            }
        }
        //
        Request ParsingRequest(std::string_view str)
        {
            Request res;
            res.type = RequestTypeInput::BadRequest;
            if (str.substr(0, 4) == "Stop")
            {
                res.type = RequestTypeInput::Stop;
                str.remove_prefix(4);
                res.name = trim(str.substr(0, str.find_first_of(':')));
                res.value = trim(str.substr(str.find_first_of(':') + 1));
            }
            if (str.substr(0, 3) == "Bus")
            {
                res.type = RequestTypeInput::Bus;
                str.remove_prefix(3);
                res.name = trim(str.substr(0, str.find_first_of(':')));
                res.value = trim(str.substr(str.find_first_of(':') + 1));
            }
            return res;
        }
        Request ParsingRequestTypeIn(std::string_view str)
        {
            Request res;
            res.type = RequestTypeInput::BadRequest;
            if (str.substr(0, 4) == "Stop")
            {
                res.type = RequestTypeInput::Stop;
                str.remove_prefix(5);
                trim(str);
                res.name = str;
            }
            if (str.substr(0, 3) == "Bus")
            {
                res.type = RequestTypeInput::Bus;
                str.remove_prefix(4);
                trim(str);
                res.name = str;
            }
            return res;
        }
        InputReader LoadStreamFlowData(std::istream &input)
        {
            InputReader res;
            LoadStreamFlowData(res, input);
            return res;
        }

        // trim from start
        std::string &ltrim(std::string &s)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                            std::not1(std::ptr_fun<int, int>(std::isspace))));
            return s;
        }

        // trim from end
        std::string &rtrim(std::string &s)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(),
                                 std::not1(std::ptr_fun<int, int>(std::isspace)))
                        .base(),
                    s.end());
            return s;
        }

        // trim from both ends. delete the firs and last backspaces.
        std::string &trim(std::string &s)
        {
            return ltrim(rtrim(s));
        }

        std::string_view trim(std::string_view str)
        {
            return str.substr(str.find_first_not_of(' '), str.find_last_not_of(' ') - str.find_first_not_of(' ') + 1);
        }

        std::vector<std::string_view> SplitIntoWords(std::string_view str, char delimiter)
        {
            std::vector<std::string_view> result;
            while (true)
            {
                uint64_t space = str.find(delimiter);
                if (str[0] != delimiter)
                {
                    result.push_back(trim(str.substr(0, space)));
                }
                if (space == str.npos)
                {
                    break;
                }
                else
                {
                    str.remove_prefix(space + 1);
                }
            }
            return result;
        }

    }
}