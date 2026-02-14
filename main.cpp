#include "Avion.h"
#include "AnalyseurEnveloppeVol.h"
#include "Simulateur.h"


int main() {
	//#ifdef MODE_SIMULATION
    // Avion avion(361.6, 6.6, 200000);
    
    // AnalyseurEnveloppeVol analyseur(avion);
    // analyseur.analyser_limites_vitesse();
    

	// #else

     Avion avion(361.6, 6.6, 140178.9);     
    // Initialisation
     avion.initialiser();

     Simulateur sim(avion, 0.01, 2000.0, "simulation_full.csv");
     
     sim.executer();
     
   //   #endif
     return 0;
 }
