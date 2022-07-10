#include "systemc.h"
#include<iostream>
#include "ACO.h"
#include<thread>
using namespace std;

#define ITERATIONS		(int) 5

#define NUMBEROFANTS	(int) 8
#define NUMBEROFCITIES	(int) 8

// if (ALPHA == 0) { stochastic search & sub-optimal route }
#define ALPHA			(double) 0.5
// if (BETA  == 0) { sub-optimal route }
#define BETA			(double) 0.8
// Estimation of the suspected best route.
#define Q				(double) 80
// Pheromones evaporation. 
#define RO				(double) 0.2
// Maximum pheromone random number.
#define TAUMAX			(int) 2

#define INITIALCITY		(int) 0

SC_MODULE(software) {
	sc_event city_connected_ev, set_city_pos_ev, city_selector_ev, route_maker_ev, validator_ev;
	Randoms* randoms;
	int graph[NUMBEROFCITIES][NUMBEROFCITIES];
	double pheromones[NUMBEROFCITIES][NUMBEROFCITIES];
	double cities[NUMBEROFCITIES][NUMBEROFCITIES];
	double probs[NUMBEROFCITIES-1][2];
	int routes[NUMBEROFCITIES][NUMBEROFCITIES];

	void connect_cities(int cityi, int cityj) {
		wait(100, SC_NS); //	delay to demonstrate software being slower than hardware
		graph[cityi][cityj] = 1;
		pheromones[cityi][cityj] = randoms->Uniforme() * TAUMAX;
		graph[cityj][cityi] = 1;
		pheromones[cityj][cityi] = pheromones[cityi][cityj];
		city_connected_ev.notify();
		wait(3, SC_SEC);
	}

	void set_city_pos(int city, double x, double y) {
		wait(100, SC_NS); //	delay to demonstrate software being slower than hardware
		cities[city][0] = x;
		cities[city][1] = y;
		set_city_pos_ev.notify();
		wait(3, SC_SEC);
	}

	int city_selector() {
		wait(100, SC_NS); //	delay to demonstrate software being slower than hardware
		double xi = randoms->Uniforme();
		int i = 0;
		double sum = probs[i][0];
		while (sum < xi) {
			i++;
			sum += probs[i][0];
		}
		city_selector_ev.notify();
		return (int)probs[i][1];
	}

	void route_maker(int antk) {
		wait(100, SC_NS); //	delay to demonstrate software being slower than hardware
		routes[antk][0] = INITIALCITY;
		for (int i = 0; i < NUMBEROFCITIES - 1; i++) {
			int cityi = routes[antk][i];
			routes[antk][i + 1] = city_selector();
		}
		route_maker_ev.notify();
		wait(3, SC_SEC);
	}

	bool validator(int antk, int iteration) {
		wait(100, SC_NS); //	delay to demonstrate software being slower than hardware
		for (int i = 0; i < NUMBEROFCITIES - 1; i++) {
			int cityi = routes[antk][i];
			int cityj = routes[antk][i + 1];
			if (cityi < 0 || cityj < 0) {
				return false;
			}
			// city i, j not connected
			for (int j = 0; j < i - 1; j++) {
				if (routes[antk][i] == routes[antk][j]) {
					return false;
				}
			}
		}
		validator_ev.notify();
		wait(3, SC_SEC);
		return true;
	}



	SC_CTOR(software) {
		sensitive << city_connected_ev, set_city_pos_ev, city_selector_ev, route_maker_ev, validator_ev;
	}

};

static software sw("sw");
// Connect cities
// random pheromone at each edge
// connect edge i->j, j->i = 1
// software
void ACO::connectCITIES(int cityi, int cityj) {
	sw.graph[cityi][cityj] = 1;
	sw.pheromones[cityi][cityj] = randoms->Uniforme() * TAUMAX;
	sw.graph[cityj][cityi] = 1;
	sw.pheromones[cityj][cityi] = PHEROMONES[cityi][cityj];
	GRAPH[cityi][cityj] = 1;
	PHEROMONES[cityi][cityj] = randoms->Uniforme() * TAUMAX;
	GRAPH[cityj][cityi] = 1;
	PHEROMONES[cityj][cityi] = PHEROMONES[cityi][cityj];
}


// Set city position x, y
void ACO::setCITYPOSITION(int city, double x, double y) {
	sw.cities[city][0] = x;
	sw.cities[city][1] = y;
	CITIES[city][0] = x;
	CITIES[city][1] = y;
}
// ------------------------------------------------
// check if edge between 2 cities connected
// if connected return true
bool ACO::exists(int cityi, int cityc) {
	return (GRAPH[cityi][cityc] == 1);
}
// ------------------------------------------------
// *NOT SURE* check ant visited city 
bool ACO::vizited(int antk, int c) {
	for (int l = 0; l < NUMBEROFCITIES; l++) {
		if (ROUTES[antk][l] == -1) {
			sw.routes[antk][l] = -1;
			break;
		}
		if (ROUTES[antk][l] == c) {
			sw.routes[antk][l] = c;
			return true;
		}
	}
	return false;
}
// print pheromones
void ACO::printPHEROMONES() {
	this_thread::sleep_for(chrono::milliseconds(500));
	time += 500;
	cout << " @ " << time << " ms " << "pheromones updated..." << endl;
	cout << " PHEROMONES: " << endl;
	cout << "  | ";
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		printf("%5d   ", i);
	}
	cout << endl << "- | ";
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		cout << "--------";
	}
	cout << endl;
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		cout << i << " | ";
		for (int j = 0; j < NUMBEROFCITIES; j++) {
			if (i == j) {
				printf("%5s   ", "x");
				continue;
			}
			if (exists(i, j)) {
				printf("%7.3f ", PHEROMONES[i][j]);
			}
			else {
				if (PHEROMONES[i][j] == 0.0) {
					printf("%5.0f   ", PHEROMONES[i][j]);
				}
				else {
					printf("%7.3f ", PHEROMONES[i][j]);
				}
			}
		}
		cout << endl;
	}
	cout << endl;
}

// selecte city if random value less than prbability
int ACO::city() {
	double xi = randoms->Uniforme();
	int i = 0;
	double sum = PROBS[i][0];
	while (sum < xi) {
		// if (i == NUMBEROFCITIES)
		//	xi = randoms->Uniforme() / 100;
		i++;
		sum += PROBS[i][0];
	}


	return (int)PROBS[i][1];
}


// *NOT SURE* validation 
int ACO::valid(int antk, int iteration) {
	for (int i = 0; i < NUMBEROFCITIES - 1; i++) {
		int cityi = ROUTES[antk][i];
		int cityj = ROUTES[antk][i + 1];
		if (cityi < 0 || cityj < 0) {
			return -1;
		}
		// city i, j not connected
		if (!exists(cityi, cityj)) {
			return -2;
		}
		for (int j = 0; j < i - 1; j++) {
			if (ROUTES[antk][i] == ROUTES[antk][j]) {
				return -3;
			}
		}
	}

	if (!exists(INITIALCITY, ROUTES[antk][NUMBEROFCITIES - 1])) {
		return -4;
	}

	return 0;
}

// print gragh of cities
void ACO::printGRAPH() {
	cout << " GRAPH: " << endl;
	cout << "  | ";
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		cout << i << " ";
	}
	cout << endl << "- | ";
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		cout << "- ";
	}
	cout << endl;
	int count = 0;
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		cout << i << " | ";
		for (int j = 0; j < NUMBEROFCITIES; j++) {
			if (i == j) {
				cout << "x ";
			}
			else {
				cout << GRAPH[i][j] << " ";
			}
			if (GRAPH[i][j] == 1) {
				count++;
			}
		}
		cout << endl;
	}
	cout << endl;
	cout << "Number of connections: " << count << endl << endl;
}
// ------------------------------------------------
// print results
// calculate distance between current city and selected city
// sum of the distance between each 2 cities equal to best length
void ACO::printRESULTS() {
	BESTLENGTH += distance(BESTROUTE[NUMBEROFCITIES - 1], INITIALCITY);
	this_thread::sleep_for(chrono::milliseconds(50));
	time += 50;
	cout << "@ " << time <<" BEST ROUTE:" << endl;
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		cout << BESTROUTE[i] << " ";
	}
	cout << endl << "length: " << BESTLENGTH << endl;
}
