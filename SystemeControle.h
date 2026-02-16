#ifndef SYSTEME_CONTROLE_H
#define SYSTEME_CONTROLE_H

class SystemeControle {
private:
    double cmd_profondeur;  
    double cmd_thrust;      
    double delta_p_max;
    
public:
    SystemeControle();
    
    void set_commande_profondeur(double val);
    void set_commande_thrust(double val);
    
    double get_cmd_profondeur() const;
    double get_cmd_thrust() const;
    double get_delta_p_max() const;
    
    void reset();
};

#endif // SYSTEME_CONTROLE_H
