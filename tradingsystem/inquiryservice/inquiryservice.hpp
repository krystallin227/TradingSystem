/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "..\soa.hpp"
#include "..\tradebookingservice\tradebookingservice.hpp"
#include "..\bondstaticdata.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
  Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

  //default ctor
  Inquiry() = default;
  // Get the inquiry ID
  const string& GetInquiryId() const;

  // Get the product
  const T& GetProduct() const;

  // Get the side on the inquiry
  Side GetSide() const;

  // Get the quantity that the client is inquiring for
  long GetQuantity() const;

  // Get the price that we have responded back with
  double GetPrice() const;

  // Get the current state on the inquiry
  InquiryState GetState() const;

  //set the quoted price of inquiry
  void SetPrice(double _price);

  //set the state of inquiry
  void SetState(InquiryState _state);


private:
  string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T& _product, Side _side, long _quantity, double _price, InquiryState _state) :
	product(_product)
{
	inquiryId = _inquiryId;
	side = _side;
	quantity = _quantity;
	price = _price;
	state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
	return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
	return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
	return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
	return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
	return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
	return state;
}

//set the quoted price of inquiry
template<typename T>
void Inquiry<T>::SetPrice(double _price)
{
	price = _price;
}

//set the state of inquiry
template<typename T>
void Inquiry<T>::SetState(InquiryState _state)
{
	state = _state;
}

//pre-declarations
template<typename T>
class InquiryDataConnector;

/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{
private:
	map<string, Inquiry<T>> inquiries;
	vector<ServiceListener<Inquiry<T>>*> listeners;
	InquiryDataConnector<T>* connector;

public:

	// Constructor and destructor
	InquiryService();
	~InquiryService();

	// Get data on our service given a key
	Inquiry<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Inquiry<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Inquiry<T>>* _listener);

	// Get the connector of the service
	InquiryDataConnector<T>* GetConnector() const;


	// Get all listeners on the Service
	const vector<ServiceListener<Inquiry<T>>*>& GetListeners() const;

	// Send a quote back to the client
	void SendQuote(const string &inquiryId, double price);

	// Reject an inquiry from the client
	void RejectInquiry(const string &inquiryId);

};

template<typename T>
InquiryService<T>::InquiryService()
{
	inquiries = map<string, Inquiry<T>>();
	listeners = vector<ServiceListener<Inquiry<T>>*>();
	connector = new InquiryDataConnector<T>(this);

}

template<typename T>
InquiryService<T>::~InquiryService() {}

template<typename T>
Inquiry<T>& InquiryService<T>::GetData(string _key)
{
	return inquiries[_key];
}

template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T>& _data)
{
	string inquiry_id = _data.GetInquiryId();
	inquiries[inquiry_id] = _data;

	//only send quote when the inquiry is in RECEIVED state
	if (_data.GetState() == RECEIVED)
	{
		SendQuote(inquiry_id, 100);
	}
}

template<typename T>
void InquiryService<T>::AddListener(ServiceListener<Inquiry<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Inquiry<T>>*>& InquiryService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
InquiryDataConnector<T>* InquiryService<T>::GetConnector() const
{
	return connector;
}

// Send a quote back to the client
template<typename T>
void InquiryService<T>::SendQuote(const string& inquiryId, double price)
{
	Inquiry<T> inquiry = inquiries[inquiryId];
	inquiry.SetPrice(price);

	//publish the quote to connector
	connector->Publish(inquiry);
}

// Reject an inquiry from the client
template<typename T>
void InquiryService<T>::RejectInquiry(const string& inquiryId)
{
	inquiries[inquiryId].SetState(REJECTED);
}


template<typename T>
class InquiryDataConnector :public Connector<Inquiry<T>>
{
private:

	InquiryService<T>* service;


public:

	InquiryDataConnector<T>(InquiryService<T>* _service);
	~InquiryDataConnector<T>();

	// Publish data to the Connector
	void Publish(Inquiry<T>& _data);

	// Subscribe data from the Connector
	void Subscribe(ifstream& _data);

};


template<typename T>
InquiryDataConnector<T>::InquiryDataConnector(InquiryService<T>* _service)
{
	service = _service;
}

template<typename T>
InquiryDataConnector<T>::~InquiryDataConnector() {}

template<typename T>
void InquiryDataConnector<T>::Publish(Inquiry<T>& _data) 
{

	//transition the inquiry to Quoted State
	_data.SetState(QUOTED);

	//send the inquiry back to service
	service->OnMessage(_data);

	//immediately set the state to done and send back
	_data.SetState(DONE);

	service->OnMessage(_data);
}

template<typename T>
void InquiryDataConnector<T>::Subscribe(ifstream& _data)
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

		//string _inquiryId, const T& _product, Side _side, long _quantity, double _price, InquiryState _state
		//10484181 - 031e-4b38 - b2e3 - 8c76d2cf940d, 2Y, BUY, 146000, 100.83203125
		T b = get_product<T>(splittedItems[1]);
		Side _side = splittedItems[2] == "BUY" ? BUY : SELL;
		Inquiry<T> inquiry(splittedItems[0], b,  _side, std::stod(splittedItems[3]), fractional_to_decimal(splittedItems[4]), RECEIVED);
		service->OnMessage(inquiry);

	}

	_data.close();
}

#endif
