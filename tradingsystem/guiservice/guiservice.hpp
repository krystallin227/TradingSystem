#ifndef GUI_SERVICE_HPP
#define GUI_SERVICE_HPP

#include "..\soa.hpp"
#include "..\pricingservice\pricingservice.hpp"
#include <ctime>
#include <limits>
#include <fstream>
#include <chrono>
#include "..\util.hpp"

//Pre declearation
template<typename T>
class GUIConnector;

template<typename T>
class GUIToPricingListener;


/**
* Service for outputing GUI with a certain throttle.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class GUIService : Service<string, Price<T>>
{

private:

	map<string, Price<T>> gui_updates;
	vector<ServiceListener<Price<T>>*> listeners;
	GUIConnector<T>* connector;
	ServiceListener<Price<T>>* listener;
	int throttle;
	std::chrono::time_point<std::chrono::system_clock> last_update;

	int max_updates;
	int count; //number of updates

public:

	// Constructor and destructor
	GUIService(int _throttle, int _max_updates);
	~GUIService();

	// Get data on our service given a key
	Price<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Price<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Price<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<Price<T>>*>& GetListeners() const;

	// Get the connector of the service
	GUIConnector<T>* GetConnector();

	// Get the listener of the service
	ServiceListener<Price<T>>* GetListener();

	// Get the throttle of the service
	int GetThrottle() const;

	// Get the millisec of the service
	std::chrono::time_point<std::chrono::system_clock> GetLastUpdate() const;

	// Get the max number of updates needed in GUI
	int GetMaxUpdate() const;

	// Get current number of updates in GUI
	int GetUpdateCount() const;

	//reset the last update time
	void SetLastUpdate(std::chrono::time_point<std::chrono::system_clock>& new_time);

	//update the count number
	void IncrementCount();
};

template<typename T>
GUIService<T>::GUIService(int _throttle, int _max_updates)
{
	gui_updates = map<string, Price<T>>();
	listeners = vector<ServiceListener<Price<T>>*>();
	connector = new GUIConnector<T>(this);
	listener = new GUIToPricingListener<T>(this);
	throttle = _throttle;
	last_update = std::chrono::system_clock::now();//- std::chrono::milliseconds(throttle);// std::numeric_limits<std::time_t>::min();
	max_updates = _max_updates;
	count = 0;
}

template<typename T>
GUIService<T>::~GUIService() {}

template<typename T>
Price<T>& GUIService<T>::GetData(string _key)
{
	return gui_updates[_key];
}

template<typename T>
void GUIService<T>::OnMessage(Price<T>& _data)
{
	gui_updates[_data.GetProduct().GetProductId()] = _data;
	connector->Publish(_data);
}

template<typename T>
void GUIService<T>::AddListener(ServiceListener<Price<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Price<T>>*>& GUIService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
GUIConnector<T>* GUIService<T>::GetConnector()
{
	return connector;
}

template<typename T>
ServiceListener<Price<T>>* GUIService<T>::GetListener()
{
	return listener;
}

template<typename T>
int GUIService<T>::GetThrottle() const
{
	return throttle;
}


template<typename T>
int GUIService<T>::GetMaxUpdate() const
{
	return max_updates;
}

template<typename T>
int GUIService<T>::GetUpdateCount() const
{
	return count;
}

template<typename T>
std::chrono::time_point<std::chrono::system_clock> GUIService<T>::GetLastUpdate() const
{
	return last_update;
}

template<typename T>
void GUIService<T>::SetLastUpdate(std::chrono::time_point<std::chrono::system_clock>& new_time)
{
	last_update = new_time;
}

template<typename T>
void GUIService<T>::IncrementCount()
{
	count++;
}


/**
* GUI Connector publishes data from GUI Service by saving it in gui.txt.
* Type T is the product type.
*/
template<typename T>
class GUIConnector : public Connector<Price<T>>
{

private:

	GUIService<T>* service;

public:

	// Connector and Destructor
	GUIConnector(GUIService<T>* _service);
	~GUIConnector();

	// Publish data to the Connector
	void Publish(Price<T>& _data);

	// Subscribe data from the Connector
	void Subscribe(ifstream& _data);

};

template<typename T>
GUIConnector<T>::GUIConnector(GUIService<T>* _service)
{
	service = _service;
}

template<typename T>
GUIConnector<T>::~GUIConnector() {}

template<typename T>
void GUIConnector<T>::Publish(Price<T>& _data)
{
	auto throttle = service->GetThrottle();
	auto last_update = service->GetLastUpdate();
	auto now_t = std::chrono::system_clock::now();
	auto now = std::chrono::system_clock::to_time_t(now_t);

	int max_update = service->GetMaxUpdate();
	int current_update = service->GetUpdateCount();

	//time diff in milliseconds
	auto temp = static_cast<long long>(now - std::chrono::system_clock::to_time_t(last_update)) * 1000;
	if (temp >= throttle && current_update < max_update)
	{
		service->IncrementCount();
		service->SetLastUpdate(now_t);

		// Create an ofstream object
		ofstream outputFile;

		// Open the file in append mode
		outputFile.open("gui.txt", ios::app);

		// Check if the file is open
		if (outputFile.is_open()) 
		{
			string product_id = _data.GetProduct().GetProductId();
			double mid = _data.GetMid();
			double bid_ask_spread = _data.GetBidOfferSpread();
			// Write to the file
			outputFile << timeToString(now_t) << " , " << product_id << " , " << mid << " , " << bid_ask_spread <<  "\n";

			// Close the file
			outputFile.close();
		}
		else {
			cout << "Unable to open file";
		}
	}
}

template<typename T>
void GUIConnector<T>::Subscribe(ifstream& _data) {}

/**
* GUI Service Listener subscribing data to GUI Data.
* Type T is the product type.
*/
template<typename T>
class GUIToPricingListener : public ServiceListener<Price<T>>
{

private:

	GUIService<T>* service;

public:

	// Connector and Destructor
	GUIToPricingListener(GUIService<T>* _service);
	~GUIToPricingListener();

	// Listener callback to process an add event to the Service
	void ProcessAdd(Price<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Price<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Price<T>& _data);

};

template<typename T>
GUIToPricingListener<T>::GUIToPricingListener(GUIService<T>* _service)
{
	service = _service;
}

template<typename T>
GUIToPricingListener<T>::~GUIToPricingListener() {}

template<typename T>
void GUIToPricingListener<T>::ProcessAdd(Price<T>& _data)
{
	service->OnMessage(_data);
}

template<typename T>
void GUIToPricingListener<T>::ProcessRemove(Price<T>& _data) {}

template<typename T>
void GUIToPricingListener<T>::ProcessUpdate(Price<T>& _data) {}



#endif // !GUI_SERVICE_HPP
