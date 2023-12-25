/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham
 * @author Krystal Lin
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include <iostream>
#include <random>
#include <set>

#include "..\soa.hpp"
#include "..\marketdataservice\marketdataservice.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:

  // ctor for an order
  ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

  //default ctor
  ExecutionOrder() = default;

  // Get the product
  const T& GetProduct() const;

  // Get the order ID
  const string& GetOrderId() const;

  //Get the pricing side
  PricingSide GetPricingSide() const;

  // Get the order type on this order
  OrderType GetOrderType() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity
  long GetHiddenQuantity() const;

  // Get the parent order ID
  const string& GetParentOrderId() const;

  // Is child order?
  bool IsChildOrder() const;

  //key used to persist data in historical data service
  string GetPersistKey() const;

  //data persisted in historical data service
  string GetPersistData() const;

private:
  T product;
  PricingSide side;
  string orderId;
  OrderType orderType;
  double price;
  double visibleQuantity;
  double hiddenQuantity;
  string parentOrderId;
  bool isChildOrder;

};

template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
	product(_product)
{
	side = _side;
	orderId = _orderId;
	orderType = _orderType;
	price = _price;
	visibleQuantity = _visibleQuantity;
	hiddenQuantity = _hiddenQuantity;
	parentOrderId = _parentOrderId;
	isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
	return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
	return orderId;
}

template<typename T>
PricingSide ExecutionOrder<T>::GetPricingSide() const
{
	return side;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
	return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
	return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
	return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
	return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
	return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
	return isChildOrder;
}

//key used to persist data in historical data service
template<typename T>
string ExecutionOrder<T>::GetPersistKey() const
{
	return orderId;
}

//data persisted in historical data service
template<typename T>
string ExecutionOrder<T>::GetPersistData() const
{
	auto now = std::chrono::system_clock::now();
	string s = timeToString(now) + " , " + this->GetPersistKey() + " , ";

	string _side = side == BID ? "BID" : "OFFER";
	s += "Side:" + _side  + " , ";
	s += ("Price:" + std::to_string(price) + " , ");
	s += ("Qty:" + std::to_string(visibleQuantity + hiddenQuantity) + "\n");

	return s;
}


/**
* AlgoExecution
* Type T is the product type.
*/
template<typename T>
class AlgoExecution
{

public:

	// ctor for an order
	AlgoExecution() = default;
	AlgoExecution(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

	// Get the order
	ExecutionOrder<T>* GetExecutionOrder() const;

private:
	ExecutionOrder<T>* execution_order;

};

template<typename T>
AlgoExecution<T>::AlgoExecution(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder)
{
	execution_order = new ExecutionOrder<T>(_product, _side, _orderId, _orderType, _price, _visibleQuantity, _hiddenQuantity, _parentOrderId, _isChildOrder);
}

template<typename T>
ExecutionOrder<T>* AlgoExecution<T>::GetExecutionOrder() const
{
	return execution_order;
}

//random order id generator - implemented by chatgpt
class OrderIDGenerator 
{
private:
	std::set<std::string> generatedIDs;
	std::mt19937 rng;
	std::uniform_int_distribution<> charDist;
	const int idLength;

	std::string generateRandomID() 
	{
		std::string id;
		for (int i = 0; i < idLength; ++i) {
			char c = static_cast<char>('A' + charDist(rng)); // Generate a random character
			id += c;
		}
		return id;
	}

public:
	OrderIDGenerator(int length) : idLength(length) {
		// Initialize the random number generator with a random device
		std::random_device rd;
		rng = std::mt19937(rd());

		// Set the range for the characters, for example, from 'A' to 'Z'
		charDist = std::uniform_int_distribution<>(0, 25);
	}

	std::string generateUniqueID() {
		std::string newID;
		do {
			newID = generateRandomID(); // Generate a random string ID
		} while (generatedIDs.find(newID) != generatedIDs.end()); // Check if it's unique

		generatedIDs.insert(newID); // Insert the unique ID into the set
		return newID;
	}
};

/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class AlgoExecutionToMarketDataListener;

/**
* Service for algo executing orders on an exchange.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{

private:

	map<string, AlgoExecution<T>> algo_executions;
	vector<ServiceListener<AlgoExecution<T>>*> listeners;
	AlgoExecutionToMarketDataListener<T>* exec_to_mkt_listener;
	OrderIDGenerator* order_id_gen;
	double tightest_spread;
	bool bid_side; //indicator that we are on the bid side

public:

	// Constructor and destructor
	AlgoExecutionService();
	~AlgoExecutionService();

	// Get data on our service given a key
	AlgoExecution<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(AlgoExecution<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<AlgoExecution<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<AlgoExecution<T>>*>& GetListeners() const;

	// Get the listener of the service
	AlgoExecutionToMarketDataListener<T>* GetListener();

	// Execute an order on a market
	void AlgoExecuteOrder(OrderBook<T>& _orderBook);

};

template<typename T>
AlgoExecutionService<T>::AlgoExecutionService()
{
	algo_executions = map<string, AlgoExecution<T>>();
	listeners = vector<ServiceListener<AlgoExecution<T>>*>();
	exec_to_mkt_listener = new AlgoExecutionToMarketDataListener<T>(this);
	order_id_gen = new OrderIDGenerator(8);
	tightest_spread = 1.0 / 128.0;
	bid_side = true;
}

template<typename T>
AlgoExecutionService<T>::~AlgoExecutionService() {}

template<typename T>
AlgoExecution<T>& AlgoExecutionService<T>::GetData(string _key)
{
	return algo_executions[_key];
}

template<typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecution<T>& _data)
{
	algo_executions[_data.GetExecutionOrder()->GetProduct().GetProductId()] = _data;
}

template<typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<AlgoExecution<T>>*>& AlgoExecutionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
AlgoExecutionToMarketDataListener<T>* AlgoExecutionService<T>::GetListener()
{
	return exec_to_mkt_listener;
}

template<typename T>
void AlgoExecutionService<T>::AlgoExecuteOrder(OrderBook<T>& _orderBook)
{
	T product = _orderBook.GetProduct();
	string product_id = product.GetProductId();
	string order_id = order_id_gen->generateUniqueID();
	double price;
	long qty;
	PricingSide side;


	Order bid_order = _orderBook.GetBidStack()[0];
	double bid = bid_order.GetPrice();

	Order offer_order = _orderBook.GetOfferStack()[0];
	double offer = offer_order.GetPrice();

	if (offer - bid <= tightest_spread)
	{
		if (bid_side)
		{	//when looking at bid side, we are a seller
			price = bid;
			qty = bid_order.GetQuantity();
			side = BID;
		}
		else
		{	//else, we are buyer
			price = offer;
			qty = offer_order.GetQuantity();
			side = OFFER;
		}
		bid_side =!bid_side;

		AlgoExecution<T> algo_execution(product, side, order_id, MARKET, price, qty, 0, "", false);
		algo_executions[product_id] = algo_execution;

		for (auto& l : listeners)
		{
			l->ProcessAdd(algo_execution);
		}
	}
}

/**
* AlgoExecutionToMarketDataListener listen to updates from MarketDataService.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionToMarketDataListener : public ServiceListener<OrderBook<T>>
{

private:

	AlgoExecutionService<T>* service;

public:

	// Connector and Destructor
	AlgoExecutionToMarketDataListener(AlgoExecutionService<T>* _service);
	~AlgoExecutionToMarketDataListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(OrderBook<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(OrderBook<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(OrderBook<T>& _data);

};

template<typename T>
AlgoExecutionToMarketDataListener<T>::AlgoExecutionToMarketDataListener(AlgoExecutionService<T>* _service)
{
	service = _service;
}

template<typename T>
AlgoExecutionToMarketDataListener<T>::~AlgoExecutionToMarketDataListener() {}

template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessAdd(OrderBook<T>& _data)
{
	service->AlgoExecuteOrder(_data);
}

template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessRemove(OrderBook<T>& _data) {}

template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessUpdate(OrderBook<T>& _data) {}

//pre declaration
template<typename T>
class ExecutionToAlgoExecutionListener;

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class ExecutionService : public Service<string,ExecutionOrder <T> >
{
private:

	map<string, ExecutionOrder<T>> execution_orders;
	vector<ServiceListener<ExecutionOrder<T>>*> listeners;
	ExecutionToAlgoExecutionListener<T>* listener;

public:

	// Constructor and destructor
	ExecutionService();
	~ExecutionService();

	// Get data on our service given a key
	ExecutionOrder<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(ExecutionOrder<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<ExecutionOrder<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const;

	// Get the listener of the service
	ExecutionToAlgoExecutionListener<T>* GetListener();

	// Execute an order on a market
	void ExecuteOrder(ExecutionOrder<T>& order, Market market);

};

template<typename T>
ExecutionService<T>::ExecutionService()
{
	execution_orders = map<string, ExecutionOrder<T>>();
	listeners = vector<ServiceListener<ExecutionOrder<T>>*>();
	listener = new ExecutionToAlgoExecutionListener<T>(this);
}

template<typename T>
ExecutionService<T>::~ExecutionService() {}

template<typename T>
ExecutionOrder<T>& ExecutionService<T>::GetData(string _key)
{
	return execution_orders[_key];
}

template<typename T>
void ExecutionService<T>::OnMessage(ExecutionOrder<T>& _data)
{
	execution_orders[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void ExecutionService<T>::AddListener(ServiceListener<ExecutionOrder<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<ExecutionOrder<T>>*>& ExecutionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
ExecutionToAlgoExecutionListener<T>* ExecutionService<T>::GetListener()
{
	return listener;
}

template<typename T>
void ExecutionService<T>::ExecuteOrder(ExecutionOrder<T>& order, Market market)
{
	string product_id = order.GetProduct().GetProductId();
	execution_orders[product_id] = order;

	for (auto& l : listeners)
	{
		l->ProcessAdd(order);
	}
}

/**
* ExecutionToAlgoExecutionListener listens to updates from AlgoExecutionService.
* Type T is the product type.
*/
template<typename T>
class ExecutionToAlgoExecutionListener : public ServiceListener<AlgoExecution<T>>
{

private:

	ExecutionService<T>* service;

public:

	// Connector and Destructor
	ExecutionToAlgoExecutionListener(ExecutionService<T>* _service);
	~ExecutionToAlgoExecutionListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(AlgoExecution<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(AlgoExecution<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(AlgoExecution<T>& _data);

};

template<typename T>
ExecutionToAlgoExecutionListener<T>::ExecutionToAlgoExecutionListener(ExecutionService<T>* _service)
{
	service = _service;
}

template<typename T>
ExecutionToAlgoExecutionListener<T>::~ExecutionToAlgoExecutionListener() {}

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessAdd(AlgoExecution<T>& _data)
{
	ExecutionOrder<T>* _executionOrder = _data.GetExecutionOrder();
	service->OnMessage(*_executionOrder);
	service->ExecuteOrder(*_executionOrder, BROKERTEC);
}

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessRemove(AlgoExecution<T>& _data) {}

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessUpdate(AlgoExecution<T>& _data) {}




#endif
