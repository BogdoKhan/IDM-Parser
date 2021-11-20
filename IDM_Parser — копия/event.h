#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <map>


enum class Detector {
	SCINT11,
	SCINT12,
	NEUTRON1,
	SCINT21,
	SCINT22,
	NEUTRON2,
	UNUSED
};

class Event {
public:
	Event();
	Event(
		size_t shot,
		size_t is_ok,
		Detector detector,
		double ampl,
		double time
		);
	size_t Shot() const;
	size_t IsOK() const;
	size_t Det() const;
	Detector dType() const;
	double Ampl() const;
	double Time() const;
	friend std::istream& operator>>(std::istream& is, Event& event);

private:
	size_t _shot;
	bool _is_ok;
	Detector _detector;
	double _ampl;
	double _time;
};

class CollectionOfEvents {
public:
	CollectionOfEvents();
	CollectionOfEvents(const std::pair<size_t, std::vector<Event>>& col,
		const double& lowerbnd, const double& upperbnd);

	std::vector<double> neutr_time();
	std::vector<double> neutr_time_OOR();

	std::vector<double> scint_time();

	std::map<Detector, std::vector<Event>> Neutr_events() const;
	std::map<Detector, std::vector<Event>> Neutr_events_OOR() const;

	std::map<Detector, std::vector<Event>> Scint_events() const;
	size_t Shot();
	std::map <Detector, size_t> Neutrons();
	std::map <Detector, size_t> Neutrons_OOR();

	size_t Scint();
	std::map <Detector, double> Trig();
private:
	size_t _shot = 0; //first member of pair

	//maps of neutron multiplicities
	std::map <Detector, size_t> _num_neutron = {
		{Detector::NEUTRON1, 0}, {Detector::NEUTRON2, 0} 
	}; //if Detector::NEUTRON ++num_neutron

	std::map <Detector, size_t> _num_neutron_wRange = {
	{Detector::NEUTRON1, 0}, {Detector::NEUTRON2, 0}
	}; //if Detector::NEUTRON AND within processing range ++num_neutron

	std::map <Detector, size_t> _num_neutron_OOR = {
		{Detector::NEUTRON1, 0}, {Detector::NEUTRON2, 0} 
	}; //if Detector::NEUTRON AND out of lowerbnd; upperbnd -> ++num_neutron

	std::map <Detector, size_t> _num_neutron_CRPT = {
	{Detector::NEUTRON1, 0}, {Detector::NEUTRON2, 0}
	}; //if Detector::NEUTRON AND below threshold ++num_neutron

	size_t _num_scint = 0; //if Detector::SCINT ++num_neutron

	std::map <Detector, double> _trigger_time = {
		{Detector::NEUTRON1, 0}, {Detector::NEUTRON2, 0} 
	}; //make bool, if found, initialize HELPS to find trigger time and set zero to histos

	//make common vectors for times & amplitudes
	std::map<Detector, std::vector<Event>> neutr_events; //all events
	std::map<Detector, std::vector<Event>> neutr_events_wRange; //within processing range
	std::map<Detector, std::vector<Event>> neutr_events_OOR; //out of processing range
	std::map<Detector, std::vector<Event>> neutr_events_CRPT; //corrupted events (below detection threshold)

	std::map<Detector, std::vector<Event>> scint_events;

	double lower_bounds;
	double upper_bounds;
	void PutEventsInDetector(bool& trigger, const Detector& det, const Event& event_);
};

