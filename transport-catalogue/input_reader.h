#pragma once
#include <vector>
#include <string>
#include <set>
#include <string.h>
#include <algorithm>
#include <iostream>
#include "stat_reader.h"

namespace transport_catalog::reader
{

    namespace input
    {
        enum RequestTypeInput
        {
            Bus,
            Stop,
            BadRequest,
        };
        struct Request
        {
            RequestTypeInput type;
            std::string name;
            std::string value;
        };
        // This class is implemented as a kafka queue. I won't delete line.
        // I will reference  to string. I will using in the future  view_string in   code.
        class InputReader
        {
        private:
            // uniq string
            std::vector<Request> requests_stop; // valid string request to stop
            std::vector<Request> requests_bus;

            TransportCatalogue transport_catalog_;

            // Parsing Data to Bus
            transport_catalog::Bus parsingDataToBusAndRoute(Request &req);
            // Parsing Data to Stops
            transport_catalog::Stop parsingDataToBusStopAndRoutes(Request &req);

        public:
            // This block to transport catalog
            void MoveTransportCatalogues(TransportCatalogue &&alias); // move object to object
            TransportCatalogue &GetTransportCatalogues();             // get transport catalog
                                                                      // Those Constructor and deconstructor ????
            explicit InputReader() = default;
            // We will get requests all
            std::vector<Request> &GetRequests(RequestTypeInput type);
            // Put request
            void RequestStoreDB(Request &req, RequestTypeInput type, bool store_req);
            void RequestStoreCurvature();
        };

    }
    // Helper functions for reading from various streams. You can tell the producer.
    // Creates a queue or you can submit the queue itself as an input.
    namespace utils
    {
        using namespace transport_catalog::reader::stat;
        using namespace transport_catalog::reader::input;

        void LoadStreamFlowData(InputReader &input_reader, std::istream &input); // load stream from flow
        InputReader LoadStreamFlowData(std::istream &input);                     // load stream from flow
        void LoadOutStreamFlowData(StatReader &res, std::istream &input);
        Request ParsingRequestTypeIn(std::string_view str);
        // It checks Where do we put data
        Request ParsingRequest(std::string_view str);

        // trim from start
        std::string &ltrim(std::string &s);

        // trim from end
        std::string &rtrim(std::string &s);

        // trim from both ends. delete the firs and last backspaces.
        std::string &trim(std::string &s);
        std::string_view trim(std::string_view str);
        std::vector<std::string_view> SplitIntoWords(std::string_view str, char delimiter);
    }
}