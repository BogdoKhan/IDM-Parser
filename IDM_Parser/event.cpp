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
	if (_detector == Detector::SCINT21) {
		return 1;
	}
	else if (_detector == Detector::SCINT22) {
		return 2;
	}
	else if (_detector == Detector::NEUTRON2) {
		return 3;
	}
	if (_detector == Detector::SCINT11) {
		return 5;
	}
	else if (_detector == Detector::SCINT12) {
		return 6;
	}
	else if (_detector == Detector::NEUTRON1) {
		return 7;
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
	if (detector == "1") {
		event._detector = Detector::SCINT21;
	}
	else if (detector == "2") {
		event._detector = Detector::SCINT22;
	}
	else if (detector == "3") {
		event._detector = Detector::NEUTRON2;
	}
	else if (detector == "5") {
		event._detector = Detector::SCINT11;
	}
	else if (detector == "6") {
		event._detector = Detector::SCINT12;
	}
	else if (detector == "7") {
		event._detector = Detector::NEUTRON1;
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
	_num_neutron = { {Detector::NEUTRON1, 0}, {Detector::NEUTRON2, 0} };
	_num_scint = 0;
	_trigger_time = { {Detector::NEUTRON1, 0}, {Detector::NEUTRON2, 0} };
}
CollectionOfEvents::CollectionOfEvents(const std::pair<size_t, std::vector<Event>>& col) {
	_shot = col.first;

	bool trigFound2 = false;
	bool trigFound1 = false;
	for (const auto& event_ : col.second) {
		if (event_.dType() == Detector::NEUTRON2) {
			if (!trigFound2) {
				_trigger_time[Detector::NEUTRON2] = event_.Time();
				trigFound2 = true;
			}
			neutr_events[Detector::NEUTRON2].push_back(event_);
			_num_neutron[event_.dType()]++;
		}
		else if (event_.dType() == Detector::NEUTRON1) {
			if (!trigFound1) {
				_trigger_time[Detector::NEUTRON1] = event_.Time();
				trigFound1 = true;
			}
			neutr_events[Detector::NEUTRON1].push_back(event_);
			_num_neutron[event_.dType()]++;
		}
		else if (event_.dType() == Detector::SCINT21) {
			scint_events[Detector::SCINT21].push_back(event_);
			_num_scint++;
		}
		else if (event_.dType() == Detector::SCINT22) {
			scint_events[Detector::SCINT22].push_back(event_);
			_num_scint++;
		}
		else if (event_.dType() == Detector::SCINT11) {
			scint_events[Detector::SCINT11].push_back(event_);
			_num_scint++;
		}
		else if (event_.dType() == Detector::SCINT12) {
			scint_events[Detector::SCINT12].push_back(event_);
			_num_scint++;
		}
	}
}

vector<double> CollectionOfEvents::neutr_time() {
	vector<double> times;
	for (const auto& vec : neutr_events) {
		for (const auto& item : vec.second) {
			double nt = item.Time() - _trigger_time.at(vec.first);
			if (nt != 0) {
				times.push_back(move(nt));
			}
		}
	}
	return times;
}

vector<double> CollectionOfEvents::scint_time() {
	vector<double> times;
	for (const auto& vec : scint_events) {
		Detector trigDet = Detector::UNUSED;
		if (vec.first == Detector::SCINT21 || vec.first == Detector::SCINT22) {
			trigDet = Detector::NEUTRON2;
		}
		else if (vec.first == Detector::SCINT11 || vec.first == Detector::SCINT12) {
			trigDet = Detector::NEUTRON1;
		}
		for (const auto& item : vec.second) {
			times.push_back(item.Time() - _trigger_time.at(trigDet));
		}
	}
	return times;
}

map<Detector, vector<Event>> CollectionOfEvents::Neutr_events() const{
	return neutr_events;
}

map<Detector, vector<Event>> CollectionOfEvents::Scint_events() const{
	return scint_events;
}

size_t CollectionOfEvents::Shot() {
	return _shot;
}

map <Detector, size_t> CollectionOfEvents::Neutrons() {
	return _num_neutron;
}

size_t CollectionOfEvents::Scint() {
	return _num_scint;
}

map<Detector, double> CollectionOfEvents::Trig() {
	return _trigger_time;
}