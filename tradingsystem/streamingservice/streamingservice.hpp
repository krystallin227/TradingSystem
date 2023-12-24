/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "..\soa.hpp"
#include "..\marketdataservice\marketdataservice.hpp"
#include "..\pricingservice\pricingservice.hpp"

/**
 * A price stream order with price and quantity (visible and hidden)
 */
class PriceStreamOrder
{

public:

  // ctor for an order
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // The side on this order
  PricingSide GetSide() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity on this order
  long GetHiddenQuantity() const;

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;

};


PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
	price = _price;
	visibleQuantity = _visibleQuantity;
	hiddenQuantity = _hiddenQuantity;
	side = _side;
}

double PriceStreamOrder::GetPrice() const
{
	return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
	return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
	return hiddenQuantity;
}


/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

  // ctor
  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;

};

template<typename T>
PriceStream<T>::PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder) :
	product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
	return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
	return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
	return offerOrder;
}


/**
* An algo stream - references a PriceStream which contains both bid and offer
* Type T is the product type.
*/
template<typename T>
class AlgoStream
{
private:
	PriceStream<T>* priceStream;

public:

	// constructors
	AlgoStream() = default;
	AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);

	// Get price stream
	PriceStream<T>* GetPriceStream() const;
};

template<typename T>
AlgoStream<T>::AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder)
{
	priceStream = new PriceStream<T>(_product, _bidOrder, _offerOrder);
}

template<typename T>
PriceStream<T>* AlgoStream<T>::GetPriceStream() const
{
	return priceStream;
}

template <typename T>
class AlgoStreamingToPricingListener;

/**
 * Streaming service to send the bid/offer prices to the BondStreamingService
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>>
{
private:

	map<string, AlgoStream<T>> algo_streams;
	vector<ServiceListener<AlgoStream<T>>*> listeners;
	ServiceListener<Price<T>>* stream_to_price_listener;
	long count;

public:

	// Constructor and destructor
	AlgoStreamingService();
	~AlgoStreamingService();

	// Get data on our service given a key
	AlgoStream<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(AlgoStream<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<AlgoStream<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const;

	// Get the listener of the service
	ServiceListener<Price<T>>* GetListener();

	// Publish two-way prices
	void AlgoPublishPrice(Price<T>& _price);

};

template<typename T>
AlgoStreamingService<T>::AlgoStreamingService()
{
	algo_streams = map<string, AlgoStream<T>>();
	listeners = vector<ServiceListener<AlgoStream<T>>*>();
	stream_to_price_listener = new AlgoStreamingToPricingListener<T>(this);
	count = 0;
}

template<typename T>
AlgoStreamingService<T>::~AlgoStreamingService() {}

template<typename T>
AlgoStream<T>& AlgoStreamingService<T>::GetData(string _key)
{
	return algo_streams[_key];
}

template<typename T>
void AlgoStreamingService<T>::OnMessage(AlgoStream<T>& _data)
{
	string product_id = _data.GetPriceStream()->GetProduct().GetProductId();
	algo_streams[product_id] = _data;
}

template<typename T>
void AlgoStreamingService<T>::AddListener(ServiceListener<AlgoStream<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<AlgoStream<T>>*>& AlgoStreamingService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
ServiceListener<Price<T>>* AlgoStreamingService<T>::GetListener()
{
	return stream_to_price_listener;
}

template<typename T>
void AlgoStreamingService<T>::AlgoPublishPrice(Price<T>& _price)
{
	T _product = _price.GetProduct();
	string _productId = _product.GetProductId();

	double _mid = _price.GetMid();
	double _bidOfferSpread = _price.GetBidOfferSpread();
	double _bidPrice = _mid - _bidOfferSpread / 2.0;
	double _offerPrice = _mid + _bidOfferSpread / 2.0;
	long _visibleQuantity = (count % 2 + 1) * 10000000;
	long _hiddenQuantity = _visibleQuantity * 2;

	count++;
	PriceStreamOrder _bidOrder(_bidPrice, _visibleQuantity, _hiddenQuantity, BID);
	PriceStreamOrder _offerOrder(_offerPrice, _visibleQuantity, _hiddenQuantity, OFFER);
	AlgoStream<T> _algoStream(_product, _bidOrder, _offerOrder);
	algoStreams[_productId] = _algoStream;

	for (auto& l : listeners)
	{
		l->ProcessAdd(_algoStream);
	}
}

/**
* Algo Streaming Service Listener subscribing data from Pricing Service to Algo Streaming Service.
* Type T is the product type.
*/
template<typename T>
class AlgoStreamingToPricingListener : public ServiceListener<Price<T>>
{

private:

	AlgoStreamingService<T>* service;

public:

	// Connector and Destructor
	AlgoStreamingToPricingListener(AlgoStreamingService<T>* _service);
	~AlgoStreamingToPricingListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(Price<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Price<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Price<T>& _data);

};

template<typename T>
AlgoStreamingToPricingListener<T>::AlgoStreamingToPricingListener(AlgoStreamingService<T>* _service)
{
	service = _service;
}

template<typename T>
AlgoStreamingToPricingListener<T>::~AlgoStreamingToPricingListener() {}

template<typename T>
void AlgoStreamingToPricingListener<T>::ProcessAdd(Price<T>& _data)
{
	service->AlgoPublishPrice(_data);
}

template<typename T>
void AlgoStreamingToPricingListener<T>::ProcessRemove(Price<T>& _data) {}

template<typename T>
void AlgoStreamingToPricingListener<T>::ProcessUpdate(Price<T>& _data) {}



/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{

public:

  // Publish two-way prices
  void PublishPrice(const PriceStream<T>& priceStream) = 0;

};



#endif
