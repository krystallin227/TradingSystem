### Introduction
The final project involves developing a bond trading system for US Treasuries, focusing on seven specific securities. Key components include various services like BondTradeBookingService, BondPositionService, and BondMarketDataService, among others. The project requires the creation of data files and the use of connectors for data flow. 


### Use of ChatGPT
Throughout the project, the usage of ChatGPT are documented for functions created by ChatGPT. These functions mainly include utility function that performs decimal to fractional notation transform for US treasuries prices, as wells as input data generation.

### Services
- MarketDataService: Manages market data updates and connect to execution services.
- AlgoExecutionService: Reads market update and execute algo trading strategy.
- ExecutionService: Executes algo orders.
- TradeBookingService: Tracks executed orders and book trades.
- PositionService: Tracks aggregated and individual book positions.
- RiskService: Tracks aggregated and individual book risks.
- PricingService: Manages pricing data and connect to streaming services.
- AlgoStreamingService: Stream prices related to algo, send requests to streaming service.
- StreamingService: Stream price to external clients/exchanges.
- GUIService: Simulates a GUI that does not receive constant price updates.
- InquiryService: Respond and quote on RFQs.
- HistoricalDataService: Persists all relevant data that could be use for risk analysis, backtesting, etc.

