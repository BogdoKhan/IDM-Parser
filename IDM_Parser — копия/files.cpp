#include "files.h"

using namespace std;
namespace fs = std::filesystem;

fs::path AskForPath() {
	string mypath = "";
	cout << "Введите путь:\n(нажмите Enter чтобы остаться в текущей папке):" << endl;
	getline(cin, mypath);
	if (mypath == "" || mypath == "/" || mypath == "\\") {
		return fs::current_path();
	}
	return static_cast<fs::path>(mypath);
}

bool PathExists(const fs::path& fpath) {
	if (fs::exists(fpath)) {
		cout << "Путь " << fs::absolute(fpath) << " существует\n";
	}
	else {
		cout << "Путь " << fs::absolute(fpath) << " не существует\n";
	}
	return fs::exists(fpath);
}

void PrintFiles(const fs::path& fpath) {
	if (PathExists(fpath)) {
		cout << "В текущей папке находятся:" << endl;
		cout << "------------------------------------------" << "\n";
		if (!fpath.has_extension()) {
			for (auto& f : fs::directory_iterator(fpath)) {
				cout << f.path().filename() << "\n";
			}
		}
		else {
			cout << "Это файл!\n";
		}
		cout << "------------------------------------------" << endl;
	}
}

map<size_t, vector<Event>> OpenFile(const std::filesystem::path& fpath) {
	map<size_t, vector<Event>> events;

	ifstream is(fpath);
	if (!is) {
		cout << "Ошибка чтения файла, выход\n";
	}
	else {
		is.ignore(256, '\n');
		if (is.peek() == EOF) {
			cout << "Достигнут конец файла\n";
		}
		while (is.peek() != EOF) {
			Event event;
			//Puts events in collection of events;
			is >> event;
			events[event.Shot()].push_back(event);
		}
	}
	return events;
}

Histo::Histo() {
	_start = 0;
	_step = 0.01;
}

Histo::Histo(const vector<double>& data, const double& start, const double& step, const size_t& nbins) {
	_start = start;
	_step = step;
	NBINS = nbins;
	x.resize(NBINS, 0);
	y.resize(NBINS, 0);
	for (size_t i = 0; i < NBINS; ++i) {
		x[i] = start + step * i;
	}
	int k = 0;
	for (const auto& item : data) {
		size_t i = static_cast<size_t>(floor(abs((item - start)/step)));
		y[i]++;
		k++;
	}
}

const vector<double> Histo::showX() const{
	return x;
}

const vector<int> Histo::showY() const{
	return y;
}

const size_t Histo::nbins() const {
	return NBINS;
}