#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <limits>
#include <algorithm>

using namespace std;
const double RAD_TO_DEG = 180.0 / M_PI;
const double DEG_TO_RAD = 1 / RAD_TO_DEG;
const double g = 9.81;


class Environnement {
public:
    double rho_0 = 1.225;  // Densité de l'air au niveau de la mer (kg/m³)
    double T_0 = 288.15;   // Température au niveau de la mer (K) = 15°C
    double L = 0.0065;     // Gradient thermique (K/m)
    double R = 287.05;     // Constante des gaz pour l'air (J/(kg·K))
    
    double calculer_rho(double altitude) {
        if (altitude < 0) altitude = 0;
        double T = T_0 - L * altitude;
        
        if (T < 0) T = 216.65;  // Température minimum (stratosphère)

        double rho = rho_0 * pow(T / T_0, g / (R * L) - 1);
        return rho;
    }
};


class Aerodynamique {
public:
    double C_L = 0.0f;   // Coefficient de portance
    double C_D = 0.0f;   // Coefficient de traînée
    double C_m = 0.0f;   // Coefficient de moment de tangage
    double delta_m; // action de la gouverne
    double S;     // Surface alaire de l'avion (en m²)
    double delta_profondeur = -0.110398;   // Choisi à alpha=1.5 deg UTILISER LE CODE EN COMMENTAIRE CI DESSOUS POUR LE RECALCULER
    double l = 6.6f; // corde moyenne
    double omega_pitch = 0;
    Aerodynamique(double s) : S(s) {}

    // void update_from_polar(double alpha_rad, double delta_p, double omega, double vitesse, double alt) { //Fonction prenant en compte effets de compressibilité sur la trainée
    //     double alpha_deg = alpha_rad * RAD_TO_DEG;
    //     if(alpha_deg <= 8) {
    //         C_L = 5 * (alpha_rad - (-0.035)) + 0.44 * delta_p;
    //     } else {
    //         C_L = 0.0;
    //     }
    //     double C_D_base = 0.0175 + 0.055 * std::pow(C_L, 2);
    //     double speed_of_sound = 320.0 - 0.065 * alt; // m/s à moyenne altitude
    //     double Mach = vitesse / speed_of_sound;
    //     // Augmentation de traînée due à la compressibilité (effet de Mach)
    //     double C_D_compressibility = 0.0;
    //     if (Mach > 0.6) {
    //         C_D_compressibility = 0.1 * std::pow((Mach - 0.6) / 0.4, 2);
    //     }
    //     if (Mach > 1.0) {
    //         C_D_compressibility += 0.3 * (Mach - 1.0); // Traînée d'onde de choc
    //     }
    //     C_D = C_D_base + C_D_compressibility;
    //     C_m = -0.1 - (alpha_rad- (-0.035)) - 1.46 * delta_p + (-12)*omega*l/vitesse;
    // }

    void update_from_polar(double alpha_rad, double delta_p, double omega, double vitesse) {
        double alpha_deg = alpha_rad * RAD_TO_DEG;

        if(alpha_deg <= 15.0) { 
            C_L = 5 * (alpha_rad - (-0.035)) + 0.44 * delta_p;  // Régime linéaire avant décrochage
        } else if(alpha_deg <= 20.0) {
            double alpha_stall_rad = 8.0 * DEG_TO_RAD;
            double CL_max = 5 * (alpha_stall_rad - (-0.035)) + 0.44 * delta_p;
            C_L = CL_max * (1.0 - 0.1 * (alpha_deg - 8.0)); // Décroissance linéaire après décrochage
        } else {
            C_L = 0.1; // Portance résiduelle ||||A MODIFIER de sorte à ce que la polaire soit continue|||||||||||||||||||||||||||||||||||
        }
        
        C_D = 0.0175 + 0.055 * std::pow(C_L, 2);
        C_m = -0.1 - (alpha_rad - (-0.035)) - 1.46 * delta_p + (-12)*omega*l/vitesse;
    }

    double calculer_portance(double vitesse, double rho) {
        return 0.5f * rho * std::pow(vitesse, 2) * S * C_L;
    }
    double calculer_trainee(double vitesse, double rho) {
        return 0.5f * rho * std::pow(vitesse, 2) * S * C_D;
        }

    double calculer_moment_pitch(double vitesse, double rho) {
        return 0.5f * rho * std::pow(vitesse, 2) * S * C_m * l; // moment de tangage, il y a aussi une contribution de la vitesse au Cm, à multiplier par cos(alpha)?
    }
};

class Avion {
public:
    double x, y, z;          // Position dans l'espace (en m)
    double vitesse_x, vitesse_y, vitesse_z; // Vitesse (en m/s)
    double roll, pitch, yaw; // Orientation (en radians)
    double masse;            // Masse de l'avion (en kg)
    Aerodynamique aero;     // Modèle aérodynamique
    Environnement env;      // Environnement (densité de l'air, etc.)

    double portance, trainee, traction, poids;
    double Fx, Fy, Fz;
    double M_pitch;  // Moment/vitesse angulaire de tangage

    double delta_p_max = 0.15; // possiblement à changer ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    double cmd_profondeur = 0.0; // commande du pilote entre -1 et 1 (négatif pour tirer, positif pour pousser)
    double cmd_thrust = 0.667520; // commande de poussée entre 0 et 1
    double altitude_croisiere = 11000;
    
    Avion(double m, Aerodynamique& aero)
        : masse(m), aero(aero), x(0), y(0), z(0),
          vitesse_x(0), vitesse_y(0), vitesse_z(0), roll(0), pitch(0), yaw(0),
          portance(0), trainee(0), traction(0), poids(0), Fx(0), Fy(0), Fz(0), M_pitch(0) {}

    void initialiser() { //// A MODIF EN FCT DES CONDITIONS DE VOL REELES AVANT L'INCIDENT ||||||||||||||||||||||||||||||||||
        x = 0; y = 0; z = 11000;
        vitesse_x = 285.0; vitesse_y = 0; vitesse_z = 0;
        roll = 0; pitch = 1.0f/RAD_TO_DEG; yaw = 0;
    }

    double calculer_poussee_max(double vitesse, double rho, double altitude) {
        double F0 = 300000.0; // 300 kN par moteur
        double n_moteurs = 2.0;

        double sigma = rho / 1.225; // Densité relative
        double F_altitude = n_moteurs * F0 * std::pow(sigma, 1); // Pour un turboréacteur, poussée ~ σ^0.7 à σ^0.9 dans d'autres modèles ||||||||||||||||
        
        // Variation avec la vitesse (ram drag)
        // double F_speed_correction = 1.0 - 0.3 * (vitesse / 300.0);
        // if (F_speed_correction < 0.3) F_speed_correction = 0.3;
        // // Effet de Mach
        // double speed_of_sound = 340.0 - 0.0065 * altitude;
        // double Mach = vitesse / speed_of_sound;
        // double F_mach_correction = 1.0;
        // if (Mach > 0.8) {
        //     F_mach_correction = 1.0 - 0.5 * (Mach - 0.8);
        // }
        // if (Mach > 1.2) {
        //     F_mach_correction = 0.4; // Poussée réduite en supersonique
        // }
        
        return F_altitude; // * F_speed_correction * F_mach_correction;
    }

    void calculer_forces(double &portance, double &trainee, double &traction, double &poids) {
        double vitesse = std::sqrt(vitesse_x * vitesse_x + vitesse_y * vitesse_y + vitesse_z * vitesse_z);
        double rho = env.calculer_rho(z);
        portance = aero.calculer_portance(vitesse, rho);
        trainee = aero.calculer_trainee(vitesse, rho);
        poids = masse * g;

        double traction_max = calculer_poussee_max(vitesse, rho, z);

        traction = cmd_thrust * traction_max;
        this->portance = portance;
        this->trainee  = trainee;
        this->traction = traction;
        this->poids    = poids;
    }
         // Recherche du delta_profondeur tel que C_m soit le plus proche de zéro
    double trouver_delta_profondeur(double vitesse, double omega, double epsilon = 1e-6f) {
    // Liste de valeurs possibles pour delta_profondeur (par exemple, entre -1.0 et 1.0)
        int n = 10000;  // Nombre de valeurs à tester
        double delta_p_min = -0.13f;
        double delta_p_max = -0.01f;
        double step = (delta_p_max - delta_p_min) / n;

        double best_delta_p = delta_p_min;
        double min_C_m = numeric_limits<double>::infinity();

        for (int i = 0; i <= n; ++i) {
            double delta_p = delta_p_min + i * step;
            aero.update_from_polar(pitch, delta_p, omega, vitesse);
            double C_m_guess = aero.C_m;

            if (std::fabs(C_m_guess) < std::fabs(min_C_m)) {
                min_C_m = C_m_guess;
                best_delta_p = delta_p;
            }

            //std::cout << "delta_p = " << delta_p << " , C_m = " << C_m_guess << std::endl;

            if (std::fabs(C_m_guess) < epsilon) {
                break;
            }
        }

        return best_delta_p;
    }
    // Ajuster alpha pour que L = W
    double trouver_alpha(double vitesse, double tol=1e-6) {

        const double alpha_min = 0.5 / RAD_TO_DEG;  // radians
        const double alpha_max = 4.0  / RAD_TO_DEG; // radians
        const int n = 1000;

        double alpha = alpha_min;
        double d_alpha = (alpha_max - alpha_min) / n;

        double L = 0.0f;
        double W = masse * g;
        double rho = env.calculer_rho(z);

        for (int i = 0; i < n; ++i) {
                double omega = omega_pitch;

            // 1️ profondeur qui annule le moment pour cet alpha
            double delta_p = trouver_delta_profondeur(vitesse, omega);

            // 2️ calcul portance pour cet alpha
            aero.update_from_polar(alpha, delta_p, omega, vitesse);
            L = aero.calculer_portance(vitesse, rho);

            // 3️ critère de convergence
            if (fabs(L - W) < tol)
                break;

            // 4️ ajustement alpha
            if (L < W)
                alpha += d_alpha;   // pas assez de portance
            else
                alpha -= d_alpha;   // trop de portance
        }

        return alpha;
    }

    double omega_pitch = 0;
    void mettre_a_jour_etat(double dt) { // Méthode d'Euler
        const double I_y = 9720000.0; // moment d'inertie autour de l'axe de tangage
        double L = 0.0, D = 0.0, T = 0.0, W = 0.0;

        double vitesse = sqrt(vitesse_x * vitesse_x + vitesse_y * vitesse_y + vitesse_z * vitesse_z);
        double gamma = atan2(vitesse_z, vitesse_x);
        double alpha = pitch - gamma;
        double rho = env.calculer_rho(z);

        double delta_p = aero.delta_profondeur + cmd_profondeur * delta_p_max;
        aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse);
        calculer_forces(L, D, T, W);
        M_pitch = aero.calculer_moment_pitch(vitesse, rho);

        double Fx = T * (std::cos(yaw) * std::cos(pitch)) - D * (std::cos(yaw) * std::cos(gamma)) - L * (std::sin(gamma) * std::cos(roll)); // Remplacement gamma par pitch
        double Fy = T * (std::sin(yaw) * std::cos(pitch)) - D * (std::sin(yaw) * std::cos(gamma)) + L * (std::sin(gamma) * std::sin(roll));
        double Fz = T *  std::sin(pitch) - D * (std::sin(gamma)) + L * (std::cos(gamma)) - W;

        double accel_x = Fx / masse;  // Accélération dans la direction X
        double accel_y = Fy / masse;  // Accélération dans la direction Y
        double accel_z = Fz / masse;  // Accélération dans la direction Z
        vitesse_x += accel_x * dt;
        vitesse_y += accel_y * dt;
        vitesse_z += accel_z * dt;
        x += vitesse_x * dt;
        y += vitesse_y * dt;
        z += vitesse_z * dt;

        double accel_pitch = M_pitch / I_y;
        omega_pitch += accel_pitch * dt;
        pitch += omega_pitch * dt;
    }
};




void analyser_limites_vitesse(Avion& avion) {
    cout << "Alt (m) | Vmax (m/s) | Mach_max | Vmin (m/s) | Mach_min | Vmin_th (m/s)" << endl;
    
    ofstream csv("vmax_analysis.csv");
    csv << "altitude,v_max_m_s,Mach_max,v_min,Mach_min,v_min_th\n";
    
    for (double alt : {5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 13000, 14000, 15000}) {
        avion.z = alt;
        double rho = avion.env.calculer_rho(alt);
        double v_max = 500.0;  // Vitesse initiale maximale à tester
        double v_min = 10;

        double S = avion.aero.S;
        double W = avion.masse * g;
        double alpha_stall_rad = 15.0 * DEG_TO_RAD;
        double CL_max = 5 * (alpha_stall_rad - (-0.035)) + 0.44 * (-0.13);  // ||||||||||||| braquage de gouverne à -0,13 |||||||||||||||||||

        double v_min_theorique = sqrt(2 * W / (rho * S * CL_max));

        for (double v_test = v_max; v_test >= 150.0; v_test -= 0.1) {
            double delta_p = avion.trouver_delta_profondeur(v_test, 0);  // calcul le plus long de la boucle
            double F_available = avion.calculer_poussee_max(v_test, rho, alt);  // Poussée disponible à v_test

            double CL_needed = (2 * W) / (rho * v_test * v_test * avion.aero.S);   // CL nécessaire pour sustentation (L = W)
            if (CL_needed > CL_max) {continue;}  // Vérifier que CL_needed est réalisable (inférieur à CL_max)

            double alpha_needed = ((CL_needed - 0.44 * delta_p) / 5.0) + (-0.035);
            
            avion.aero.update_from_polar(alpha_needed, -0.13, 0, v_test);
            double D = avion.aero.calculer_trainee(v_test, rho);
            
            
            if (F_available <= D) {v_max = v_test;}   // Vérifier l'équilibre F = D
            else if (F_available > D) {v_min = v_test;}
            if (v_test < 10.0) {break;}
            if (v_min > v_max) {if (v_min > v_max) {std::swap(v_min, v_max);}}  // Si le coin supérieur du domaine de vol est atteint
        }
        
        double sound_speed = 340.0 - 0.0065 * alt;
        double Mach_max = v_max / sound_speed;
        double Mach_min = v_min / sound_speed;

        cout << fixed << "\n" ;
        cout << alt << " | " << v_max << " | " << Mach_max << " | " << v_min << " | " << Mach_min << " | " << v_min_theorique << endl;
        csv << alt << "," << fixed << v_max << "," << Mach_max << "," << v_min << "," << Mach_min << "," << v_min_theorique << endl;
    }
    csv.close();
    cout << "Résultats sauvegardés dans 'vmax_analysis.csv'" << endl;
}


//// ============================================ Main pour le domaine de vol ===================================================
int main() {
    Aerodynamique aero(361.6);
    Avion avion(200000, aero);

    analyser_limites_vitesse(avion);

    return 0;
}



/// ============================================ Main pour la simulation principale de vol ===================================================
// int main() {
//     setlocale(LC_ALL, ".utf8"); // Pour les accents

//     Aerodynamique aero(361.6); // Surface alaire
//     Avion avion(140178.9, aero); // masse, aero
//     avion.initialiser();

//     const double dt = 0.01;
//     const double total_time = 2000.0;
//     const int steps = static_cast<int>(total_time / dt);


//     const char* csv_filename = "simulation_full.csv";
//     std::ofstream csv(csv_filename);
//     if (!csv.is_open()) {
//         std::cerr << "Impossible d'ouvrir " << csv_filename << " en écriture\n";
//         return 1;
//     }
//     double speed = avion.vitesse_x;
//     double alpha_trim = avion.trouver_alpha(speed);
//     double delta_trim = avion.trouver_delta_profondeur(speed, avion.omega_pitch);
//     avion.pitch = alpha_trim;  // pitch=alpha en croisère
//     aero.delta_profondeur = delta_trim;



//     // En-tête CSV : ajoute ou supprime champs selon besoins
//     csv << "time,"
//         << "x,y,z,vx,vy,vz,"
//         << "roll,pitch,yaw,"
//         << "M_pitch,"
//         << "Fx,Fy,Fz,portance,trainee,traction,"
//         << "Cl,Cd,Cm,"
//         << "speed,AoA_deg,cmd_profondeur,alpha,delta_profondeur,n_factor\n";

//     for (int i = 0; i < steps; ++i) {
//         double t = (i + 1) * dt;
//         avion.mettre_a_jour_etat(dt); // simuler une étape

//         double speed = std::sqrt(avion.vitesse_x * avion.vitesse_x + avion.vitesse_y * avion.vitesse_y + avion.vitesse_z * avion.vitesse_z);
//         double AoA_deg =(static_cast<double>(avion.pitch) - std::atan2(avion.vitesse_z, avion.vitesse_x))*RAD_TO_DEG;
//         double alpha = avion.pitch - atan(avion.vitesse_z/avion.vitesse_x);
//         //cout << "alpha = " << (avion.pitch - atan(avion.vitesse_z/avion.vitesse_x)) * RAD_TO_DEG << endl;
//         // Écriture des données dans le fichier CSV
//         //cout << aero.delta_profondeur << endl;

//         if (t >= 100 && t < 500){
//             avion.cmd_profondeur = -0.55;   // tirer
//             avion.cmd_thrust = 0.9;
//         }
//         // if (t >= 120 && t < 120){
//         //     avion.cmd_profondeur = -0.2;   // tirer
//         //     avion.cmd_thrust = 0.9;
//         // }
//         // else if (t >= 120 && t < 135){
//         //     avion.cmd_profondeur = 0.7;   // pousser
//         // }
//         // else {
//         //     avion.cmd_profondeur = 0.0;
//         //     avion.cmd_thrust = 0.667520;
//         // }

//         double n_factor = avion.portance / (avion.masse * g); // Calcul du facteur de charge n = L / (m*g)

//         csv << t << ',' << avion.x << ',' << avion.y << ',' << avion.z << ',' << avion.vitesse_x << ',' << avion.vitesse_y << ',' << avion.vitesse_z
//             << ',' << avion.roll << ',' << avion.pitch << ',' << avion.yaw << ',' << avion.M_pitch << ',' << avion.Fx << ',' << avion.Fy << ','
//             << avion.Fz << ',' << avion.portance << ',' << avion.trainee << ',' << avion.traction << ',' << avion.aero.C_L << ',' << avion.aero.C_D << ',' << avion.aero.C_m
//             << ',' << speed << ',' << AoA_deg << ',' << avion.cmd_profondeur << ',' << alpha << ',' << aero.delta_profondeur << ',' << n_factor <<'\n';

//             if(avion.z<=0){
//                 cout << "Crash !" << endl;
//                 break;
//             }
//     }

//     csv.close();
//     std::cout << "Simulation enregistrée dans : " << csv_filename << std::endl;
//     return 0;
// }