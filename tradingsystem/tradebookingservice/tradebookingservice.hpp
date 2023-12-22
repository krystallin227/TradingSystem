/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham
 * @author Krystal Lin
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "..\soa.hpp"
#include "..\bondstaticdata.hpp"
#include "..\util.hpp"


// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

  // ctor for a trade
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);
  
  //default ctor
  Trade() = default;

  // Get the product
  const T& GetProduct() const;

  // Get the trade ID
  const string& GetTradeId() const;

  // Get the mid price
  double GetPrice() const;

  // Get the book
  const string& GetBook() const;

  // Get the quantity
  long GetQuantity() const;

  // Get the side
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;

};


template<typename T>
Trade<T>::Trade(const T& _product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
    product(_product)
{
    tradeId = _tradeId;
    price = _price;
    book = _book;
    quantity = _quantity;
    side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
    return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
    return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
    return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
    return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
    return side;
}

//predeclaration
template<typename T>
class TradeBookingConnector;

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{
private:
    map<string, Trade<T>> trades;
    vector<ServiceListener<Trade<T>>*> listeners;
    TradeBookingConnector<T>* connector;

public:

    // Constructor and destructor
    TradeBookingService();
    ~TradeBookingService();

    // Get data on our service given a key
    Trade<T>& GetData(string _key);

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Trade<T>& _data);

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Trade<T>>* _listener);

    // Get the connector of the service
    TradeBookingConnector<T>* GetConnector() const;

    // Get all listeners on the Service
    const vector<ServiceListener<Trade<T>>*>& GetListeners() const;

    // Book the trade
    void BookTrade(const Trade<T> &trade);


};

template<typename T>
void TradeBookingService<T>::BookTrade(const Trade<T>& trade)
{
    //book trade
    trades[trade.GetTradeId()] = trade;
}

template<typename T>
TradeBookingService<T>::TradeBookingService()
{
    trades = map<string, Trade<T>>();
    listeners = vector<ServiceListener<Trade<T>>*>();
    connector = new TradeBookingConnector<T>(this);
}

template<typename T>
TradeBookingService<T>::~TradeBookingService() {}

template<typename T>
Trade<T>& TradeBookingService<T>::GetData(string _key)
{
    return trades[_key];
}

template<typename T>
void TradeBookingService<T>::OnMessage(Trade<T>& _data)
{
    this->BookTrade(_data);
    for (auto& l : listeners)
    {
        l->ProcessAdd(_data);
    }
}

template<typename T>
void TradeBookingService<T>::AddListener(ServiceListener<Trade<T>>* _listener)
{
    listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Trade<T>>*>& TradeBookingService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
TradeBookingConnector<T>* TradeBookingService<T>::GetConnector() const
{
    return connector;
}


/**
*Booking Connector subscribing data to Booking Service.
*/
template<typename T>
class TradeBookingConnector : public Connector<Trade<T>>
{

private:

    TradeBookingService<T>* service;

public:

    // Connector and Destructor
    TradeBookingConnector(TradeBookingService<T>* _service);
    ~TradeBookingConnector();

    // Publish data to the Connector
    void Publish(Trade<T>& _data);
    void Subscribe(ifstream& _data);

};




template<typename T>
TradeBookingConnector<T>::TradeBookingConnector(TradeBookingService<T>* _service)
{
    service = _service;
}

template<typename T>
TradeBookingConnector<T>::~TradeBookingConnector() {}

template<typename T>
void TradeBookingConnector<T>::Publish(Trade<T>& _data) 
{
}

template<typename T>
void TradeBookingConnector<T>::Subscribe(ifstream& _data)
{
    if (!_data.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return;
    }

    std::string line;
    while (getline(_data, line)) {
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> splittedItems;


        while (getline(ss, item, ','))
        {
            splittedItems.push_back(item);
        }

        T b = get_product<T>(splittedItems[0]);
        Trade<T> trade(b, splittedItems[1], fractional_to_decimal(splittedItems[2]), splittedItems[3], std::stod(splittedItems[4]), splittedItems[5] == "BUY" ?BUY:SELL);
        service->OnMessage(trade);

    }

    _data.close();


}


#endif
