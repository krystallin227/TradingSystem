#ifndef ALGO_STREAM_SERVICE_HPP
#define ALGO_STREAM_SERVICE_HPP

/**
 * algostreamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 * @author Krystal Lin
 */

#include "..\soa.hpp"
#include "..\marketdataservice\marketdataservice.hpp"

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

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

	// ctor
	PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);

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

PricingSide PriceStreamOrder::GetSide() const
{
	return side;
}

/**
* An algo streaming that process algo streaming.
* Type T is the product type.
*/
template<typename T>
class AlgoStream
{

public:

	// ctor for an order
	AlgoStream() = default;
	AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);

	// Get the order
	PriceStream<T>* GetPriceStream() const;

private:
	PriceStream<T>* priceStream;

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

/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string, PriceStream <T> >
{

public:

	// Publish two-way prices
	void PublishPrice(const PriceStream<T>& priceStream) = 0;

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
 * Algo Streamng Service to stream out bid/offer prices to streaming service.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string, AlgoStream<T> >
{


};


#endif // !ALGO_STREAM_SERVICE_HPP
