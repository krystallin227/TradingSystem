#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "..\util.hpp"
#include "..\products.hpp"
#include "pricingservice.hpp"
#include "..\streamingservice\streamingservice.hpp"

int main() 
{
    //create bond pricing service and connect to bond pricing connector
    PricingService<Bond>* bond_pricing_service = new PricingService<Bond>();
    PricingConnector<Bond>* bond_pricing_connector = bond_pricing_service->GetConnector();

    //connect algo streaming service with pricing services
    AlgoStreamingService<Bond>* algo_streaming_service = new AlgoStreamingService<Bond>();
    bond_pricing_service->AddListener(algo_streaming_service->GetListener());

    //connect streaming service with algo streaming service
    StreamingService<Bond>* streaming_serive = new StreamingService<Bond>();
    algo_streaming_service->AddListener(streaming_serive->GetListener());

    //subscribe connector
    std::string filename = "prices.txt";
    std::ifstream file(filename);
    bond_pricing_connector->Subscribe(file);


    return 0;
}
