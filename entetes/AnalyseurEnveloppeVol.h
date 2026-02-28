#ifndef ANALYSEUR_ENVELOPPE_VOL_H
#define ANALYSEUR_ENVELOPPE_VOL_H

class Avion;

class AnalyseurEnveloppeVol {
private:
    Avion& avion;
    
    // Check longitudinal dynamic stability using simplified eigenvalue analysis
    bool check_stability(double V, double alpha, double mach, 
                        double rho, double delta_p);
    
public:
    explicit AnalyseurEnveloppeVol(Avion& av);
    
    void analyser_limites_vitesse();
};

#endif // ANALYSEUR_ENVELOPPE_VOL_H
