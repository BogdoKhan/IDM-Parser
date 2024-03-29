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
		event._detector = Detector::NEUTRON1;
	}
	else if (detector == "4") {
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
	else if (detector == "8") {
		event._detector = Detector::NEUTRON2;
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
	lower_bounds = 0;
	upper_bounds = 10.0;
}

void CollectionOfEvents::PutEventsInDetector(bool& trigger, const Detector& det, const Event& event_) {
	if (det == Detector::NEUTRON1 || det == Detector::NEUTRON2) {
		//set trigger time if not set;
		if (!trigger) {
			_trigger_time[det] = event_.Time();
			trigger = true;
		}

		//put all events to vector
		neutr_events[det].push_back(event_);
		_num_neutron[event_.dType()]++;

		//put correct events within processing range
		if (event_.Ampl() > lower_bounds && event_.Ampl() < upper_bounds) {
			neutr_events_wRange[det].push_back(event_);
			_num_neutron_wRange[event_.dType()]++;
		}

		//put events with exceeding amplitude
		else if (event_.Ampl() >= upper_bounds) {
			neutr_events_OOR[det].push_back(event_);
			_num_neutron_OOR[event_.dType()]++;
		}

		//put "corrupted" events below threshold
		else {
			neutr_events_CRPT[det].push_back(event_);
			_num_neutron_CRPT[event_.dType()]++;
		}
	}
	else if (
		det == Detector::SCINT11 || det == Detector::SCINT12 ||
		det == Detector::SCINT21 || det == Detector::SCINT22
		) {
			//just scintillator events
			scint_events[det].push_back(event_);
			_num_scint++;
	}
	else {
		cout << "Detector role not defined, abort addition of event\n";
	}
}

CollectionOfEvents::CollectionOfEvents(const std::pair<size_t, std::vector<Event>>& col, 
	const double& lowerbnd, const double& upperbnd) {
	_shot = col.first;
	lower_bounds = lowerbnd;
	upper_bounds = upperbnd;

	bool trigFound2 = false;
	bool trigFound1 = false;
	bool trigScint = true;

	for (const auto& event_ : col.second) {
		if (event_.dType() == Detector::NEUTRON2) {
			PutEventsInDetector(trigFound2, Detector::NEUTRON2, event_);
		}
		else if (event_.dType() == Detector::NEUTRON1) {
			PutEventsInDetector(trigFound1, Detector::NEUTRON1, event_);
		}
		else if (event_.dType() == Detector::SCINT21) {
			PutEventsInDetector(trigScint, Detector::SCINT21, event_);
		}
		else if (event_.dType() == Detector::SCINT22) {
			PutEventsInDetector(trigScint, Detector::SCINT22, event_);
		}
		else if (event_.dType() == Detector::SCINT11) {
			PutEventsInDetector(trigScint, Detector::SCINT11, event_);
		}
		else if (event_.dType() == Detector::SCINT12) {
			PutEventsInDetector(trigScint, Detector::SCINT12, event_);
		}
	}
	if (_trigger_time.at(Detector::NEUTRON1) > 0 &&
		_trigger_time.at(Detector::NEUTRON2) > 0) {
		_trigger_time_start = min(_trigger_time.at(Detector::NEUTRON1), _trigger_time.at(Detector::NEUTRON2));
	}
	else {
		_trigger_time_start = max(_trigger_time.at(Detector::NEUTRON1), _trigger_time.at(Detector::NEUTRON2));
	}
	

}

map<string, vector<double>> CollectionOfEvents::neutr_time() {
	map<string, vector<double>> times = {
					{"TOTAL", {}}, {"RANGE", {}}, {"OOR", {}}, {"CRPT", {}}
	};
	for (const auto& vec : neutr_events) {
		for (const auto& item : vec.second) {
			double nt = item.Time() - _trigger_time_start;
			if (nt != 0) {
				times.at("TOTAL").push_back(move(nt));
			}
		}
	}
	for (const auto& vec : neutr_events_wRange) {
		for (const auto& item : vec.second) {
			double nt = item.Time() - _trigger_time_start;
			if (nt != 0) {
				times.at("RANGE").push_back(move(nt));
			}
		}
	}
	for (const auto& vec : neutr_events_OOR) {
		for (const auto& item : vec.second) {
			double nt = item.Time() - _trigger_time_start;
			if (nt != 0) {
				times.at("OOR").push_back(move(nt));
			}
		}
	}
	for (const auto& vec : neutr_events_CRPT) {
		for (const auto& item : vec.second) {
			double nt = item.Time() - _trigger_time_start;
			if (nt != 0) {
				times.at("CRPT").push_back(move(nt));
			}
		}
	}
	return times;
}

vector<double> CollectionOfEvents::scint_time() {
	vector<double> times;
	for (const auto& vec : scint_events) {
		if (_trigger_time_start != 0) {
			for (const auto& item : vec.second) {
				times.push_back(item.Time() - _trigger_time_start);
			}
		}
	}
	return times;
}

map<string, map<Detector, vector<Event>>> CollectionOfEvents::Neutr_events() const{
	map<string, map<Detector, vector<Event>>> NE_map = {
					{"TOTAL", neutr_events}, {"RANGE", neutr_events_wRange}, {"OOR", neutr_events_OOR}, {"CRPT", neutr_events_CRPT}
	};
	return NE_map;
}

map<Detector, vector<Event>> CollectionOfEvents::Scint_events() const{
	return scint_events;
}

size_t CollectionOfEvents::Shot() {
	return _shot;
}

map<string, map<Detector, size_t>> CollectionOfEvents::Neutrons() {
	map<string, map<Detector, size_t>> NE_map = {
		{"TOTAL", _num_neutron},
		{"RANGE", _num_neutron_wRange},
		{"OOR", _num_neutron_OOR},
		{"CRPT", _num_neutron_CRPT}
	};
	return NE_map;
}

size_t CollectionOfEvents::Scint() {
	return _num_scint;
}

double CollectionOfEvents::Trig() {
	return _trigger_time_start;
}