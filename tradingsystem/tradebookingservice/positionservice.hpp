/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "..\soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{
public:

  // ctor for a position
  Position(const T &_product);

  //default ctor
  Position() = default;


  // Get the product
  const T& GetProduct() const;

  // Get the position quantity
  long GetPosition(string &book);

  // Get the aggregate position
  long GetAggregatePosition();

  //update the position quantity for a particular book
  void UpdatePosition(string& book, long quantity);

private:
  T product;
  map<string,long> positions;

};

template<typename T>
Position<T>::Position(const T& _product) :
	product(_product)
{
	//initialize positions for all books
	positions = { {"TRSY1", 0}, {"TRSY2", 0} , {"TRSY3", 0} };
}


template<typename T>
void Position<T>::UpdatePosition(string& book, long quantity)
{
	//update the position quantity for a particular book
	positions[book] += quantity;
}

template<typename T>
const T& Position<T>::GetProduct() const
{
	return product;
}

template<typename T>
long Position<T>::GetPosition(string& book)
{
	return positions[book];
}

//return the aggregate position for this product across all books
template<typename T>
long Position<T>::GetAggregatePosition()
{
	long agg_position = 0;

	for (const auto& pair : positions)
	{
		agg_position += pair.second;
	}

	return agg_position;
}


//Pre-declearations
template<typename T>
class PositionToTradeBookingListener;


/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{

private:

	map<string, Position<T>> positions;
	vector<ServiceListener<Position<T>>*> listeners;
	PositionToTradeBookingListener<T>* listener_to_trade_booking;
		 
public:

	// Constructor and destructor
	PositionService();
	~PositionService();

	// Get data on our service given a key
	Position<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Position<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Position<T>>* _listener);

	PositionToTradeBookingListener<T>* GetListener() const;

	// Get all listeners on the Service
	const vector<ServiceListener<Position<T>>*>& GetListeners() const;

	// Add a trade to the service
	void AddTrade(const Trade<T> &trade);



};

template<typename T>
PositionService<T>::PositionService()
{
	positions = map<string, Position<T>>();
	listeners = vector<ServiceListener<Position<T>>*>();
	listener_to_trade_booking = new PositionToTradeBookingListener<T>(this);

}

template<typename T>
PositionService<T>::~PositionService() {}

template<typename T>
Position<T>& PositionService<T>::GetData(string _key)
{
	return positions[_key];
}

template<typename T>
void PositionService<T>::OnMessage(Position<T>& _data)
{
	positions[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void PositionService<T>::AddListener(ServiceListener<Position<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Position<T>>*>& PositionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
PositionToTradeBookingListener<T>* PositionService<T>::GetListener() const
{
	return listener_to_trade_booking;
}


template<typename T>
void PositionService<T>::AddTrade(const Trade<T>& trade)
{
	T product = trade.GetProduct();
	string product_id = product.GetProductId();
	auto side = trade.GetSide();
	auto book = trade.GetBook();
	long trade_quantity = side == BUY? trade.GetQuantity() : -trade.GetQuantity();

	//update the cumulative positions
	if (positions.find(product_id) == positions.end()) 
	{  //does not have a position; create a new position.
		Position<T> position(product);
		positions[product_id] = position;
	}
	positions[product_id].UpdatePosition(book, trade_quantity);

	//for listeners, send position associated with new trade.
	Position<T> position_update(product);
	position_update.UpdatePosition(book, trade_quantity);

	for (auto& l : listeners)
	{
		l->ProcessAdd(position_update);
	}
}

/**
* Position Service Listener listening to Trading Booking Service.
* Type T is the product type.
*/
template<typename T>
class PositionToTradeBookingListener : public ServiceListener<Trade<T>>
{

private:

	PositionService<T>* service;

public:

	// Connector and Destructor
	PositionToTradeBookingListener(PositionService<T>* _service);
	~PositionToTradeBookingListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(Trade<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Trade<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Trade<T>& _data);

};


template<typename T>
PositionToTradeBookingListener<T>::PositionToTradeBookingListener(PositionService<T>* _service)
{
	service = _service;
}

template<typename T>
PositionToTradeBookingListener<T>::~PositionToTradeBookingListener() {}

template<typename T>
void PositionToTradeBookingListener<T>::ProcessAdd(Trade<T>& _data)
{
	service->AddTrade(_data);
}

template<typename T>
void PositionToTradeBookingListener<T>::ProcessRemove(Trade<T>& _data) {}

template<typename T>
void PositionToTradeBookingListener<T>::ProcessUpdate(Trade<T>& _data) {}

#endif
