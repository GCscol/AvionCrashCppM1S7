Ce code a pour objectif d'étudier le sauvetage d'un avion en décrochage en haute altitude. 
À ce titre, il possède plusieurs fonctionnalités. Ce document a pour objectif d'aider l'utilisateur dans la manipulation du code.


=========
1ere etape : renseignée les paramètres de la simulation dans "Config.txt"
=========

La première chose que doit faire l'utilisateur est de choisir l'utilisation qu'il cherche à en faire.


Pour ce faire, un document "Config.txt" est mis à disposition.
Ce document consitue la principale interface avec les programmes en plus des commandes 'make run' et 'make plot'.
A l'intérieur de ce document, la première partie est composé des éléments principaux que l'utilisateur se doit de contrôler afin de maitriser son résultat.
La partie inférieur quant à elle, contient les paramètres spécifiques à chaque fonctions du code et qui doivent être modifiés selon les opérations à réaliser demander au code.

Attention : les noms renseignés après une variable dans "Config.txt" se doivent d'être exactes pour les opérations nommées. De plus, il est primordial de ne pas mettre d'espace entre le "=" et le valeur de la variable renseignée, ni d'espace après le nom de la variable. Ainsi par exemple, "rescue_strategy=GEN_FIND " n'est pas une entrée valide de "Config.txt" mais "rescue_strategy=GEN_FIND" l'est.
Si un problème intervient dès le lancement du programme, il est probable que l'erreur soit dû à la manière dont "Config.txt" a été rempli.

Remarque : un lexique est présent à la fin du document afin d'indiquer à l'utilisateur l'ensemble des paramètres utilisables dans "Config.txt" et leur signification.


Ainsi en l'ouvrant, l'utilisateur choisit les actions (rappelées à la 1ère ligne à "# Operations to perform") qu'il souhaite réaliser en complétant la variable "operations=".
La list de ces opérations est : 
GENERAL_GEN_FIND  : détermination d'une stratégie de sauvetage via algorithme génétique ;
SIMULATION  : simulation du décrochage d'un unique avion ;
ENERGIE  : vérification de la conservation de l'énergie ;
RUN_BATCH  : réalisation de plusieurs simulations en parallèle ;
TEST_INITIALISATION  : test d'initialisation de l'avion ;
COMPARE_RESCUE_STRATEGIES  : obtention de données de comparaison entre les 4 stratégies de sauvetage (Attention : le chemin d'accès d'un chromosome doit être renseigné)
MIN_RESCUE_ALTITUDE  : comparaison des altitudes minimales de sauvetage (Attention : le chemin d'accès d'un chromosome doit être renseigné)
PHUGOID  : mise en évidence de la phugoïde.   


Attention : Les noms indiqués doivent être exactement les bons, écris en majuscule. Dans le cas où plusieurs opérations sont renseignées, l'utilisateur doit seulement séparer les opérations par une ",".
Exemple d'opérations possible : 
operations=SIMULATION,COMPARE_RESCUE_STRATEGIES,MIN_RESCUE_ALTITUDE



Si l'utilisateur choisit l'option SIMULATION, il peut également choisir d'activer le sauvetage automatique en cas de décrochage ("enable_rescue_system=true") ou non ("enable_rescue_system=false").
Par défaut, le sauvetage est activé.

Si l'utilisateur choisit d'activer le sauvetage, il doit alors renseigner la méthode voulue ("rescue_strategy=") :

THRUST_FIRST
PROFILE_FIRST
SIMULTANEOUS
GEN_GIVE 

Exemple : rescue_strategy=PROFILE_FIRST

Si l'utilisateur choisit une stratégie génétique, il se doit de renseigner l'emplacement du chromosome à utiliser dans "rescue_gen_file=".


Remarque : De part la structure de l'algorithme, il est possible de renseigner "operations=GENERAL_GEN_FIND,SIMULATION" avec "rescue_strategy=GEN_FIND". Dans ce cas, l'obtention d'une stratégie génétique retournera un chromosome au sein de "rescue_gen_file=" qui sera ensuite utilisé pour la simulation.


Selon les operations à réaliser choisies, l'utilisateur devra également renseigner certains autres paramètres dans la section spécialisées de "Config.txt". Par défaut, des paramètres sont déjà remplis afin de faciliter la prise en main initiale. De plus, certains paramètres de Config.txt étant considéré comme non vital au bon fonctionnement de l'algorithme, des valeurs par défaut leur ont été renseignée dans "codecpp/Constantes.cpp". L'utilisateur peut choisir de modifier ces valeurs par défaut, en veillant, dans un tel cas, à respecter les règles de typographies en vigueur dans "Config.txt".



=========
2ème etape : Lancement de l'algorithme via le terminal
=========

Une fois le fichier "Config.txt" rempli, l'utilisateur doit se rendre dans le terminal, se placer dans le dossier du fichier du projet, et écrire "make run" dans le terminal. Des instructions sont alors données pour savoir si l'utilisateur souhaite écraser les fichiers existants par ceux de la nouvelle simulation : N=No, le programme s'arrête laissant la possibilité de changer le chemin d'accès ; Y=Yes, le fichier originel est écrasé ; A=All, le fichier originel est écrasé et la vérification ne sera plus demandée (Attention donc à ne pas écraser accidentellement des résultats ou des chromosomes de cette façon).



=========
3ème etape : Visualisation des résultats
=========

Pour l'affichage des différentes données, l'utilisateur doit simplement écrire "make plot" dans le terminal, et choisir ce qu'il veut afficher (Il va de soit que la simulation en question doit avoir été compilée avant, à défaut de quoi le programme informera l'utilisateur de l'absence de données)

La commande plot compile et exécute le fichier plot_menu.cpp, qui demande à l'utilisateur un chiffre correspondant à un certain affichage, et exécute un programme python correspondant, prenant en entrée les données stockées dans output_plot, et affiche le graphe correspondant.


Remarque :
La fonction "Genetic stats visualization" du 'make plot' se base sur le programme "plotting/visualize_gen_stats.py". Ce programme utiliser un chemin d'accès encodé en dur. Afin de réaliser cette opération sur le fichier souhaité, il est nécessaire de vérifier que le chemin d'accès dans le programme python soit bien celui retourné à l'adresse "gen_log_file=".

=========
Assistance 
=========

L'utilisateur peut aussi entrer "make help" dans le terminal afin d'obtenir les différentes commandes possibles.


=========
Remarque autre
=========

IMPORTANT:
Sur les ordinateurs du Magistère, certaines permissions peuvent ne pas être accordées à la compilation de certains fichiers.

Si cela arrive, il suffit d'entrer "chmod +x nom_du_fichier.cpp" dans le terminal, puis "make run" ou "make plot", selon le fichier concerné, afin d'accorder les autorisations nécessaires.


Dans une démarche plus poussée, l'utilisateur peut aussi changer les paramètres généraux, sous la limite "#####", tel que le nombre de gènes ou de chromosomes de l'algorithme génétique, ou plus simplement la surface, la corde ou la masse de l'avion.

=========
Lexique
=========
L'ensemble des valeurs possibles pour une variables est indique entre {}, ces derniers ne devant pas apparaitre dans le Config.txt.
De plus, à l'exeception d'operations, une seule valeur, parmi celle possible, peut être renseigné à la fois. Pour les valeurs n'étant pas des string, seul le type est indiqué entre {} (Exemple, dans "Config.txt", on renseignera g=9.81, ce que nous affichons ici par g={double})


operations= liste des opérations à réaliser par l'algorithme {GENERAL_GEN_FIND, SIMULATION, ENERGIE, RUN_BATCH, TEST_INITIALISATION, COMPARE_RESCUE_STRATEGIES, MIN_RESCUE_ALTITUDE, PHUGOID} (possibilité d'en avoir plusieurs séparés par des , sans espace) 


enable_rescue_system= Sytème de sauvetage actif ou non  {true,false}  
rescue_strategy= Stratégie de sauvetage choisie si activée {THRUST_FIRST,PROFILE_FIRST,SIMULTANEOUS,GEN_FIND,GEN_GIVE}



######################## Paramètres généraux

#Paramètres physiques
g= gravité  {double}  \\
z_t= distance entre le centre de gravité et les moteurs {double}

#Paramètres avion
surface= surface alaire de l'avion {double}
corde= corde moyenne de l'avion {double}
masse= masse de l'avion {double}
vx_ini= vitesse horizontale initiale {double}
z_ini= altitude initiale {double}

#Paramètres généraux de simulation
dt= pas de temps de la simulation {double}
duree= durée maximale de la simulation {double} (maximale si non interrompu par sauvetage ou crash)
methode_integration= méthode d'intégration pour la simulation {RK4,EULER}
useHysteresis= utilisation du modèle d'histérésis pour le coefficient aérodynamique (=true) ou du modèle linéaire (=falese) {false, true}
cmd_profondeur= profondeur de la commande de l'avion {double}
cmd_thrust= poussée de l'avion {double}


cmd_start= début commande nose_up {double}
cmd_end= fin commande nose_up {double}

#Seuil critique sauvetage
seuil_altitude_critique= altitude critique en-dessous de laquelle le sauvetage peut se déclencher (si autre conditions remplies) {double}
seuil_descente_critique= angle de descente critique en-dessous de laquelle le sauvetage peut se déclencher (si autre conditions remplies) {double}
seuil_pitch_critique= pitch critique en-dessous de laquelle le sauvetage peut se déclencher (si autre conditions remplies) {double}

#Paramètres d'affichage 
quiet_optimizer_logs= affichage de messages au cours des simulations {false,true}

#Gestion des fichiers
#Fichiers liés à GEN_GIVE et GENERAL_GEN_FIND
#Si GEN_GIVE = on donne un strategie genetique deja construite, on doit donc fournir le fichier d'entrée
rescue_gen_file= chemin d'accès pointant vers le meilleur chromosome (utilisé pour la sauvegarde du chromosome après GENERAL_GEN_FIND ou pour l'utilisation d'un chromosome dans le cas de GEN_GIVE) {chemin d'accès vers fichier .txt}
gen_log_file= historique de paramètres d'intérêts pour chaque chromosomes de chaque simulation (permettant de visualiser l'évolution de l'algorithme génétique grâce à "make plot") {chemin d'accès vers fichier .txt}

#Fichiers liés à simulation
output_file= fichier contenant les résultats de la simulation {chemin d'accès vers fichier .csv}
config_file= fichier contenant les paramètres de la simulation {chemin d'accès vers fichier .txt}


#################### Paramètres spécialisés ci-dessous

#Batch_runner
duree_batch= durée de la simulation en batck {double}
p_min= profondeur maximale {double}
p_max= profondeur minimale {double}
p_step= pas de profondeur {double}
t_min= poussé minimale {double}
t_max= poussée maximale {double}
t_step= pas de la poussée {double}


#MIN_RESCUE_ALTITUDE
min_rescue_altitude_min= altitude minimale de test du décrochage {double}
min_rescue_altitude_max= altitude maximale de test du décrochage {double}
min_rescue_altitude_step= pas d'altitude pour le test du décrochage {double}


#Genetic algorithm search parameter
gen_nbr_generation= nombre de générations {int}
gen_nbr_chr= nombre de chromosome (=d'avions) pour chaque générations {int}
gen_nbr_chr_kept= nombre de meilleur chromosome à garder afin de créer la génération suivante {int}
gen_mutationRate_times100= taux de mutations (=% de gènes d'un chromosomes impactés par des mutations) {int}
