#include "AnalyseurEnveloppeVol.h"
#include "Avion.h"
#include "Constantes.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

AnalyseurEnveloppeVol::AnalyseurEnveloppeVol(Avion& av) : avion(av) {}

void AnalyseurEnveloppeVol::analyser_limites_vitesse() {
    using namespace Math;
    
    // Structural limit: Mach = 0.82 (fixed by aircraft design constraints)
    const double MACH_LIMIT_STRUCTURAL = 0.82;
    
    cout << "Altitude (m) | V_min (m/s) | V_max (m/s) | Mach_min | Mach_max" << endl;
    
    ofstream csv("vmax_analysis.csv");
    csv << "altitude,v_min,v_max,Mach_min,Mach_max\n";
    
    vector<double> altitudes = {5000, 6000, 7000, 8000, 9000, 10000, 
                                 11000, 12000, 13000, 14000, 15000};
    
    for (double alt : altitudes) {
        avion.z_ref() = alt;
        double rho = avion.get_env().calculer_rho(alt);
        double sound_speed = avion.get_env().calculer_vitesse_son(alt);
        
        double S = avion.get_aero().get_surface();
        double W = avion.get_masse() * config.getDouble("g");
        double alpha_stall_rad = 15.0 * DEG_TO_RAD;
        double CL_max = 5.0 * (alpha_stall_rad - (-0.035)) + 0.44 * (-0.13);
        
        double v_min = std::sqrt(2.0 * W / (rho * S * CL_max));
        
        double v_max = MACH_LIMIT_STRUCTURAL * sound_speed;
        
        double Mach_min = v_min / sound_speed;
        double Mach_max = v_max / sound_speed;
        
        cout << fixed << alt << " | " << v_min << " | " << v_max 
             << " | " << Mach_min << " | " << Mach_max << endl;
             
        csv << alt << "," << v_min << "," << v_max << "," 
            << Mach_min << "," << Mach_max << endl;
    }
    
    csv.close();
    cout << "Results saved to vmax_analysis.csv" << endl;
}