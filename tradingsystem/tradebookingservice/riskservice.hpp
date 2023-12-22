/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "..\soa.hpp"
#include "positionservice.hpp"
#include "..\bondstaticdata.hpp"
/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // ctor for a PV01 value
  PV01(const T &_product, double _pv01, long _quantity);

  //defualt ctor
  PV01() = default;

  // Get the product on this PV01 value
  const T& GetProduct() const;

  // Get the PV01 value
  double GetPV01() const;

  // Get the quantity that this risk value is associated with
  long GetQuantity() const;

  //update the risk quantity
  void UpdateQuantity(long _quantity);

private:
  T product;
  double pv01;
  long quantity;

};

template<typename T>
PV01<T>::PV01(const T& _product, double _pv01, long _quantity) :
	product(_product)
{
	pv01 = _pv01;
	quantity = _quantity;
}

// Get the product on this PV01 value
template<typename T>
const T& PV01<T>::GetProduct() const
{
	return product;
}

// Get the PV01 value
template<typename T>
double PV01<T>::GetPV01() const
{
	return pv01;
}

// Get the quantity that this risk value is associated with
template<typename T>
long PV01<T>::GetQuantity() const
{
	return quantity;
}


template<typename T>
void PV01<T>::UpdateQuantity(long _quantity)
{
	quantity += _quantity;
}


/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

  // ctor for a bucket sector
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

  const string& GetProductId() const;


private:
  vector<T> products;
  string name;

};

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
	products(_products)
{
	name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
	return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
	return name;
}

template<typename T>
const string& BucketedSector<T>::GetProductId() const
{
	return name;
}


//Pre-declearations to avoid errors.
template<typename T>
class RiskToPositionListener;


/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string,PV01 <T> >
{

private:
	map<string, PV01<T>> risks; //risks for a particular security
	vector<ServiceListener<PV01<T>>*> listeners;
	RiskToPositionListener<T>* risk_to_pos_listener;
	
public:

	// Constructor and destructor
	RiskService();
	~RiskService();


	// Get data on our service given a key
	PV01<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(PV01<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<PV01<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<PV01<T>>*>& GetListeners() const;

	// Get the listener of the service
	RiskToPositionListener<T>* GetListener();

	// Add a position that the service will risk
	void AddPosition(Position<T> &position);

	// Get the bucketed risk for the bucket sector
	const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T> &sector) const ;

};

template<typename T>
RiskService<T>::RiskService()
{
	risks = map<string, PV01<T>>();
	listeners = vector<ServiceListener<PV01<T>>*>();
	risk_to_pos_listener = new RiskToPositionListener<T>(this);
}

template<typename T>
RiskService<T>::~RiskService() {}

template<typename T>
PV01<T>& RiskService<T>::GetData(string _key)
{
	return risks[_key];
}

template<typename T>
void RiskService<T>::OnMessage(PV01<T>& _data)
{
	risks[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void RiskService<T>::AddListener(ServiceListener<PV01<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<PV01<T>>*>& RiskService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
RiskToPositionListener<T>* RiskService<T>::GetListener()
{
	return risk_to_pos_listener;
}

template<typename T>
void RiskService<T>::AddPosition(Position<T>& _position)
{
	string product_id = _position.GetProduct().GetProductId();

	if (risks.find(product_id) == risks.end())
	{  //does not have a risk; create a new risk.
		PV01<T> pv01(_position.GetProduct(), get_pv01(product_id), 0);
		risks[product_id] = pv01;
	}

	//get total quantity across all books
	long total_qty = _position.GetAggregatePosition();
	risks[product_id].UpdateQuantity(total_qty);
}

template<typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& _sector) const
{
	double pv01 = 0;

	vector<T>& products = _sector.GetProducts();
	for (auto& p : products)
	{
		string product_id = p.GetProductId();
		pv01 += risks[product_id].GetPV01() * risks[product_id].GetQuantity();
	}

	return PV01<BucketedSector<T>>(_sector, pv01, 1);
}


template<typename T>
class RiskToPositionListener : public ServiceListener<Position<T>>
{

private:

	RiskService<T>* service;

public:

	// Connector and Destructor
	RiskToPositionListener(RiskService<T>* _service);
	~RiskToPositionListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(Position<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Position<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Position<T>& _data);

};

template<typename T>
RiskToPositionListener<T>::RiskToPositionListener(RiskService<T>* _service)
{
	service = _service;
}

template<typename T>
RiskToPositionListener<T>::~RiskToPositionListener() {}

template<typename T>
void RiskToPositionListener<T>::ProcessAdd(Position<T>& _data)
{
	service->AddPosition(_data);
}

template<typename T>
void RiskToPositionListener<T>::ProcessRemove(Position<T>& _data) {}

template<typename T>
void RiskToPositionListener<T>::ProcessUpdate(Position<T>& _data) {}



#endif
