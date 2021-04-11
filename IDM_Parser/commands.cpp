#include "commands.h"
#include "files.h"

using namespace std;
namespace fs = std::filesystem;

istream& operator >> (istream& is, Query& q) {
	string s = "";
	cout << "Введите команду: " << endl;
	getline(cin, s);
	if (s == "stop" || s == "STOP" || s == ".q") {
		q.command = Command::STOP;
	}
	else if (s == "ls" || s == "list" || s == "LIST") {
		q.command = Command::LIST;
	}
	else if (s == "cd" || s == "CD" || s == "CHDIR") {
		q.command = Command::CHDIR;
	}
	else if (s == "mkdir" || s == "MKDIR") {
		q.command = Command::MKDIR;
	}
	else if (s == "help" || s == "HELP") {
		q.command = Command::HELP;
	}
	else if (s == "an" || s == "analyze" || s == "ANALYZE") {
		q.command = Command::ANALYZE;
	}
	else {
		q.command = Command::ERR;
	}
	return is;
}

void ListDir() {
	fs::path mypath = AskForPath();
	PrintFiles(mypath);
	cout << "\n";
}

void ChangeDir() {
	fs::path mypath = AskForPath();
	if (PathExists(mypath) && !mypath.has_extension()) {
		fs::current_path(mypath);
		cout << "Рабочая папка изменена\n";
	}
	cout << "---------------------------------------------------\n";
}

void MakeDir() {
	fs::path mypath = AskForPath();
	if (!PathExists(mypath)) {
		fs::create_directory(mypath);
		cout << "Создана новая папка в " << fs::absolute(mypath) << "\n";
	}
}

void ShowHelp() {
	cout << "Список команд:" << "\n";
	cout << "stop, STOP, .q - stops the execution of program\n";
	cout << "list, LIST - shows the list of files in specified directory\n";
	cout << "cd, CD, CHDIR - changes the working directory\n";
	cout << "mkdir, MKDIR - creates a new directory\n";
	cout << "help, HELP - shows the help\n";
	cout << "an, ANALYZE - analyze the file\n";
	cout << "---------------------------------------------------\n";
}

bool FileNameMask(const fs::path& fpath) {
	string s = fpath.stem().string();
	string mask = "DE";
	if (s.size() < 5) {
		return false;
	}
	else {
		return equal(s.end() - 2, s.end(), mask.begin());
	}
}

void OpenSession() {
	fs::path mypath = AskForPath();
	if (PathExists(mypath) && mypath.has_extension()) {
		if (mypath.extension() == ".txt" && FileNameMask(mypath)) {
			if (!fs::is_directory(mypath.stem())) {
				fs::create_directory(mypath.stem());
			}

			ofstream fileTotal(mypath.stem().string()+"\\total.txt", ios::trunc);
			ofstream hTimeN(mypath.stem().string() + "\\hTimeN.txt", ios::trunc);
			ofstream hTimeS(mypath.stem().string() + "\\hTimeS.txt", ios::trunc);
			ofstream fileMultN(mypath.stem().string() + "\\fileMultiNeutron.txt", ios::trunc);
			ofstream hSpectrN(mypath.stem().string() + "\\hSpectrN.txt", ios::trunc);

			cout << "---------------------------------------------------\n";
			cout << "Начат анализ файла.\n";

			map<size_t, vector<Event>> events = OpenFile(mypath);
			vector<double> TimesForNeutrons;
			vector<double> TimesForScint;
			vector<double> SpectrumNeutrons;
			map<int, size_t> Cluster;
			size_t nNeutrons25 = 0;
			size_t nNeutrons = 0;
			size_t nScint = 0;
			//Fill collection of times for neutron and scint detectors, find their number
			for (const auto& event_ : events) {
				CollectionOfEvents col(event_);

				if (col.Neutrons() > 0) {
					vector<double> col_nt = col.neutr_time();
					TimesForNeutrons.insert(TimesForNeutrons.end(), col_nt.begin(), col_nt.end());
					nNeutrons += col.Neutrons();
					Cluster[col.Neutrons()]++;

					vector<Event> col_neut = col.Neutr_events();
					for (const auto& item : col_neut) {
						SpectrumNeutrons.push_back(item.Ampl());
						if (item.Ampl() > 2.0) {
							nNeutrons25++;
						}
					}

					if (col.Neutrons() > 1) {
						vector<Event> colNeut = move(col_neut);
						vector<Event> colScint = col.Scint_events();

						fileMultN << "Выстрел " << col.Shot() << " с " << col.Neutrons() << " событиями с нейтронного "
							<< "и " << col.Scint() << " со сцинтилляционного детектора\n";
						fileMultN << "---------------------------------------------------\nСобытия на нейтронном детекторе: \n";
						fileMultN << setw(10) << "Ампл., V" << " " << setw(15) << "Время, мкс" << '\n';
						for (const auto& item : colNeut) {
							fileMultN << setw(10) << item.Ampl() << " " << setw(15) << (item.Time() - col.Trig()) << '\n';
						}
						fileMultN << "---------------------------------------------------\n";
						if (col.Scint() > 0) {
							fileMultN << "События на сцинтилляционном: \n";
							fileMultN << setw(10) << "Ампл., V" << " " << setw(15) << "Время, мкс" << '\n';
							for (const auto& item : colScint) {
								fileMultN << setw(10) << item.Ampl() << " " << setw(15) << (item.Time() - col.Trig()) << '\n';
							}
							fileMultN << "---------------------------------------------------\n";
						}

					}
				}

				if (col.Scint() > 0) {
					vector<double> col_st = col.scint_time();
					TimesForScint.insert(TimesForScint.end(), col_st.begin(), col_st.end());
					nScint += col.Scint();
				}
			}
			//Fill total information
			if (events.size() > 0) {
				fileTotal << "Запусков: " << events.rbegin()->first << "; ";
			}
			fileTotal << "Нейтронов: " << nNeutrons << ", "
				<< "с амплитудой выше 2 V: " << nNeutrons25 << '\n';
			if (Cluster.size() > 0) {
				fileTotal << "Событий по числу нейтронов:\n";
				for (const auto& cl : Cluster) {
					fileTotal << cl.first << " ";
				}
				fileTotal << endl;
				for (const auto& cl : Cluster) {
					fileTotal << cl.second << " ";
				}
				fileTotal << endl;
			}
			fileTotal << "Событий на сцинтилляционном детекторе: " << nScint << '\n';
			fileTotal << "Всего событий: " << nNeutrons + nScint << '\n';

			//Fill histogram with times from neutron detector
			//fileTotal << "Non-zero times for neutron detector: " << '\n';

			Histo hTimesNeutron(TimesForNeutrons, -200.0, 0.1, 6000);
			vector<double> hXNeutron = hTimesNeutron.showX();
			vector<int> hYNeutron = hTimesNeutron.showY();
			for (size_t i = 0; i < hTimesNeutron.nbins(); i++) {
				hTimeN << setw(10) << hXNeutron.at(i) << " " << setw(10) << hYNeutron.at(i) << '\n';
			}
			//fileTotal << "\n---------------------------------------------------\n";

			//Fill histogram with times from scint detector
			//fileTotal << "Times for scint detector: " << '\n';
			Histo hTimesScint(TimesForScint, -200.0, 5, 120);
			vector<double> hXScint = hTimesScint.showX();
			vector<int> hYScint = hTimesScint.showY();
			for (size_t i = 0; i < hTimesScint.nbins(); i++) {
				hTimeS << setw(10) << hXScint.at(i) << " " << setw(10) << hYScint.at(i) << '\n';
			}
			//cout << "\n---------------------------------------------------\n";
			Histo hSpectrumN(SpectrumNeutrons, 0., 0.01, 500);
			vector<double> hXSN = hSpectrumN.showX();
			vector<int> hYSN = hSpectrumN.showY();
			for (size_t i = 0; i < hSpectrumN.nbins(); i++) {
				hSpectrN << setw(10) << hXSN.at(i) << " " << setw(10) << hYSN.at(i) << '\n';
			}

			fileTotal.close();
			hTimeN.close();
			hTimeS.close();
			fileMultN.close();
			hSpectrN.close();
			cout << "Анализ файла завершен!\n";
			cout << "---------------------------------------------------\n";
		}
	}
}