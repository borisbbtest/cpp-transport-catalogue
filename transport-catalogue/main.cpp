#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include "json_reader.h"

using namespace std;
using namespace transport_catalog;
int main()
{

    json_reader::JsonReader x =(std::cin);
    // reader::input::InputReader ir;
    // reader::utils::LoadStreamFlowData(ir, std::cin);

    // const transport_catalog::TransportCatalogue &tr = ir.GetTransportCatalogues();

    // reader::stat::StatReader out(tr, std::cout);
    // reader::utils::LoadOutStreamFlowData(out, std::cin);
    // out.Stat();

    return 0;
}