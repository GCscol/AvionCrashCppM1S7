#ifndef PROPRIETES_INERTIE_H
#define PROPRIETES_INERTIE_H

class ProprietesInertie {
private:
    double masse;
    double I_y;  // Moment d'inertie en tangage
    
public:
    ProprietesInertie(double m, double inertie_y = 9720000.0);
    
    double get_masse() const;
    double get_I_pitch() const;
    double calculer_poids() const;
    
    void set_masse(double m);
};

#endif // PROPRIETES_INERTIE_H
