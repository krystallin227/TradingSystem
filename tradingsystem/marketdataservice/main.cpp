#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "marketdataservice.hpp"
#include "..\executionservice\executionservice.hpp"
#include "..\tradebookingservice\tradebookingservice.hpp"
#include "..\tradebookingservice\positionservice.hpp"
#include "..\tradebookingservice\riskservice.hpp"
#include "..\historicaldataservice\historicaldataservice.hpp"


int main() {

    //create a trade booking service and subscribe to the booking connector to get trade data
    MarketDataService<Bond>* market_data_service = new MarketDataService<Bond>(5);
    MarketDataConnector<Bond>* market_data_connector = market_data_service->GetConnector();
    
    //create an algo execution service and subscribe to the market_data_service
    AlgoExecutionService<Bond>* algo_execution_service = new AlgoExecutionService<Bond>();
    market_data_service->AddListener(algo_execution_service->GetListener());

    //create an execution service and subscribe to the algo_execution_service
    ExecutionService<Bond>* execution_service = new ExecutionService<Bond>();
    algo_execution_service->AddListener(execution_service->GetListener());


    //create historical data service for execution_service
    HistoricalDataService< ExecutionOrder<Bond>>  historical_execution_service(ExecutionType);
    execution_service->AddListener(historical_execution_service.GetListener());

    //create a trading service and subscribe to execution_service
    TradeBookingService<Bond>* trading_book_service = new TradeBookingService<Bond>();
    execution_service->AddListener(trading_book_service->GetListener());

    //create position service and add it to booking service's listeners
    PositionService<Bond>* bond_position_service = new PositionService<Bond>();
    PositionToTradeBookingListener<Bond>* pos_to_trade_booking_listener = bond_position_service->GetListener();
    trading_book_service->AddListener(pos_to_trade_booking_listener);


    //create risk service and add it to position service's listeners
    RiskService<Bond>* bond_risk_service = new RiskService<Bond>();
    RiskToPositionListener<Bond>* risk_to_pos_listener = bond_risk_service->GetListener();
    bond_position_service->AddListener(risk_to_pos_listener);


    //start reading market data
    std::string filename = "marketdata.txt";
    std::ifstream file(filename);
    market_data_connector->Subscribe(file);

    return 0;
}