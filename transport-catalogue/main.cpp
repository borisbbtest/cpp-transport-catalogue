#include <iostream>
#include <iomanip>
#include "input_reader.h"

using namespace std;
using namespace transport_catalog;
int main()
{

    reader::input::InputReader ir;
    reader::utils::LoadStreamFlowData(ir, std::cin);

    const transport_catalog::TransportCatalogue &tr = ir.GetTransportCatalogues();

    reader::stat::StatReader out(tr, std::cout);
    reader::utils::LoadOutStreamFlowData(out, std::cin);
    // out.StatToBus();
    // out.StatToStop();
    out.Stat();
    return 0;
}