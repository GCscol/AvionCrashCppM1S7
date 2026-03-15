#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstdlib>   // pour std::rand
#include <cmath>     // pour std::exp
#include "Avion.h"
#include "OptiSauvetageGeneral.h"
#include "Constantes.h"


// Constante pour seuil de discretisation . Attention faut etre sur que ce sont les memes seuils que ceux utilises pour la config si on fait un GEN_GIVE
const std::vector<double> SEUILS_Z = { 1000.0, 2000.0, 4000.0, 6000.0, 7000.0, 8000.0, 9000.0, 11000.0, 13000.0 };
const std::vector<double> SEUILS_VZ    = { -80.0, -50.0, -30.0, -15.0, -5.0, 0.0, 5.0, 15.0, 30.0 };
const std::vector<double> SEUILS_VTOT  = { 80.0, 120.0, 150.0, 180.0, 220.0, 260.0, 300.0 };
const std::vector<double> SEUILS_PITCH = { -0.5, -0.35, -0.2, -0.1, 0.0, 0.1, 0.2, 0.35, 0.5 };
const std::vector<double> SEUILS_GAMMA = { -0.8, -0.5, -0.3, -0.1, 0.0, 0.1, 0.2, 0.5, 0.8 };

int Discretisation(const double valeur, const std::vector<double>& seuils) {  // nécessaire pour facilement traduire une situation continu dans le memoire du chromosome 
    return int(std::lower_bound(seuils.begin(), seuils.end(), valeur) - seuils.begin());
} 


// Destructeurs + Constructeurs
OptiSauvetageGeneral::ParamsRescue::ParamsRescue() {
    reserve_genes(100);
}

OptiSauvetageGeneral::OptiSauvetageGeneral(){
    population.resize(Nbr_chr);
}

OptiSauvetageGeneral::~OptiSauvetageGeneral() {}

//getters
int OptiSauvetageGeneral::get_Nbr_generation() const {
    return OptiSauvetageGeneral::Nbr_generation;
}

int OptiSauvetageGeneral::get_Nbr_chr() const {
    return OptiSauvetageGeneral::Nbr_chr;
}

int OptiSauvetageGeneral::get_Nbr_chr_kept() const {
    return Nbr_chr_kept;
}


// gestion fichier
void OptiSauvetageGeneral::SaveBestChrom(const std::string& filename, const ParamsRescue& best) const {
    std::ofstream f(filename);  // écrase à chaque fois
    f << "# fitness=" << best.fitness << "\n";
    f << "# z_env,vz_env,vtot_env,pitch_env,gamma_env,cmd_thrust,cmd_prof\n";
    for (int i = 0; i < (int)best.vz_env.size(); i++) {
        f << best.z_env[i] << "," << best.vz_env[i] << ","
          << best.vtot_env[i] << "," << best.pitch_env[i] << ","
          << best.gamma_env[i] << ","
          << best.cmd_thrust_ratio_max[i] << ","
          << best.cmd_prof_ratio_max[i] << "\n";
    }
}

// à refaire
void OptiSauvetageGeneral::LogGenerationStats(const std::string& filename, int generation,
                                               const std::vector<double>& altitudes,
                                               const std::vector<double>& temps,
                                               const std::vector<double>& fitness) {
    // Ouvre en append pour accumuler toutes les générations dans le même fichier
    std::ofstream f(filename, std::ios::app);
    for (int k = 0; k < (int)altitudes.size(); k++) {
        f << generation << "," << k << "," 
          << altitudes[k] << "," << temps[k] << "," << fitness[k] << "\n";
    }
}
//


// gestion gene
void OptiSauvetageGeneral::ParamsRescue::push_gene(const int z_elem, const int vz_elem, const int vtot_elem, const int pitch_elem, const int gamma_elem, 
               const double cmd_thrust_ratio_max_elem, const double cmd_prof_ratio_max_elem ){
    z_env.push_back(z_elem);
    vz_env.push_back(vz_elem);
    vtot_env.push_back(vtot_elem);
    pitch_env.push_back(pitch_elem);
    gamma_env.push_back(gamma_elem);
    cmd_thrust_ratio_max.push_back(cmd_thrust_ratio_max_elem);
    cmd_prof_ratio_max.push_back(cmd_prof_ratio_max_elem);
} 

void OptiSauvetageGeneral::ParamsRescue::reserve_genes (const int capacity){
    z_env.reserve(capacity);
    vz_env.reserve(capacity);
    vtot_env.reserve(capacity);
    pitch_env.reserve(capacity);
    gamma_env.reserve(capacity);
    cmd_thrust_ratio_max.reserve(capacity);
    cmd_prof_ratio_max.reserve(capacity);
}

// Fonctions
double OptiSauvetageGeneral::Eval_Fitness(const double derniere_altitude_recuperation, const double dernier_temps_recuperation) {
    if (dernier_temps_recuperation <= 0.0) {  // punir si sauvegarde ne s'active pas = crash immédiat
        return -1e8; 
    }
    // on souhaite avoir fitnesse la plus grande possible 
    // (c'est pas ouf dans les effets on aimerait tendre vers 0 pour pouvoir bien quantifier l'évolution de la fitness)
    else {  // si fitness neg c'est qu'on a crash  // + altitude neg ⁼ plus on avait de la vitesse au moment de l'impact
        return ( 
            10*(derniere_altitude_recuperation-1.0) /(100*(dernier_temps_recuperation+1.0)) 
        );
    };
}

std::vector<OptiSauvetageGeneral::ParamsRescue> OptiSauvetageGeneral::SortAndKeep(const std::vector<OptiSauvetageGeneral::ParamsRescue>& Population_parents) {
    std::vector<OptiSauvetageGeneral::ParamsRescue> Population_select = Population_parents;
    // Tri décroissant par fitness
    std::sort(Population_select.begin(), Population_select.end(),
        [](const OptiSauvetageGeneral::ParamsRescue& a, const OptiSauvetageGeneral::ParamsRescue& b) {   // [] = fonction lambda en python
            return a.fitness > b.fitness;
        });
    
    if (int(Population_select.size()) <Nbr_chr_kept) {  // debugage
        throw std::runtime_error("Problème taille dans SortAndKeep, size(pop parent)<nbr chromo kept");
    }

    Population_select.resize(Nbr_chr_kept);     // Garder uniquement les meilleurs chr
    return Population_select ;
}

// gestion de la population
OptiSauvetageGeneral::ParamsRescue OptiSauvetageGeneral::Croisement( ParamsRescue chromo_m, ParamsRescue chromo_p) {
    int size_m = chromo_m.vz_env.size();
    int size_p = chromo_p.vz_env.size();

    // Creation mémoire indice pere libre
    std::vector<bool> Memoire_gene_p_free(size_p, true);

    //Préconstruction du chromo_f
    ParamsRescue chromo_f; 
    int capacite_estimee = (size_m+size_p)*0.7;
    chromo_f.reserve_genes(capacite_estimee);

    // parcours genes
    for (int n_m=0; n_m<size_m; n_m++ ){
        int n_p=-1 ; // Manière de détecter si on a trouver l'indice' ou pas.
        for (int k_p = 0; k_p < size_p; k_p++) {
            if (chromo_p.z_env[k_p] == chromo_m.z_env[n_m]         &&
                chromo_p.vz_env[k_p] == chromo_m.vz_env[n_m]       &&
                chromo_p.vtot_env[k_p] == chromo_m.vtot_env[n_m]   &&
                chromo_p.pitch_env[k_p] == chromo_m.pitch_env[n_m] &&
                chromo_p.gamma_env[k_p] == chromo_m.gamma_env[n_m]) {

                n_p = k_p;
                break;
            }
        }
        if (n_p == -1) { // Pas de gene du pere donc on ajoute la mere
            chromo_f.push_gene(
                chromo_m.z_env[n_m],
                chromo_m.vz_env[n_m],
                chromo_m.vtot_env[n_m],
                chromo_m.pitch_env[n_m],
                chromo_m.gamma_env[n_m],
                chromo_m.cmd_thrust_ratio_max[n_m],
                chromo_m.cmd_prof_ratio_max[n_m]
            );
        }
        else {
            if (std::rand() % 2 == 0) { // 50/50 d'avoir pere ou mere | Si pair, mere gagne
                chromo_f.push_gene(
                    chromo_m.z_env[n_m],
                    chromo_m.vz_env[n_m],
                    chromo_m.vtot_env[n_m],
                    chromo_m.pitch_env[n_m],
                    chromo_m.gamma_env[n_m],
                    chromo_m.cmd_thrust_ratio_max[n_m],
                    chromo_m.cmd_prof_ratio_max[n_m]
                );
            }
            else { // pere gagne
                chromo_f.push_gene(
                    chromo_p.z_env[n_p],
                    chromo_p.vz_env[n_p],
                    chromo_p.vtot_env[n_p],
                    chromo_p.pitch_env[n_p],
                    chromo_p.gamma_env[n_p],
                    chromo_p.cmd_thrust_ratio_max[n_p],
                    chromo_p.cmd_prof_ratio_max[n_p]
                );
            }
            //On enleve l'indice du pere de ceux valables
            Memoire_gene_p_free[n_p] = false;
        }
    }

    // on finit de remplir avec ceux du pere non alloué
    for (int n_p = 0; n_p < size_p; n_p++) {
        if (!Memoire_gene_p_free[n_p]) continue;
        chromo_f.push_gene(
            chromo_p.z_env[n_p],
            chromo_p.vz_env[n_p],
            chromo_p.vtot_env[n_p],
            chromo_p.pitch_env[n_p],
            chromo_p.gamma_env[n_p],
            chromo_p.cmd_thrust_ratio_max[n_p],
            chromo_p.cmd_prof_ratio_max[n_p]
        );
    }

    return chromo_f ;
}

OptiSauvetageGeneral::ParamsRescue OptiSauvetageGeneral::Mutation(ParamsRescue chromo){
    int taille_chromo = chromo.vz_env.size();

    if (taille_chromo == 0) { // possible au début
        std::cout<<"Un chromosome de taille nulle"<<std::endl;
        return chromo; 
    }

    int nbr_mut_max= std::max(std::rand()%2,taille_chromo*MutationRate_times100/100); // Possiblement 0 mut mais ok sinon mettre un min
    
    for (int n_mut=0; n_mut<nbr_mut_max; n_mut++) {
        int n_f = std::rand()%taille_chromo ; // bon c'est pas distribué de manière uniforme comme proba mais vu que taille << RAND_MAX on va dire que c est quasi le cas
        chromo.cmd_thrust_ratio_max[n_f]= std::rand()/double(RAND_MAX); // on divise par le max donc on aura un double
        chromo.cmd_prof_ratio_max[n_f]= -1 +2*std::rand()/double(RAND_MAX);
    }
    return chromo ;
}


std::vector<OptiSauvetageGeneral::ParamsRescue> OptiSauvetageGeneral::Create_Population (const int nbr_chr, 
                                                                                        const std::vector<ParamsRescue>& Population_parents) {
    std::vector<ParamsRescue> population_enfant;
    population_enfant.reserve(nbr_chr);

    int nbr_chr_kept = Population_parents.size();
    
    for (int i=0; i<nbr_chr; i++){
        int n_chr_m=std::rand()%nbr_chr_kept ;
        int n_chr_p=( ( std::rand()%(nbr_chr_kept-1) ) + n_chr_m + 1 )%nbr_chr_kept ;  // normalement ça empeche d'avoir n_chr_p=n_chr_m
        population_enfant.push_back( 
            Mutation( 
                Croisement(Population_parents[n_chr_m],Population_parents[n_chr_p]) 
            )
        );
    }
    return population_enfant;
}

