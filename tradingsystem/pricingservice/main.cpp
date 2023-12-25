#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "..\util.hpp"
#include "..\products.hpp"
#include "pricingservice.hpp"
#include "..\streamingservice\streamingservice.hpp"
#include "..\guiservice\guiservice.hpp"
#include "..\historicaldataservice\historicaldataservice.hpp"

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


    //create historical data service for streaming_service
    HistoricalDataService< PriceStream<Bond>>  historical_streaming_service(StreamingType);
    streaming_serive->AddListener(historical_streaming_service.GetListener());


    //create a guiservice and connect it to pricing service
    GUIService<Bond>* gui_service = new GUIService<Bond>(300, 1000);
    bond_pricing_service->AddListener(gui_service->GetListener());


    //subscribe connector
    std::string filename = "prices.txt";
    std::ifstream file(filename);
    bond_pricing_connector->Subscribe(file);


    return 0;
}
