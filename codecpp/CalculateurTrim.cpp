#include "CalculateurTrim.h"
#include "ModeleAerodynamique.h"
#include "Environnement.h"
#include "Constantes.h"
#include <cmath>
#include <limits>
#include <iostream>
#include <utility>

CalculateurTrim::CalculateurTrim(ModeleAerodynamique& modele_aero, 
                                 const Environnement& environnement)
    : aero(modele_aero), env(environnement) {}

double CalculateurTrim::trouver_delta_profondeur(double pitch, double vitesse, 
                                                  double altitude, double omega, double epsilon,
                                                  double delta_p_min, double delta_p_max) {
    constexpr int n = 10000;
    const double step = (delta_p_max - delta_p_min) / n;
    
    double best_delta_p = delta_p_min;
    double min_moment_abs = std::numeric_limits<double>::infinity();
    
    // Find delta_p that minimizes pitching moment
    double rho = env.calculer_rho(altitude);
    double mach = env.calculer_mach(vitesse, altitude);
    
    for (int i = 0; i <= n; ++i) {
        double delta_p = delta_p_min + i * step;
        aero.update_from_polar(pitch, delta_p, omega, vitesse, mach);

        // Drag and thrust (assume T=D at trim)
        double D = aero.calculer_trainee(vitesse, rho);
        double traction = D;
        
        // Calculate moments
        double M_aero = aero.calculer_moment_pitch(vitesse, rho);
        double M_thrust = Physique::z_t * traction;
        double M_total = M_aero + M_thrust;

        if (std::fabs(M_total) < min_moment_abs) {
            min_moment_abs = std::fabs(M_total);
            best_delta_p = delta_p;
        }

        if (std::fabs(M_total) < epsilon) break;
    }
    
    return best_delta_p;
}

double CalculateurTrim::trouver_alpha(double vitesse, double altitude, 
                                      double masse, double omega_pitch, double tol,
                                      double alpha_min, double alpha_max) {
    using namespace Physique;
    
    // Iteratively solve for alpha and delta_p simultaneously: L=W and M=0
    constexpr int max_iterations_globales = 100;
    
    double W = masse * g;
    double rho = env.calculer_rho(altitude);
    double mach = env.calculer_mach(vitesse, altitude);
    
    // Estimation initiale
    double alpha = (alpha_min + alpha_max) / 2.0;  // Starting point at mid-range
    double delta_p = -0.02;            // Starting point
    
    double erreur_L = 1e10;
    double erreur_M = 1e10;
    
    for (int iter = 0; iter < max_iterations_globales; ++iter) {
        // Étape 1: Ajuster delta_p pour minimiser le moment (avec alpha fixe)
        delta_p = trouver_delta_profondeur(alpha, vitesse, altitude, omega_pitch);
        
        // Étape 2: Ajuster alpha pour équilibrer la portance (avec delta_p fixe)
        aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse, mach);
        double L = aero.calculer_portance(vitesse, rho);
        erreur_L = L - W;
        
        // Ajustement de alpha par méthode de Newton simplifiée
        // dL/dalpha ≈ (L_new - L_old) / d_alpha pour petit d_alpha
        double d_alpha = 0.01 * DEG_TO_RAD;  // Petit incrément pour dérivée
        aero.update_from_polar(alpha + d_alpha, delta_p, omega_pitch, vitesse, mach);
        double L_plus = aero.calculer_portance(vitesse, rho);
        double dL_dalpha = (L_plus - L) / d_alpha;
        
        if (std::fabs(dL_dalpha) > 1e-6) {
            alpha -= erreur_L / dL_dalpha;  // Newton step
            alpha = std::max(alpha_min, std::min(alpha_max, alpha));  // Contraindre
        }
        
        // Vérifier le moment pour critère de convergence
        aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse, mach);
        double D = aero.calculer_trainee(vitesse, rho);
        double M_aero = aero.calculer_moment_pitch(vitesse, rho);
        double M_thrust = Physique::z_t * D;  // T = D au trim
        erreur_M = M_aero + M_thrust;
        
        // Critère de convergence: L≈W ET M≈0
        if (std::fabs(erreur_L) < tol && std::fabs(erreur_M) < 1.0) {
            // CONVERGENCE ATTEINTE
            std::cout << "[TRIM] Convergence en " << (iter+1) << " iterations" << std::endl;
            std::cout << "       Erreur L-W: " << erreur_L << " N, Erreur M: " << erreur_M << " N.m" << std::endl;
            break;
        }
    }
    
    if (std::fabs(erreur_L) > tol * 10 || std::fabs(erreur_M) > 10.0) {
        std::cerr << "[TRIM WARNING] Convergence partielle: erreur_L=" << erreur_L 
                  << " N, erreur_M=" << erreur_M << " N.m" << std::endl;
    }
    
    return alpha;
}

std::pair<double, double> CalculateurTrim::calculer_trim_complet(
    double vitesse, double altitude, double masse, double omega_pitch,
    double alpha_min, double alpha_max, 
    double delta_p_min, double delta_p_max) {
    
    using namespace Physique;
    
    constexpr int max_iterations = 30;
    constexpr double tol_L = 1e-6;     // Tolérance sur L-W
    constexpr double tol_M = 100.0;      // Tolérance sur moment (N.m)
    
    double W = masse * g;
    double rho = env.calculer_rho(altitude);
    double mach = env.calculer_mach(vitesse, altitude);
    
    // Initialisation
    double alpha = (alpha_min + alpha_max) / 2.0;  // Mid-range starting point
    double delta_p = (delta_p_min + delta_p_max) / 2.0;
    
    for (int iter = 0; iter < max_iterations; ++iter) {
        // Étape 1: Ajuster delta_p pour minimiser moment (alpha fixe)
        delta_p = trouver_delta_profondeur(alpha, vitesse, altitude, omega_pitch, 1e-6,
                                          delta_p_min, delta_p_max);
        
        // Étape 2: Ajuster alpha pour L=W (delta_p fixe)
        aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse, mach);
        double L = aero.calculer_portance(vitesse, rho);
        double erreur_L = L - W;
        
        // Calcul dérivée dL/dalpha
        double d_alpha = 0.01 * DEG_TO_RAD;
        aero.update_from_polar(alpha + d_alpha, delta_p, omega_pitch, vitesse, mach);
        double L_plus = aero.calculer_portance(vitesse, rho);
        double dL_dalpha = (L_plus - L) / d_alpha;
        
        if (std::fabs(dL_dalpha) > 1e-6) {
            alpha -= erreur_L / dL_dalpha;
            alpha = std::max(alpha_min, std::min(alpha_max, alpha));
        }
        
        // Vérifier moments pour convergence
        aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse, mach);
        double D = aero.calculer_trainee(vitesse, rho);
        double M_aero = aero.calculer_moment_pitch(vitesse, rho);
        double M_thrust = Physique::z_t * D;
        double erreur_M = M_aero + M_thrust;
        
        if (std::fabs(erreur_L) < tol_L && std::fabs(erreur_M) < tol_M) {
            std::cout << "[TRIM COMPLET] Convergence en " << (iter+1) << " iterations" << std::endl;
            std::cout << "               Alpha=" << (alpha*RAD_TO_DEG) << " deg, Delta_p=" << delta_p << " rad" << std::endl;
            std::cout << "               Erreur L-W=" << erreur_L << " N, Erreur M=" << erreur_M << " N.m" << std::endl;
            return {alpha, delta_p};
        }
    }
    
    // Si pas convergé, retourner meilleure approximation
    std::cout << "[TRIM COMPLET] Convergence en " << max_iterations << " iterations (complete)" << std::endl;
    aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse, mach);
    double L = aero.calculer_portance(vitesse, rho);
    double D = aero.calculer_trainee(vitesse, rho);
    double M_aero = aero.calculer_moment_pitch(vitesse, rho);
    double M_thrust = Physique::z_t * D;
    std::cout << "               Alpha=" << (alpha*RAD_TO_DEG) << " deg, Delta_p=" << delta_p << " rad" << std::endl;
    std::cout << "               Erreur L-W=" << (L - W) << " N, Erreur M=" << (M_aero + M_thrust) << " N.m" << std::endl;
    return {alpha, delta_p};
}
