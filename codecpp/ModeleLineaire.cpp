#include "ModeleLineaire.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

ModeleLineaire::ModeleLineaire(double surface, double corde, double delta_prof_init)
    : ModeleAerodynamique(surface, corde, delta_prof_init) {}

void ModeleLineaire::update_from_polar(double alpha_rad, double delta_p, 
                                       double omega, double vitesse, double mach) {
    using namespace Math;
    
    // Parabolic stall model with smooth tangent transition
    // Linear region: alpha <= 13°
    // Parabolic post-stall region: alpha > 13°
    
    const double alpha0 = -0.035;  // zero-lift angle in radians
    const double alpha_tangent_deg = 13.0;  // transition point (degrees)
    const double alpha_peak_deg = 15.0;  // peak lift angle (degrees)
    const double alpha_tangent_rad = alpha_tangent_deg * DEG_TO_RAD;
    const double alpha_peak_rad = alpha_peak_deg * DEG_TO_RAD;
    const double alpha_tangent_neg_deg = -4.0;
    const double alpha_tangent_neg_rad = alpha_tangent_neg_deg * DEG_TO_RAD;
    
    // Parabola coefficient: ensures tangency at 13° and maximum at 15°
    // Corrected to eliminate discontinuity
    const double A = -71.6197;  // coefficient of quadratic term
    const double C_L_peak = 1.3967;  // C_L maximum at alpha_peak
    
    // Linear slope and elevator authority
    const double slope = 5.0;
    const double elevator_auth = 0.44;

    // Mach transition for aerodynamic effects
    const double k_M = 0.4;        // attenuation coefficient (reduced for more stability)
    const double M_c = 0.78;       // critical Mach (aligned with early phase Mach)
    const double delta_M = 0.04;   // transition width
    const double f_M = 0.5 * (1.0 + std::tanh((mach - M_c) / delta_M));
    
    // Compute attached-flow lift
    double C_L_attached = slope * (alpha_rad - alpha0) + elevator_auth * delta_p;
    
    if (alpha_rad <= alpha_tangent_rad) {
        // Attached flow region (linear)
        C_L = C_L_attached;
    } else {
        // Post-stall region: parabolic decay
        // C_L = A*(alpha - alpha_peak)^2 + C_L_peak
        double alpha_offset = alpha_rad - alpha_peak_rad;
        double parabolic = A * alpha_offset * alpha_offset + C_L_peak;
        // Clamp to non-negative (C_L cannot be negative at high alpha)
        C_L = std::max(0.0, parabolic);
    }
    
    // Drag: induced + 2*sin^2(alpha) post-stall with continuity at -4 and 13 deg
    const double cd0 = 0.0175;
    const double cd_induced = 0.055;
    if (alpha_rad <= alpha_tangent_neg_rad) {
        double C_L_neg = slope * (alpha_tangent_neg_rad - alpha0) + elevator_auth * delta_p;
        double C_D_neg = cd0 + cd_induced * C_L_neg * C_L_neg;
        double sin2_delta = std::sin(alpha_rad) * std::sin(alpha_rad)
            - std::sin(alpha_tangent_neg_rad) * std::sin(alpha_tangent_neg_rad);
        C_D = C_D_neg + 2.0 * sin2_delta;
    } else if (alpha_rad >= alpha_tangent_rad) {
        double C_L_pos = slope * (alpha_tangent_rad - alpha0) + elevator_auth * delta_p;
        double C_D_pos = cd0 + cd_induced * C_L_pos * C_L_pos;
        double sin2_delta = std::sin(alpha_rad) * std::sin(alpha_rad)
            - std::sin(alpha_tangent_rad) * std::sin(alpha_tangent_rad);
        C_D = C_D_pos + 2.0 * sin2_delta;
    } else {
        C_D = cd0 + cd_induced * C_L * C_L;
    }
    
    // Pitching moment with damping (with velocity safety check)
    // Cm_alpha(M) = Cm_alpha0 * (1 - k_M * f(M)) with smooth transonic transition
    const double cm_alpha0 = 1;   // baseline low-Mach slope
    const double cm_alpha = cm_alpha0 * (1.0 - k_M * f_M);
    double vitesse_safe = std::max(vitesse, 1.0);
    C_m = -0.1 - cm_alpha * (alpha_rad - alpha0) - 1.46 * delta_p
        - 12 * omega * l / vitesse_safe;
}