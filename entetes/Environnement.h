#ifndef ENVIRONNEMENT_H
#define ENVIRONNEMENT_H

class Environnement {
private:
    const double rho_0 = 1.225;   // Densité de l'air au niveau de la mer (kg/m³)
    const double T_0 = 288.15;    // Température au niveau de la mer (K) = 15°C  (Convention agence internationale aviation)
    const double L = 0.0065;      // Gradient thermique (K/m)
    const double R = 287.05;      // Constante des gaz pour l'air (J/(kg·K))
    const double T_min = 216.65;  // Température minimum (stratosphère)
    
public:
    double calculer_rho(double altitude) const;
    double calculer_vitesse_son(double altitude) const;
};

#endif // ENVIRONNEMENT_H
