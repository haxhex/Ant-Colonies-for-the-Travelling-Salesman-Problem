#include "systemc.h"
#include "ACO.h"
#include<iostream>
using namespace std;

#include <iostream>
#include <cstdlib>

#include <cmath>
#include <limits>
#include <climits>

#define ITERATIONS		(int) 5

#define NUMBEROFANTS	(int) 8
#define NUMBEROFCITIES	(int) 8

#define ALPHA			(double) 0.5
#define BETA			(double) 0.8
#define Q				(double) 80
// Pheromones evaporation. 
#define RO				(double) 0.2
// Maximum pheromone random number.
#define TAUMAX			(int) 2

#define INITIALCITY		(int) 0


int sc_main(int argc, char* argv[]) {

	    ACO *ANTS = new ACO (NUMBEROFANTS, NUMBEROFCITIES, 
    			 			ALPHA, BETA, Q, RO, TAUMAX,
    			 			INITIALCITY);
        bool error = true;
    
    	ANTS -> init();
    	// connect cities - edges
    	ANTS -> connectCITIES (0, 1);
    	ANTS -> connectCITIES (0, 2);
    	ANTS -> connectCITIES (0, 3);
    	ANTS -> connectCITIES (0, 7);
    	ANTS -> connectCITIES (1, 3);
    	ANTS -> connectCITIES (1, 5);
    	ANTS -> connectCITIES (1, 7);
    	ANTS -> connectCITIES (2, 4);
    	ANTS -> connectCITIES (2, 5);
    	ANTS -> connectCITIES (2, 6);
    	ANTS -> connectCITIES (4, 3);
    	ANTS -> connectCITIES (4, 5);
    	ANTS -> connectCITIES (4, 7);
    	ANTS -> connectCITIES (6, 7);
    
    	// set city positions
    	ANTS -> setCITYPOSITION (0,  1,  1);
    	ANTS -> setCITYPOSITION (1, 10, 10);
    	ANTS -> setCITYPOSITION (2, 20, 10);
    	ANTS -> setCITYPOSITION (3, 10, 30);
    	ANTS -> setCITYPOSITION (4, 15,  5);
    	ANTS -> setCITYPOSITION (5, 10,  1);
    	ANTS -> setCITYPOSITION (6, 20, 20);
    	ANTS -> setCITYPOSITION (7, 20, 30);
    
    	ANTS -> printGRAPH ();
    
    	ANTS -> printPHEROMONES ();
    
    	ANTS -> optimize (ITERATIONS);
    
    	ANTS -> printRESULTS ();

        if (!error)
            sc_start();

	return 0;
}
