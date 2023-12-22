#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "..\util.hpp"
#include "..\products.hpp"
#include "pricingservice.hpp"

int main() {
  //  Bond b;

    std::cout << "hello" << std::endl;
    PricingService<Bond>* bond_pricing_service = new PricingService<Bond>();
    std::string filename = "prices.txt";
    std::ifstream file(filename);

    PricingConnector<Bond>* bond_pricing_connector = bond_pricing_service->GetConnector();

    bond_pricing_connector->Subscribe(file);


    return 0;
}
