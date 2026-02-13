#ifndef SIMULATEUR_H
#define SIMULATEUR_H

#include <string>

class Avion;

class Simulateur {
private:
    Avion& avion;
    double dt;
    double temps_total;
    std::string fichier_sortie;
    
public:
    Simulateur(Avion& av, double pas_temps = 0.01, double duree = 2000.0,
               const std::string& fichier = "simulation_full.csv");
    
    void executer();
};

#endif // SIMULATEUR_H
