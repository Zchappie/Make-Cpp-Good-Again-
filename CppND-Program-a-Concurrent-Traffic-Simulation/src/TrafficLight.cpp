#include <iostream>
#include <random>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
	std::unique_lock<std::mutex> uLock(_mutex);
	_condVar.wait(uLock, [this] {return !_queue.empty(); });
	
	// pull the message
	T msg = std::move(_queue.back());
	_queue.pop_back();
	return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
	std::lock_guard<std::mutex> gLock(_mutex);
	
	// add message to the queue, and the queue should be cleared for peripheral lights (intersections with fewer traffics)
	_queue.clear();
	_queue.emplace_back(std::move(msg));
	_condVar.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
= default;

void TrafficLight::waitForGreen()
{
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (_messages.receive() == TrafficLightPhase::green) {
			return;
		}
	}
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}


void TrafficLight::simulate()
{
	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

void TrafficLight::cycleThroughPhases()
{
	std::unique_lock<std::mutex> lck(_mutex);
	std::cout << "Traffic light #" << _id << "::cycleThroughPhases: thread id = " << std::this_thread::get_id() << std::endl;
	lck.unlock();
	
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distribution(4000, 6000);
	auto cycleDuration = distribution(eng);
	//std::cout << "Current cycle duration is " << cycleDuration << std::endl;
	
	std::chrono::time_point<std::chrono::system_clock> lastUpdate;
	lastUpdate = std::chrono::system_clock::now();
	while (true){
		// sleep at every iteration to reduce CPU usage
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//cycleDuration = distribution(eng);
		
		// compute time difference to stop watch
		long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
		if (timeSinceLastUpdate >= cycleDuration){
			
			// traffic light phase toggles between red and greed
			_currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
			_messages.send(std::move(_currentPhase));
			lastUpdate = std::chrono::system_clock::now();
		}
	}
}