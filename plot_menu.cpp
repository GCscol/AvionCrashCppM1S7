#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

void afficher_menu() {
    cout << "\n=== MENU PLOTS ===" << endl;
    cout << "1. Simulation" << endl;
    cout << "2. Rescue comparison" << endl;
    cout << "3. Aerodynamic coefficients" << endl;
    cout << "4. CL vs alpha" << endl;
    cout << "5. Phugoid" << endl;
    cout << "6. Min rescue altitude" << endl;
    cout << "7. Batch crash heatmap" << endl;
    cout << "0. Quitter" << endl;
    cout << "\nEntrez votre choix: ";
}

int main() {
    int choix;
    string commande;
    
    while (true) {
        afficher_menu();
        cin >> choix;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Entree invalide. Veuillez entrer un nombre." << endl;
            continue;
        }
        
        switch (choix) {
            case 1:
                cout << "\nExecution de plot_simulation.py..." << endl;
                commande = "python \"Plot Python/plot_simulation.py\"";
                system(commande.c_str());
                break;
                
            case 2:
                cout << "\nExecution de plot_rescue_comparison.py..." << endl;
                commande = "python \"Plot Python/plot_rescue_comparison.py\"";
                system(commande.c_str());
                break;
                
            // option 3 (Flight envelope) removed

            case 3:
                cout << "\nExecution de plot_aerodynamic_coefficients.py..." << endl;
                commande = "python \"Plot Python/plot_aerodynamic_coefficients.py\"";
                system(commande.c_str());
                break;

            case 4:
                cout << "\nExecution de plot_CL_vs_alpha.py..." << endl;
                commande = "python \"Plot Python/plot_CL_vs_alpha.py\"";
                system(commande.c_str());
                break;
            case 5:
                cout << "\nExecution de Phugoide.py..." << endl;
                commande = "python \"Plot Python/Phugoide.py\"";
                system(commande.c_str());
                break;

            case 6:
                cout << "\nExecution de plot_min_rescue_altitude.py..." << endl;
                commande = "python \"Plot Python/plot_min_rescue_altitude.py\"";
                system(commande.c_str());
                break;

            case 7:
                cout << "\nExecution de plot_heatmap_crash.py..." << endl;
                commande = "python \"Plot Python/plot_heatmap_crash.py\"";
                system(commande.c_str());
                break;
                
            case 0:
                cout << "Au revoir!" << endl;
                return 0;
                
            default:
                cout << "Choix invalide. Veuillez choisir entre 0 et 7." << endl;
        }
    }
    
    return 0;
}
