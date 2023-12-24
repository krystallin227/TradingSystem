/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham
 * @author Krystal Lin
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "..\soa.hpp"
#include "..\bondstaticdata.hpp"
/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // ctor for a price
  Price(const T &_product, double _mid, double _bidOfferSpread);
  
  //default ctor
  Price() = default;

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

private:
  T product;
  double mid;
  double bidOfferSpread;

};



template<typename T>
Price<T>::Price(const T& _product, double _mid, double _bidOfferSpread) :
    product(_product)
{
    mid = _mid;
    bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
    return product;
}

template<typename T>
double Price<T>::GetMid() const
{
    return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
    return bidOfferSpread;
}



//Pre-declearations
template<typename T>
class PricingConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string,Price <T> >
{

private:

    map<string, Price<T>> prices;
    vector<ServiceListener<Price<T>>*> listeners;
    PricingConnector<T>* connector;

public:

    // Constructor and destructor
    PricingService();
    ~PricingService();

    // Get data on our service given a key
    Price<T>& GetData(string _key);

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Price<T>& _data);

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Price<T>>* _listener);

    // Get all listeners on the Service
    const vector<ServiceListener<Price<T>>*>& GetListeners() const;

    PricingConnector<T>* GetConnector() const;

};


template<typename T>
PricingService<T>::PricingService()
{
    prices = map<string, Price<T>>();
    listeners = vector<ServiceListener<Price<T>>*>();
    connector = new PricingConnector<T>(this);
}

template<typename T>
PricingService<T>::~PricingService() {}

template<typename T>
Price<T>& PricingService<T>::GetData(string _key)
{
    return prices[_key];
}

template<typename T>
void PricingService<T>::OnMessage(Price<T>& _data)
{
    prices[_data.GetProduct().GetProductId()] = _data;

    for (auto& l : listeners)
    {
        l->ProcessAdd(_data);
    }
}

template<typename T>
void PricingService<T>::AddListener(ServiceListener<Price<T>>* _listener)
{
    listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Price<T>>*>& PricingService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
PricingConnector<T>* PricingService<T>::GetConnector() const
{
    return connector;
}


/**
*Pricing Connector subscribing data to Pricing Service.
*/
template<typename T>
class PricingConnector : public Connector<Price<T>>
{

private:

    PricingService<T>* service;

public:

    // Connector and Destructor
    PricingConnector(PricingService<T>* _service);
    ~PricingConnector();

    // Publish data to the Connector
    void Publish(Price<T>& _data);
    void Subscribe(ifstream& _data);

};



template<typename T>
PricingConnector<T>::PricingConnector(PricingService<T>* _service)
{
    service = _service;
}

template<typename T>
PricingConnector<T>::~PricingConnector() {}

template<typename T>
void PricingConnector<T>::Publish(Price<T>& _data) {}

template<typename T>
void PricingConnector<T>::Subscribe(ifstream& _data)
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
        Price<T> price(b, fractional_to_decimal(splittedItems[1]), std::stod(splittedItems[2]));
        service->OnMessage(price);

    }

    _data.close();
}

#endif
