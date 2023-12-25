#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "inquiryservice.hpp"
#include "..\historicaldataservice\historicaldataservice.hpp"

int main() {

    //create a trade booking service and subscribe to the booking connector to get trade data
    InquiryService<Bond>* inquiry_service = new InquiryService<Bond>();

    InquiryDataConnector<Bond>* inquiry_data_connector = inquiry_service->GetConnector();


    //create historical data service for inquiry_service
    HistoricalDataService< Inquiry<Bond>>  historical_inquiry_service(InquiryType);
    inquiry_service->AddListener(historical_inquiry_service.GetListener());

    //start reading trade data
    std::string filename = "inquiries.txt";
    std::ifstream file(filename);
    inquiry_data_connector->Subscribe(file);

    return 0;
}