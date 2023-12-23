/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include "..\soa.hpp"
#include "..\util.hpp"
#include "..\bondstaticdata.hpp"


using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

  // ctor for an order
  Order(double _price, long _quantity, PricingSide _side);

  // Get the price on the order
  double GetPrice() const;

  // Get the quantity on the order
  long GetQuantity() const;

  // Get the side on the order
  PricingSide GetSide() const;

private:
  double price;
  long quantity;
  PricingSide side;

};


Order::Order(double _price, long _quantity, PricingSide _side)
{
	price = _price;
	quantity = _quantity;
	side = _side;
}

double Order::GetPrice() const
{
	return price;
}

long Order::GetQuantity() const
{
	return quantity;
}

PricingSide Order::GetSide() const
{
	return side;
}


/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

  // ctor for bid/offer
  BidOffer(const Order &_bidOrder, const Order &_offerOrder);

  // Get the bid order
  const Order& GetBidOrder() const;

  // Get the offer order
  const Order& GetOfferOrder() const;

private:
  Order bidOrder;
  Order offerOrder;

};

BidOffer::BidOffer(const Order& _bidOrder, const Order& _offerOrder) :
	bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
	return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
	return offerOrder;
}

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // ctor for the order book
  OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

  // Get the product
  const T& GetProduct() const;

  // Get the bid stack
  const vector<Order>& GetBidStack() const;

  // Get the offer stack
  const vector<Order>& GetOfferStack() const;

private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;

};

template<typename T>
OrderBook<T>::OrderBook(const T& _product, const vector<Order>& _bidStack, const vector<Order>& _offerStack) :
	product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
	return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
	return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
	return offerStack;
}


//predeclaration
template<typename T>
class MarketDataConnector;


/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{

private:

	vector<ServiceListener<OrderBook<T>>*> listeners;
	MarketDataConnector<T>* connector;

	int depth;

	//latest orderbook per instrument
	map<string, OrderBook<T>> order_books;

public:

	// Constructor and destructor
	MarketDataService(int _depth);
	~MarketDataService();


	// Get data on our service given a key
	OrderBook<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(OrderBook<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<OrderBook<T>>* _listener);

	// Get the connector of the service
	MarketDataConnector<T>* GetConnector() const;

	// Get all listeners on the Service
	const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const;

	// Get the best bid/offer order
	const BidOffer& GetBestBidOffer(const string &productId);

	// Aggregate the order book
	const OrderBook<T>& AggregateDepth(const string &productId);

	int GetDepth();

};
template<typename T>
MarketDataService<T>::MarketDataService(int _depth)
{
	order_books = map<string, OrderBook<T>>();
	listeners = vector<ServiceListener<OrderBook<T>>*>();
	connector = new MarketDataConnector<T>(this);
	depth = _depth;
}

template<typename T>
MarketDataService<T>::~MarketDataService(){}


template<typename T>
OrderBook<T>& MarketDataService<T>::GetData(string _key)
{
	return order_books[_key];
}

template<typename T>
void MarketDataService<T>::OnMessage(OrderBook<T>& _data)
{
	string product_id = _data.GetProduct().GetProductID();
	order_books[product_id] = _data;

	for (auto& l : listeners)
	{
		l->ProcessAdd(_data);
	}
}

template<typename T>
void MarketDataService<T>::AddListener(ServiceListener<OrderBook<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<OrderBook<T>>*>& MarketDataService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
MarketDataConnector<T>* MarketDataService<T>::GetConnector() const
{
	return connector;
}


// Get the best bid/offer order
template<typename T>
const BidOffer& MarketDataService<T>::GetBestBidOffer(const string& productId)
{
	Order best_bid = order_books[productId].GetBidStack()[0];
	Order best_offer = order_books[productId].GetOfferStack()[0];
	return BidOffer(best_bid, best_offer)

}
// Aggregate the order book
template<typename T>
const OrderBook<T>& MarketDataService<T>::AggregateDepth(const string& productId)
{
	return order_books[productId];
}

template<typename T>
int MarketDataService<T>::GetDepth()
{
	return depth;
}

/**
* Market Data Connector reads data from marketdata.txt
* Type T is the product type.
*/
template<typename T>
class MarketDataConnector : public Connector<OrderBook<T>>
{

private:

	MarketDataService<T>* service;

public:

	// Connector and Destructor
	MarketDataConnector(MarketDataService<T>* _service);
	~MarketDataConnector();

	// Publish data to the Connector
	void Publish(OrderBook<T>& _data);

	// Subscribe data from the Connector
	void Subscribe(ifstream& _data);

};

template<typename T>
MarketDataConnector<T>::MarketDataConnector(MarketDataService<T>* _service)
{
	service = _service;
}

template<typename T>
MarketDataConnector<T>::~MarketDataConnector() {}

template<typename T>
void MarketDataConnector<T>::Publish(OrderBook<T>& _data) {}

template<typename T>
void MarketDataConnector<T>::Subscribe(ifstream& _data)
{
	if (!_data.is_open()) 
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}
	int depth = service->GetDepth();
	int count == 0;
	std::string line;
	vector<Order> bids; vector<Order> asks;
	Order bid; Order ask;
	double mid;
	double spread;
	long quantity;

	while (getline(_data, line)) {
		count++;
		std::stringstream ss(line);
		std::string item;
		std::vector<std::string> splittedItems;

		while (getline(ss, item, ','))
		{
			splittedItems.push_back(item);
		}

		//2Y,99-00,0.0078125,10000000,10000000
		mid = fractional_to_decimal(splittedItems[1]);
		spread = std::stod(splittedItems[2]);
		quantity = std::stod(splittedItems[3]);

		bid = Order(mid - spread, quantity, BID);
		ask = Order(mid + spread, quantity, ASK);

		bids.push_back(bid);
		asks.push_back(ask);

		if (count % depth == 0)
		{
			vector<Order> bids_order(bids);
			vector<Order> asks_order(asks);
			bids.clear();
			asks.clear();

			OrderBook<T> order_book(get_product(splittedItems[0], bids_order, asks_order);
			service->OnMessage(order_book);
			count = 0;
		}

	}

	_data.close();
}


#endif
