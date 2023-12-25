cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)
project(TradingSystem)

include_directories(/opt/homebrew/Cellar/boost/1.81.0/include/)

add_executable(executable1
        tradingsystem/inquiryservice/main.cpp
        tradingsystem/tradebookingservice/tradebookingservice.hpp
        tradingsystem/soa.hpp
        tradingsystem/bondstaticdata.hpp
	tradingsystem/util.hpp
	tradingsystem/products.hpp
	tradingsystem/inquiryservice/inquiryservice.hpp
	tradingsystem/historicaldataservice/historicaldataservice.hpp)


add_executable(executable2
        tradingsystem/marketdataservice/main.cpp
	tradingsystem/marketdataservice/marketdataservice.hpp
	tradingsystem/executionservice/executionservice.hpp
	tradingsystem/tradebookingservice/tradebookingservice.hpp
	tradingsystem/tradebookingservice/positionservice.hpp
	tradingsystem/tradebookingservice/riskservice.hpp
	tradingsystem/util.hpp
	tradingsystem/historicaldataservice/historicaldataservice.hpp)


add_executable(executable3
        tradingsystem/pricingservice/main.cpp
        tradingsystem/pricingservice/pricingservice.hpp
	tradingsystem/util.hpp
	tradingsystem/products.hpp
	tradingsystem/pricingservice/pricingservice.hpp
	tradingsystem/streamingservice/streamingservice.hpp
	tradingsystem/guiservice/guiservice.hpp
	tradingsystem/historicaldataservice/historicaldataservice.hpp)


add_executable(executable4
        tradingsystem/tradebookingservice/main.cpp
	tradingsystem/tradebookingservice/positionservice.hpp
        tradingsystem/tradebookingservice/riskservice.hpp
	tradingsystem/tradebookingservice/tradebookingservice.hpp
	tradingsystem/util.hpp
	tradingsystem/products.hpp  	
	tradingsystem/historicaldataservice/historicaldataservice.hpp)


