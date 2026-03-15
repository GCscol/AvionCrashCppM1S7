#ifndef OPTI_SAUVETAGE_GENERAL
#define OPTI_SAUVETAGE_GENERAL

#include <vector>
#include <algorithm>  //pour avoir lower bound
#include <fstream>

class Avion;

extern const std::vector<double> SEUILS_Z;
extern const std::vector<double> SEUILS_VZ;
extern const std::vector<double> SEUILS_VTOT;
extern const std::vector<double> SEUILS_PITCH;
extern const std::vector<double> SEUILS_GAMMA;
int Discretisation(const double valeur, const std::vector<double>& seuils) ;


class OptiSauvetageGeneral {
    public :
        struct ParamsRescue {
                std::vector<int> z_env ;
                std::vector<int> vz_env ;
                std::vector<int> vtot_env ;
                std::vector<int> pitch_env ;
                std::vector<int> gamma_env ;
                // possiblement yaw et roll s'ils ne sont plus constamment nuls

                // réponse 
                std::vector<double> cmd_thrust_ratio_max ;// Thrust reduction factor [0, 1]
                std::vector<double> cmd_prof_ratio_max ; // Elevator reduction factor [-1, 1]

                double fitness =-1e9; 
                // possiblement t_recup et Deltaz ou zdecrochage 


                void push_gene(const int z_elem, const int vz_elem, const int vtot_elem, const int pitch_elem, const int gamma_elem, 
                            const double cmd_thrust_ratio_max_elem, const double cmd_prof_ratio_max_elem );
                void reserve_genes(int capacity);

                ParamsRescue ();
            };
            
    private :
        int Nbr_chr = 50 ; /// nombre de chromosomes = nbr d'avions simulés
        int Nbr_chr_kept = 15 ; // nombre de chromosomes gardés selon la fitness pour repeupler
        int MutationRate_times100 = 7 ; // nbr de genes touchés en %*100
        int Nbr_generation = 100;

        ParamsRescue Croisement(const ParamsRescue chromo_m, const ParamsRescue chromo_p);
        ParamsRescue Mutation( ParamsRescue chromo);

    public :
        // à refaire 
        static void LogGenerationStats(const std::string& filename, int generation,
                                const std::vector<double>& altitudes,
                                const std::vector<double>& temps,
                                const std::vector<double>& fitness);
        //
            
        double Eval_Fitness(const double derniere_altitude_recuperation, const double dernier_temps_recuperation);
        std::vector<ParamsRescue> SortAndKeep(const std::vector<ParamsRescue>& Population_parents);

        std::vector<ParamsRescue> population; // ensemble de chromosomes = d'avion
        std::vector<ParamsRescue> Create_Population (const int nbr_chr, const std::vector<ParamsRescue>& Population_parents);

        //lien avec fichier txt
        void SaveBestChrom(const std::string& filename, const ParamsRescue& best) const;

        OptiSauvetageGeneral();
        ~OptiSauvetageGeneral();

        int get_Nbr_chr() const;
        int get_Nbr_chr_kept() const;
        int get_Nbr_generation() const;

};

#endif // OPTI_SAUVETAGE_GENERAL