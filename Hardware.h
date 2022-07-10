#include "systemc.h"
#include "ACO.h"
#include<thread>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <climits>

#define cityNums (int) 8

using namespace std;

SC_MODULE(hardware) {
	sc_int<4> NUMBEROFCITIES = 8;
	sc_int<4> NUMBEROFANTS = 8;
	sc_int<8> Q = 80;
	sc_int<4> cityi, cityj;
	sc_event distance_ev, phi_ev, length_ev, up_pher_ev;
	double BETA = 0.5,
		ALPHA = 0.8;
	double cities[cityNums][cityNums], pheromones[cityNums][cityNums];
	double distance[cityNums];
	int routes[cityNums][cityNums];
	double deltapheromones[cityNums][cityNums];


	void calc_distance() {
		int i = 0;
		while (true) {
			double dist = sqrt(pow(cities[cityi][0] - cities[cityj][0], 2) +
				pow(cities[cityi][1] - cities[cityj][1], 2));
			distance[i] = dist;
			++i;
			distance_ev.notify();
			wait(3, SC_SEC);
		}

	}

	void calc_phi() {
	int j = 0;
	while (true)
	{
		wait(distance_ev);
		double ETAij = (double)pow(1 / distance[j], BETA);
		double TAUij = (double)pow(pheromones[cityi][cityj], ALPHA);
		double sum = 0.0;
		double ETA = (double)pow(1 / distance[j], BETA);
		double TAU = (double)pow(pheromones[cityi][cityj], ALPHA);
		sum += ETA * TAU;
		double prob = (ETAij * TAUij) / sum;
		phi_ev.notify();
		wait(3, SC_SEC);

	}
}

// calculate length of route
//// hardware
//// add ROUTES
	double calc_length(int antk) {
		double sum = 0.0;
		for (int j = 0; j < NUMBEROFCITIES - 1; j++) {
			sum += distance[j];
		}
		length_ev.notify();
		wait(3, SC_SEC);
		return sum;
	}

	void update_pheromones() {
		for (int k = 0; k < NUMBEROFANTS; k++) {
			double rlength = calc_length(k);
			for (int r = 0; r < NUMBEROFCITIES - 1; r++) {
				int cityi = routes[k][r];
				int cityj = routes[k][r + 1];
				// add phromones if edge of city i and city j occurs in path
				// positive feedback
				deltapheromones[cityi][cityj] += Q / rlength;
				deltapheromones[cityj][cityi] += Q / rlength;
			}
		}
		up_pher_ev.notify();
		wait(10, SC_SEC);
		cout << " @ " << sc_time_stamp() << " ns" << endl;
	}


	    SC_CTOR(hardware) {
			SC_THREAD(calc_distance);
			SC_THREAD(calc_phi);
			sensitive << distance_ev, phi_ev, length_ev, up_pher_ev;

	}

};

static hardware hw("hw");

// parameters
ACO::ACO(int nAnts, int nCities,
	double alpha, double beta, double q, double ro, double taumax,
	int initCity) {
	NUMBEROFANTS = nAnts;
	NUMBEROFCITIES = nCities;
	ALPHA = alpha;
	BETA = beta;
	Q = q;
	RO = ro;
	TAUMAX = taumax;
	INITIALCITY = initCity;
	// randoms = new Randoms(21);
	randoms = new Randoms(21);

}
// ------------------------------------------------
// virtual func - release memory
// soft
ACO::~ACO() {
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		delete[] GRAPH[i];
		delete[] CITIES[i];
		delete[] PHEROMONES[i];
		delete[] DELTAPHEROMONES[i];
		if (i < NUMBEROFCITIES - 1) {
			delete[] PROBS[i];
		}
	}
	delete[] GRAPH;
	delete[] CITIES;
	delete[] PHEROMONES;
	delete[] DELTAPHEROMONES;
	delete[] PROBS;
}
// ------------------------------------------------
// initialize parameters - allocate memory
void ACO::init() {
	GRAPH = new int* [NUMBEROFCITIES];
	CITIES = new double* [NUMBEROFCITIES];
	PHEROMONES = new double* [NUMBEROFCITIES];
	DELTAPHEROMONES = new double* [NUMBEROFCITIES];
	PROBS = new double* [NUMBEROFCITIES - 1];
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		GRAPH[i] = new int[NUMBEROFCITIES];
		CITIES[i] = new double[2];
		PHEROMONES[i] = new double[NUMBEROFCITIES];
		DELTAPHEROMONES[i] = new double[NUMBEROFCITIES];
		PROBS[i] = new double[2];
		for (int j = 0; j < 2; j++) {
			CITIES[i][j] = -1.0;
			PROBS[i][j] = -1.0;
		}
		for (int j = 0; j < NUMBEROFCITIES; j++) {
			GRAPH[i][j] = 0;
			PHEROMONES[i][j] = 0.0;
			DELTAPHEROMONES[i][j] = 0.0;
		}
	}

	ROUTES = new int* [NUMBEROFANTS];
	for (int i = 0; i < NUMBEROFANTS; i++) {
		ROUTES[i] = new int[NUMBEROFCITIES];
		for (int j = 0; j < NUMBEROFCITIES; j++) {
			ROUTES[i][j] = -1;
		}
	}

	BESTLENGTH = (double)INT_MAX;
	BESTROUTE = new int[NUMBEROFCITIES];
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		BESTROUTE[i] = -1;
	}
}

// ------------------------------------------------
// claculate distance between 2 cities
// d = sqrt( (x2 - x1)^2 + (y2 - y1)^2 )
double ACO::distance(int cityi, int cityj) {
	hw.cityi = cityi;
	hw.cityj = cityj;
	//hw.calc_distance();
	return (double)
		sqrt(pow(CITIES[cityi][0] - CITIES[cityj][0], 2) +
			pow(CITIES[cityi][1] - CITIES[cityj][1], 2));
}

// ------------------------------------------------
// probability function
// calculate Probabilities
// eta = 1/d
// P = (tau^alpha * eta^beta) / sigma(tau^alpha * eta^beta)
// hardware
double ACO::PHI(int cityi, int cityj, int antk) {
	hw.cityi = cityi;
	hw.cityj = cityj;
	// hw.calc_phi();
	double ETAij = (double)pow(1 / distance(cityi, cityj), BETA);
	double TAUij = (double)pow(PHEROMONES[cityi][cityj], ALPHA);

	double sum = 0.0;
	for (int c = 0; c < NUMBEROFCITIES; c++) {
		// true if city i connected to city c
		if (exists(cityi, c)) {
			// true if antk did't visit city c
			if (!vizited(antk, c)) {
				// wait
				double ETA = (double)pow(1 / distance(cityi, c), BETA);
				double TAU = (double)pow(PHEROMONES[cityi][c], ALPHA);
				sum += ETA * TAU;
				// notify phi
			}
		}
	}
	// return prob
	return (ETAij * TAUij) / sum;
}
// ------------------------------------------------
// calculate length of route
// hardware
double ACO::length(int antk) {
	//hw.calc_length(antk);
	double sum = 0.0;
	for (int j = 0; j < NUMBEROFCITIES - 1; j++) {
		sum += distance(ROUTES[antk][j], ROUTES[antk][j + 1]);
	}
	return sum;
}


// update pheromones
// hardware
void ACO::updatePHEROMONES() {
	//hw.update_pheromones();
	for (int k = 0; k < NUMBEROFANTS; k++) {
		double rlength = length(k);
		for (int r = 0; r < NUMBEROFCITIES - 1; r++) {
			int cityi = ROUTES[k][r];
			int cityj = ROUTES[k][r + 1];
			// add phromones if edge of city i and city j occurs in path
			// positive feedback
			DELTAPHEROMONES[cityi][cityj] += Q / rlength;
			DELTAPHEROMONES[cityj][cityi] += Q / rlength;
			hw.deltapheromones[cityi][cityj] += Q / rlength;
			hw.deltapheromones[cityj][cityj] += Q / rlength;

		}
	}
	for (int i = 0; i < NUMBEROFCITIES; i++) {
		for (int j = 0; j < NUMBEROFCITIES; j++) {
			hw.deltapheromones[i][j] = 0.0;
			PHEROMONES[i][j] = (1 - RO) * PHEROMONES[i][j] + DELTAPHEROMONES[i][j];
			DELTAPHEROMONES[i][j] = 0.0;
		}
	}
}

// *NOT SURE* select next city according prob
// hardware
void ACO::route(int antk) {
	hw.routes[antk][0] = INITIALCITY;
	ROUTES[antk][0] = INITIALCITY;
	for (int i = 0; i < NUMBEROFCITIES - 1; i++) {
		int cityi = ROUTES[antk][i];
		int count = 0;
		for (int c = 0; c < NUMBEROFCITIES; c++) {
			// wait
			if (cityi == c) {
				continue;
			}
			// true if city i and cit c connected
			// wait
			if (exists(cityi, c)) {
				// true if antk did't visit city c
				if (!vizited(antk, c)) {
					// wait
					PROBS[count][0] = PHI(cityi, c, antk);
					PROBS[count][1] = (double)c;
					count++;
				}

			}
		}

		// deadlock
		if (0 == count) {
			return;
		}

		hw.routes[antk][i + 1] = city();
		ROUTES[antk][i + 1] = city();
	}
}
// ------------------------------------------------
// *NOT SURE* main func
// combine all calculations 
// main
void ACO::optimize(int ITERATIONS) {
	for (int iterations = 1; iterations <= ITERATIONS; iterations++) {
		// print iteration
		cout << flush;
		cout << "@ " << time << " ITERATION " << iterations << " HAS STARTED!" << endl << endl;
		this_thread::sleep_for(chrono::milliseconds(10));
		time += 10;

		for (int k = 0; k < NUMBEROFANTS; k++) {

			cout << "@ " << time << " : ant " << k << " has been released!" << endl;
			this_thread::sleep_for(chrono::milliseconds(5));
			time += 5;
			while (0 != valid(k, iterations)) {
				this_thread::sleep_for(chrono::milliseconds(5));
				time += 5;
				cout << "@ " << time << "  :: releasing ant " << k << " again!" << endl;
				this_thread::sleep_for(chrono::milliseconds(10));
				time += 5;
				for (int i = 0; i < NUMBEROFCITIES; i++) {
					ROUTES[k][i] = -1;
				}
				// *NOT SURE* select city
				route(k);
			}
			// print route
			for (int i = 0; i < NUMBEROFCITIES; i++) {
				cout << ROUTES[k][i] << " ";
			}
			cout << endl;
			// print length of route
			this_thread::sleep_for(chrono::milliseconds(10));
			time += 5;
			cout << "@ " << time << "  :: route done" << endl;
			double rlength = length(k);

			if (rlength < BESTLENGTH) {
				BESTLENGTH = rlength;
				for (int i = 0; i < NUMBEROFCITIES; i++) {
					BESTROUTE[i] = ROUTES[k][i];
				}
			}
			cout << "@ " << time << " ms " << " : ant " << k << " has ended!" << endl;
		}
		// update pherompones
		cout << endl << "updating PHEROMONES . . .";
		updatePHEROMONES();
		cout << " done!" << endl << endl;
		// print current pheromones
		printPHEROMONES();

		for (int i = 0; i < NUMBEROFANTS; i++) {
			for (int j = 0; j < NUMBEROFCITIES; j++) {
				ROUTES[i][j] = -1;
			}
		}
		this_thread::sleep_for(chrono::milliseconds(50));
		time += 50;
		cout << endl << "@ " << time << " ms " << "ITERATION " << iterations << " HAS ENDED!" << endl << endl;
	}
}
