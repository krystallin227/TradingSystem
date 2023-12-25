/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 * @author Krystal Lin
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

  //default ctor
  PriceStreamOrder() = default;

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

PricingSide PriceStreamOrder::GetSide() const
{
	return side;
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

  //default ctor
  PriceStream() = default;

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

  //key used to persist data in historical data service
  string GetPersistKey() const;

  //data persisted in historical data service
  string GetPersistData() const;


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

//key used to persist data in historical data service
template<typename T>
string PriceStream<T>::GetPersistKey() const
{
	return product.GetProductId();
}

//data persisted in historical data service
template<typename T>
string PriceStream<T>::GetPersistData() const
{
	auto now = std::chrono::system_clock::now();
	string s = timeToString(now) + " , " + this->GetPersistKey() + " , ";

	s += ("BidOrder , Price: " + std::to_string(bidOrder.GetPrice()));
	s += (" , Qty:" + std::to_string(bidOrder.GetHiddenQuantity() + bidOrder.GetVisibleQuantity()) + " , ");
	s += ("OfferOrder , Price: " + std::to_string(offerOrder.GetPrice()));
	s += (" , Qty:" + std::to_string(offerOrder.GetHiddenQuantity() + offerOrder.GetVisibleQuantity()) + " \n ");
	return s;
}



/**
* An algo stream - references a PriceStream which contains both bid and offer
* Type T is the product type.
*/
template<typename T>
class AlgoStream
{
private:
	PriceStream<T>* price_stream;

public:

	AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);

	//default ctor
	AlgoStream() = default;

	// Get price stream
	PriceStream<T>* GetPriceStream() const;
};

template<typename T>
AlgoStream<T>::AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder)
{
	price_stream = new PriceStream<T>(_product, _bidOrder, _offerOrder);
}

template<typename T>
PriceStream<T>* AlgoStream<T>::GetPriceStream() const
{
	return price_stream;
}

//pre declaration
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
	bool even;

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

	// Get the stream_to_price_listener
	ServiceListener<Price<T>>* GetListener();

	// Publish price to listners when the service listern get updates from pricing service
	void PublishPrice(Price<T>& _price);

};

template<typename T>
AlgoStreamingService<T>::AlgoStreamingService()
{
	algo_streams = map<string, AlgoStream<T>>();
	listeners = vector<ServiceListener<AlgoStream<T>>*>();
	stream_to_price_listener = new AlgoStreamingToPricingListener<T>(this);
	even = false;
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
void AlgoStreamingService<T>::PublishPrice(Price<T>& _price)
{
	T product = _price.GetProduct();
	string product_id = product.GetProductId();

	double mid = _price.GetMid();
	double spread = _price.GetBidOfferSpread();
	double bid = mid - spread / 2.0;
	double offer = mid + spread / 2.0;
	long visible_qty = (even + 1) * 10000000;
	long hidden_qty = visible_qty * 2;

	//negate the bool
	even = !even;

	PriceStreamOrder bid_order(bid, visible_qty, hidden_qty, BID);
	PriceStreamOrder offer_order(offer, visible_qty, hidden_qty, OFFER);
	AlgoStream<T> algo_stream(product, bid_order, offer_order);
	algo_streams[product_id] = algo_stream;

	for (auto& l : listeners)
	{
		l->ProcessAdd(algo_stream);
	}
}

/**
* AlgoStreamingToPricingListener listen to updates from Pricing Service
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
	service->PublishPrice(_data);
}

template<typename T>
void AlgoStreamingToPricingListener<T>::ProcessRemove(Price<T>& _data) {}

template<typename T>
void AlgoStreamingToPricingListener<T>::ProcessUpdate(Price<T>& _data) {}


//Pre declearations 
template<typename T>
class StreamingToAlgoStreamingListener;

/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{

private:

	map<string, PriceStream<T>> price_streams;
	vector<ServiceListener<PriceStream<T>>*> listeners;
	ServiceListener<AlgoStream<T>>* stream_to_algo_listener;

public:

	// Constructor and destructor
	StreamingService();
	~StreamingService();


	// Get data on our service given a key
	PriceStream<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(PriceStream<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<PriceStream<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const;

	// Get the stream_to_algo_listener
	ServiceListener<AlgoStream<T>>* GetListener();

	// Publish two-way prices
	void PublishPrice(PriceStream<T>& priceStream);

};


template<typename T>
StreamingService<T>::StreamingService()
{
	price_streams = map<string, PriceStream<T>>();
	listeners = vector<ServiceListener<PriceStream<T>>*>();
	stream_to_algo_listener = new StreamingToAlgoStreamingListener<T>(this);
}

template<typename T>
StreamingService<T>::~StreamingService() {}

template<typename T>
PriceStream<T>& StreamingService<T>::GetData(string _key)
{
	return price_streams[_key];
}

template<typename T>
void StreamingService<T>::OnMessage(PriceStream<T>& _data)
{
	string product_id = _data.GetProduct().GetProductId();
	price_streams[product_id] = _data;
}

template<typename T>
void StreamingService<T>::AddListener(ServiceListener<PriceStream<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<PriceStream<T>>*>& StreamingService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
ServiceListener<AlgoStream<T>>* StreamingService<T>::GetListener()
{
	return stream_to_algo_listener;
}

template<typename T>
void StreamingService<T>::PublishPrice(PriceStream<T>& _price_stream)
{
	for (auto& l : listeners)
	{
		l->ProcessAdd(_price_stream);
	}
}

/**
* StreamingToAlgoStreamingListener listens to updates from Algo Streaming Service.
* Type T is the product type.
*/
template<typename T>
class StreamingToAlgoStreamingListener : public ServiceListener<AlgoStream<T>>
{

private:

	StreamingService<T>* service;

public:

	// Connector and Destructor
	StreamingToAlgoStreamingListener(StreamingService<T>* _service);
	~StreamingToAlgoStreamingListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(AlgoStream<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(AlgoStream<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(AlgoStream<T>& _data);

};

template<typename T>
StreamingToAlgoStreamingListener<T>::StreamingToAlgoStreamingListener(StreamingService<T>* _service)
{
	service = _service;
}

template<typename T>
StreamingToAlgoStreamingListener<T>::~StreamingToAlgoStreamingListener() {}

template<typename T>
void StreamingToAlgoStreamingListener<T>::ProcessAdd(AlgoStream<T>& _data)
{
	PriceStream<T>* price_stream = _data.GetPriceStream();
	service->OnMessage(*price_stream);
	service->PublishPrice(*price_stream);
}

template<typename T>
void StreamingToAlgoStreamingListener<T>::ProcessRemove(AlgoStream<T>& _data) {}

template<typename T>
void StreamingToAlgoStreamingListener<T>::ProcessUpdate(AlgoStream<T>& _data) {}



#endif
