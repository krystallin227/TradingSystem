#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "..\util.hpp"
#include "..\products.hpp"
#include "tradebookingservice.hpp"
#include "positionservice.hpp"
#include "riskservice.hpp"

int main() {

    //create a trade booking service and subscribe to the booking connector to get trade data
    TradeBookingService<Bond>* bond_booking_service = new TradeBookingService<Bond>();

    TradeBookingConnector<Bond>* bond_booking_connector = bond_booking_service->GetConnector();

    //create position service and add it to booking service's listeners
    PositionService<Bond>* bond_position_service = new PositionService<Bond>();
    PositionToTradeBookingListener<Bond>* pos_to_trade_booking_listener = bond_position_service->GetListener();
    bond_booking_service->AddListener(pos_to_trade_booking_listener);


    //create risk service and add it to position service's listeners
    RiskService<Bond>* bond_risk_service = new RiskService<Bond>();
    RiskToPositionListener<Bond>* risk_to_pos_listener = bond_risk_service->GetListener();
    bond_position_service->AddListener(risk_to_pos_listener);



    //start reading trade data
    std::string filename = "trades.txt";
    std::ifstream file(filename);
    bond_booking_connector->Subscribe(file);

    return 0;
}