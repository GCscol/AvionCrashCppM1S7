#ifndef ANALYSEUR_ENVELOPPE_VOL_H
#define ANALYSEUR_ENVELOPPE_VOL_H

class Avion;

class AnalyseurEnveloppeVol {
private:
    Avion& avion;
    
public:
    explicit AnalyseurEnveloppeVol(Avion& av);
    
    void analyser_limites_vitesse();
};

#endif // ANALYSEUR_ENVELOPPE_VOL_H
