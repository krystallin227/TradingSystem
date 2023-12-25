/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham
 * @author Krystal Lin
 */

#include "..\soa.hpp"
#include <string>

#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP

//BondPositionService, BondRiskService, BondExecutionService, BondStreamingService, and BondInquiryService
enum ServiceType
{
    PositionType,
    RiskType,
    ExecutionType,
    StreamingType,
    InquiryType
};


//pre declaration
template<typename T>
class HistoricalDataConnector;

template<typename T>
class HistoricalDataListener;

/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist. 
 * T Corresponds to the value type of the persisting service. e.g., for BondPositionService, type is Position<Bond> 
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{
private:
    map<string, T> historical_data;
    vector<ServiceListener<T>*> listeners;
    HistoricalDataConnector<T>* connector;
    ServiceListener<T>* listener;
	ServiceType service;

public:

	// Constructor and destructor
	HistoricalDataService(ServiceType _service);
	~HistoricalDataService();

	// Get data on our service given a key
	T& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(T& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<T>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<T>*>& GetListeners() const;

	// Get the connector of the service
	HistoricalDataConnector<T>* GetConnector();

	// Get the listener of the service
	ServiceListener<T>* GetListener();

	// Get the service type that this service is persisting
	ServiceType GetServiceType() const;

	// Persist data to a store
	void PersistData(string persistKey, T& data);
};


template<typename T>
HistoricalDataService<T>::HistoricalDataService(ServiceType _service)
{
	historical_data = map<string, T>();
	listeners = vector<ServiceListener<T>*>();
	connector = new HistoricalDataConnector<T>(this);
	listener = new HistoricalDataListener<T>(this);
	service = _service;
}

template<typename T>
HistoricalDataService<T>::~HistoricalDataService() {}

template<typename T>
T& HistoricalDataService<T>::GetData(string _key)
{
	return historical_data[_key];
}

template<typename T>
void HistoricalDataService<T>::OnMessage(T& _data)
{
	historical_data[_data.GetPersistKey()] = _data;
}

template<typename T>
void HistoricalDataService<T>::AddListener(ServiceListener<T>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<T>*>& HistoricalDataService<T>::GetListeners() const
{
	return listeners;
}


template<typename T>
HistoricalDataConnector<T>* HistoricalDataService<T>::GetConnector()
{
	return connector;
}


template<typename T>
ServiceListener<T>* HistoricalDataService<T>::GetListener()
{
	return listener;
}

template<typename T>
ServiceType HistoricalDataService<T>::GetServiceType() const
{
	return service;
}

template<typename T>
void HistoricalDataService<T>::PersistData(string _persistKey, T& _data)
{
	connector->Publish(_data);
}

/**
* Historical Data Connector outputs data from services into txt files.
* Type T is the data type to persist.
*/
template<typename T>
class HistoricalDataConnector : public Connector<T>
{

private:

	HistoricalDataService<T>* service;

public:

	// Connector and Destructor
	HistoricalDataConnector(HistoricalDataService<T>* _service);
	~HistoricalDataConnector();

	// Publish data to the Connector
	void Publish(T& _data);

	// Subscribe data from the Connector
	void Subscribe(ifstream& _data);

};

template<typename T>
HistoricalDataConnector<T>::HistoricalDataConnector(HistoricalDataService<T>* _service)
{
	service = _service;
}

template<typename T>
HistoricalDataConnector<T>::~HistoricalDataConnector() {}

template<typename T>
void HistoricalDataConnector<T>::Publish(T& _data)
{
	auto service_type = service->GetServiceType();
	string filename;
	
	switch (service_type)
	{
	case PositionType:
		filename = "outputs/positions.txt";
		break;
	case RiskType:
		filename = "outputs/risk.txt";
		break;
	case ExecutionType:
		filename = "outputs/executions.txt";
		break;
	case StreamingType:
		filename = "outputs/streaming.txt";
		break;
	case InquiryType:
		filename = "outputs/allinquiries.txt";
		break;
	}

	// Create an ofstream object - implemented by gpt.
	ofstream outputFile;

	// Open the file in append mode
	outputFile.open(filename, ios::app);

	// Check if the file is open
	if (outputFile.is_open())
	{
		string temp = _data.GetPersistData();
		outputFile << _data.GetPersistData() << "\n";

		// Close the file
		outputFile.close();
	}
	else {
		cout << "Unable to open file";
	}
}

template<typename T>
void HistoricalDataConnector<T>::Subscribe(ifstream& _data) {}

/**
* Historical Data Service Listener subscribing data to Historical Data.
* Type T is the data type to persist.
*/
template<typename T>
class HistoricalDataListener : public ServiceListener<T>
{

private:

	HistoricalDataService<T>* service;

public:

	// Constructor and Destructor
	HistoricalDataListener(HistoricalDataService<T>* _service);
	~HistoricalDataListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(T& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(T& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(T& _data);

};

template<typename T>
HistoricalDataListener<T>::HistoricalDataListener(HistoricalDataService<T>* _service)
{
	service = _service;
}

template<typename T>
HistoricalDataListener<T>::~HistoricalDataListener() {}

template<typename T>
void HistoricalDataListener<T>::ProcessAdd(T& _data)
{
	string key = _data.GetPersistKey();
	service->PersistData(key, _data);
}

template<typename T>
void HistoricalDataListener<T>::ProcessRemove(T& _data) {}

template<typename T>
void HistoricalDataListener<T>::ProcessUpdate(T& _data) {}


#endif
