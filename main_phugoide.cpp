#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <limits>
#include "LectureNACA.cpp"
#include <algorithm>

using namespace std;
const double RAD_TO_DEG = 180.0 / M_PI;
const double DEG_TO_RAD = 1 / RAD_TO_DEG;
const double g = 9.81;


class Aerodynamique {

public:
    double C_L = 0.0f;   // Coefficient de portance
    double C_D = 0.0f;   // Coefficient de traînée
    double C_m = 0.0f;   // Coefficient de moment de tangage
    double delta_m; // action de la gouverne
    double S;     // Surface alaire de l'avion (en m²)
    double rho;   // Densité de l'air (en kg/m³)
    double delta_profondeur = -0.110398;   // Choisi à alpha=1.5 deg UTILISER LE CODE EN COMMENTAIRE CI DESSOUS POUR LE RECALCULER
    double l = 6.6f; // corde moyenne
    double omega_pitch = 0;
    Aerodynamique(double s, double air_density) : S(s), rho(air_density) {}

    void update_from_polar(double alpha_rad, double delta_p, double omega, double vitesse) { // q = d theta/dt = omega
        double alpha_deg = alpha_rad * RAD_TO_DEG;
        if(alpha_deg<=8) // condition de décrochage
            C_L = 5 * (alpha_rad - (-0.035)) + 0.44 * delta_p;
        else
            C_L = 0.0;
        C_D = 0.0175 + 0.055 * std::pow(C_L, 2);

        C_m = -0.1 - (alpha_rad- (-0.035)) - 1.46 * delta_p + (-12)*omega*l/vitesse;
    }

    double calculer_portance(double vitesse) {
        return 0.5f * rho * std::pow(vitesse, 2) * S * C_L;
    }
    double calculer_trainee(double vitesse) {
        return 0.5f * rho * std::pow(vitesse, 2) * S * C_D;
        }

    double calculer_moment_pitch(double vitesse) {

        //update_from_polar(alpha_rad, delta_p, omega, vitesse);
        return 0.5f * rho * std::pow(vitesse, 2) * S * C_m * l; // moment de tangage, il y a aussi une contribution de la vitesse au Cm, à multiplier par cos(alpha)?
    }



};

class Avion {
public:
    // Variables de position, vitesse, orientation et masse
    double x, y, z;          // Position dans l'espace (en m)
    double vitesse_x, vitesse_y, vitesse_z; // Vitesse (en m/s)
    double roll, pitch, yaw; // Orientation (en radians)
    double masse;            // Masse de l'avion (en kg)
    Aerodynamique aero;     // Modèle aérodynamique
    // Variables pour les forces et moments
    double portance, trainee, traction, poids;

    double Fx, Fy, Fz;
    double M_pitch;  // Moment/vitesse angulaire de tangage
    bool en_croisiere = true;
    double delta_p_max = 0.15; // possiblement à changer
    double cmd_profondeur = 0.0; // commande du pilote entre -1 et 1 (négatif pour tirer, positif pour pousser)
    double cmd_thrust =0.667520; // commande de poussée entre 0 et 1
    double altitude_croisière = 11000;
    Avion(double m, Aerodynamique& aero)
        : masse(m), aero(aero), x(0), y(0), z(0),
          vitesse_x(0), vitesse_y(0), vitesse_z(0), roll(0), pitch(0), yaw(0),
          portance(0), trainee(0), traction(0), poids(0), Fx(0), Fy(0), Fz(0), M_pitch(0) {}

    void initialiser() {
        // Initialisation des paramètres de l'avion
        x = 0; y = 0; z = 11000;
        vitesse_x = 285.0; vitesse_y = 0; vitesse_z = 0;
        roll = 0; pitch = 1.0f/RAD_TO_DEG; yaw = 0;
    }

    void calculer_forces(double &portance, double &trainee, double &traction, double &poids) {
        double vitesse = std::sqrt(vitesse_x * vitesse_x + vitesse_y * vitesse_y + vitesse_z * vitesse_z);
        portance = aero.calculer_portance(vitesse);
        trainee = aero.calculer_trainee(vitesse);
        poids = masse * g;

        double traction_max = 2 * aero.rho / 1.292 * 300000.0; // 2 moteur de F1=rho/rho0 * F_max
        //cmd_thrust = min(trainee / traction_max,1.0); //puissance relative des moteurs, nécessaire pour contrebalancer la trainee en croisère


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
    double trouver_alpha(double vitesse, double tol=1e-6) {

        const double alpha_min = 0.5 / RAD_TO_DEG;  // radians
        const double alpha_max = 4.0  / RAD_TO_DEG; // radians
        const int n = 1000;

        double alpha = alpha_min;
        double d_alpha = (alpha_max - alpha_min) / n;

        double L = 0.0f;
        double W = masse * g;

        for (int i = 0; i < n; ++i) {
                double omega = omega_pitch;

            // 1️ profondeur qui annule le moment pour cet alpha
            double delta_p = trouver_delta_profondeur(vitesse, omega);

            // 2️ calcul portance pour cet alpha
            aero.update_from_polar(alpha, delta_p, omega, vitesse);
            L = aero.calculer_portance(vitesse);

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



    // Ajuster vitesse ou alpha pour que L = W



    double omega_pitch = 0;
    void mettre_a_jour_etat(double dt) { // Méthode d'Euler
        const double I_y = 9720000.0; // moment d'inertie autour de l'axe de tangage
        double L = 0.0, D = 0.0, T = 0.0, W = 0.0;

        double vitesse = sqrt(vitesse_x * vitesse_x + vitesse_y * vitesse_y + vitesse_z * vitesse_z);
        double gamma = atan2(vitesse_z, vitesse_x);
        double alpha = pitch - gamma;

        if (alpha*RAD_TO_DEG < -15.25 || alpha*RAD_TO_DEG > 18.0) {
           // cout << "⚠️ alpha hors domaine : " << alpha*RAD_TO_DEG << "°" << std::endl;
        }

        double delta_p = aero.delta_profondeur + cmd_profondeur * delta_p_max;
        aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse);
        calculer_forces(L, D, T, W);
        M_pitch = aero.calculer_moment_pitch(vitesse);

        double Fx = T * (std::cos(yaw) * std::cos(pitch)) - D * (std::cos(yaw) * std::cos(gamma)) - L * (std::sin(gamma) * std::cos(roll)); // Remplacement gamma par pitch
        double Fy = T * (std::sin(yaw) * std::cos(pitch)) - D * (std::sin(yaw) * std::cos(gamma)) + L * (std::sin(gamma) * std::sin(roll));
        double Fz = T *  std::sin(pitch) - D * (std::sin(gamma)) + L * (std::cos(gamma)) - W;

        // Calcul des accélérations et mises à jour des vitesses et positions
        double accel_x = Fx / masse;  // Accélération dans la direction X
        double accel_y = Fy / masse;  // Accélération dans la direction Y
        double accel_z = Fz / masse;  // Accélération dans la direction Z

        vitesse_x += accel_x * dt;
        vitesse_y += accel_y * dt;
        vitesse_z += accel_z * dt;

        x += vitesse_x * dt;
        y += vitesse_y * dt;
        z += vitesse_z * dt;

        // Mise à jour de l'angle de tangage
        double accel_pitch = M_pitch / I_y;
        omega_pitch += accel_pitch * dt;
        pitch += omega_pitch * dt;

        //std::cout << "alpha=" << alpha*RAD_TO_DEG <<", pitch=" << pitch*RAD_TO_DEG <<"\n";
        //std::cout << "Forces: Portance=" << L << " N, Trainée=" << D << " N, Traction=" << T << " N, Poids=" << W << " N\n";
        //std::cout << "Forces: Fx=" << Fx << " N, Fy=" << Fy << " N, Fz=" << Fz << " N\n";
        //std::cout << "Moments: M_pitch=" << M_pitch << " N.m\n";


    }
};

int main() {
    setlocale(LC_ALL, ".utf8"); // Pour les accents

    Aerodynamique aero(361.6, 0.3639); // Surface alaire, densité de l'air
    Avion avion(140178.9, aero); // masse, aero
    avion.initialiser();

    const double dt = 0.01;
    const double total_time = 2000.0;
    const int steps = static_cast<int>(total_time / dt);


    const char* csv_filename = "simulation_full.csv";
    std::ofstream csv(csv_filename);
    if (!csv.is_open()) {
        std::cerr << "Impossible d'ouvrir " << csv_filename << " en écriture\n";
        return 1;
    }
    double speed = avion.vitesse_x;
    double alpha_trim = avion.trouver_alpha(speed);
    double delta_trim = avion.trouver_delta_profondeur(speed, avion.omega_pitch);
    avion.pitch = alpha_trim;
    aero.delta_profondeur = delta_trim;



    // En-tête CSV : ajoute ou supprime champs selon besoins
    csv << "time,"
        << "x,y,z,vx,vy,vz,"
        << "roll,pitch,yaw,"
        << "M_pitch,"
        << "Fx,Fy,Fz,portance,trainee,traction,"
        << "Cl,Cd,Cm,"
        << "speed,AoA_deg,cmd_profondeur,alpha,delta_profondeur\n";

    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * dt;
        avion.mettre_a_jour_etat(dt); // simuler une étape
        // Enregistrer les valeurs calculées précédemment
        double speed = std::sqrt(avion.vitesse_x * avion.vitesse_x + avion.vitesse_y * avion.vitesse_y + avion.vitesse_z * avion.vitesse_z);
        double AoA_deg =(static_cast<double>(avion.pitch) - std::atan2(avion.vitesse_z, avion.vitesse_x))*RAD_TO_DEG;
        double alpha = avion.pitch - atan(avion.vitesse_z/avion.vitesse_x);
        //cout << "alpha = " << (avion.pitch - atan(avion.vitesse_z/avion.vitesse_x)) * RAD_TO_DEG << endl;
        // Écriture des données dans le fichier CSV
        //cout << aero.delta_profondeur << endl;

        if (t > 100 && t < 120){
            avion.en_croisiere = false;
            avion.cmd_profondeur = -0.2;   // tirer
            avion.cmd_thrust = 0.9;
        }
        else if (t > 121 && t < 135){
            avion.en_croisiere = false;
            avion.cmd_profondeur = 0.7;   // pousser
        }
        else{
            avion.en_croisiere = true;
            avion.cmd_profondeur = 0.0;
            avion.cmd_thrust = 0.667520;
        }


        csv << t << ',' << avion.x << ',' << avion.y << ',' << avion.z << ',' << avion.vitesse_x << ',' << avion.vitesse_y << ',' << avion.vitesse_z
            << ',' << avion.roll << ',' << avion.pitch << ',' << avion.yaw << ',' << avion.M_pitch << ',' << avion.Fx << ',' << avion.Fy << ','
            << avion.Fz << ',' << avion.portance << ',' << avion.trainee << ',' << avion.traction << ',' << avion.aero.C_L << ',' << avion.aero.C_D << ',' << avion.aero.C_m
            << ',' << speed << ',' << AoA_deg << ',' << avion.cmd_profondeur << ',' << alpha << aero.delta_profondeur <<'\n';


            if(avion.z<=0){
                    cout << "Crash !" << endl;
                break;
            }
    }

    csv.close();
    std::cout << "Simulation enregistrée dans : " << csv_filename << std::endl;
    return 0;
}



















