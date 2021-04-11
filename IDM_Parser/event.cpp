#include "event.h"

using namespace std;

Event::Event():
	_shot(0),
	_is_ok(0),
	_detector(Detector::UNUSED),
	_ampl(0),
	_time(0)
{

}

Event::Event(
	size_t shot,
	size_t is_ok,
	Detector detector,
	double ampl,
	double time
):
	_shot(shot), _is_ok(is_ok), _detector(detector),
	_ampl(ampl), _time(time)
{
}

size_t Event::Shot() const {
	return _shot;
}
size_t Event::IsOK() const {
	return static_cast<size_t>(_is_ok);
}
size_t Event::Det() const {
	if (_detector == Detector::SCINT) {
		return 1;
	}
	else if (_detector == Detector::NEUTRON) {
		return 3;
	}
	else {
		return 0;
	}
}

Detector Event::dType() const {
	return _detector;
}

double Event::Ampl() const {
	return _ampl;
}
double Event::Time() const {
	return _time;
}

istream& operator>>(istream& is, Event& event) {
	string shot;
	string is_ok;
	string detector;
	string ampl = "0";
	string time = "0";
	getline(is, shot, '	');
	getline(is, is_ok, '	');
	getline(is, detector, '	');
	getline(is, ampl, '	');
	getline(is, time, '\n');
	//time = string(time.begin(), time.end());

	//Number of shot
	event._shot = static_cast<size_t>(stoi(shot));
	//Is shot OK +/-
	if (is_ok == "+") {
		event._is_ok = true;
	}
	else {
		event._is_ok = false;
	}
	//Detector type
	if (detector == "1" || detector == "5") {
		event._detector = Detector::SCINT;
	}
	else if (detector == "3" || detector == "7") {
		event._detector = Detector::NEUTRON;
	}
	else {
		event._detector = Detector::UNUSED;
	}
	//Amplitude
	event._ampl = -1.0 * stod(ampl);
	//Time
	event._time = stod(time);
	return is;
}

CollectionOfEvents::CollectionOfEvents() {
	_shot = 0;
	_num_neutron = 0;
	_num_scint = 0;
	_trigger_time = 0;
}
CollectionOfEvents::CollectionOfEvents(const std::pair<size_t, std::vector<Event>>& col) {
	_shot = col.first;

	bool trigFound = false;
	for (const auto& event_ : col.second) {
		if (event_.dType() == Detector::NEUTRON) {
			if (!trigFound) {
				_trigger_time = event_.Time();
				trigFound = true;
			}
			neutr_events.push_back(event_);
			_num_neutron++;
		}
		else if (event_.dType() == Detector::SCINT) {
			scint_events.push_back(event_);
			_num_scint++;
		}
	}
}

vector<double> CollectionOfEvents::neutr_time() {
	vector<double> times;
	times.reserve(_num_neutron);
	for (const auto& item : neutr_events) {
		double nt = item.Time() - _trigger_time;
		if (nt != 0) {
			times.push_back(move(nt));
		}
	}
	return times;
}

vector<double> CollectionOfEvents::scint_time() {
	vector<double> times;
	times.reserve(_num_scint);
	for (const auto& item : scint_events) {
		times.push_back(item.Time() - _trigger_time);
	}
	return times;
}

vector<Event> CollectionOfEvents::Neutr_events() const{
	return neutr_events;
}

vector<Event> CollectionOfEvents::Scint_events() const{
	return scint_events;
}

size_t CollectionOfEvents::Shot() {
	return _shot;
}

size_t CollectionOfEvents::Neutrons() {
	return _num_neutron;
}

size_t CollectionOfEvents::Scint() {
	return _num_scint;
}

double CollectionOfEvents::Trig() {
	return _trigger_time;
}