


L'avion vole puis décroche. Jusqu'à quand est il sauvable ?


Ce code a pour objectif d'étudier le sauvetage d'un avion en décrochage en haute altitude. À ce titre, il possède plusieurs fonctionnalités. Ce document a pour objectif d'aider l'utilisateur dans la manipulation du code.


La première chose que doit faire l'utilisateur est de choisir l'utilisation qu'il cherche à en faire.


Pour ce faire, un document "Config.txt" est mis à disposition.

En l'ouvrant, l'utilisateur peut choisir dans la partie à modifier (en haut du document, au dessus des "###", en dessous de "#Paramètres de simulation"), une action (rappelées à la 1ère ligne à "# Operations to perform") parmi:
GENERAL_GEN_FIND
SIMULATION
ENERGIE
RUN_BATCH
TEST_INITIALISATION
COMPARE_RESCUE_STRATEGIES
MIN_RESCUE_ALTITUDE
MIN_RESCUE_ALT_OPT
PHUGOID

Ensuite, écrire juste en dessous: operations=CHOIX_EFFECTUE (Par exemple: operations=COMPARE_RESCUE_STRATEGIES).





Si l'utilisateur choisit l'option GENERAL_GEN_FIND, il doit également choisir une stratégie (Rescue Strategy) parmi:

THRUST_FIRST
PROFILE_FIRST
SIMULTANEOUS
GEN_FIND
GEN_GIVE

Puis, écrire simplement rescue_strategy=STRATEGIE_CHOISIE (par exemple: rescue_strategy=GEN_GIVE).

L'utilisateur doit faire attention à ne mettre aucun espace lorsqu'il remplit ces données 
(exemple rescue_strategy= GEN_GIVE ne fonctionnera pas).

(Le fichier "Config.txt est prérempli à titre d'exemple, il ne reste à l'utilisateur qu'à modifier les valeurs en majuscule)



Une fois le fichier "Config.txt" rempli, l'utilisateur doit se rendre dans le terminal, se placer dans le dossier du fichier du projet, et écrire "make run" dans le terminal. Des instructions sont alors données pour savoir si l'utilisateur souhaite écraser les fichiers existants sur ceux de la nouvelle simulation.


Pour l'affichage des différentes données, l'utilisateur doit simplement écrire "make plot" dans le terminal, et choisir ce qu'il veut afficher (Il va de soit que la simulation en question doit avoir été compilée avant, à défaut de quoi le programme informera l'utilisateur de l'absence de données)

La commande plot compile et exécute le fichier plot_menu.cpp, qui demande à l'utilisateur un chiffre correspondant à un certain affichage, et exécute un programme python correspondant, prenant en entrée les données stockées dans output_plot, et affiche le graphe correspondant.


L'utilisateur peut aussi entrer "make help" dans le terminal afin d'obtenir les différentes commandes possibles.

IMPORTANT:
Sur les ordinateurs du Magistère, certaines permissions peuvent ne pas être accordées à la compilation de certains fichiers.

Si cela arrive, il suffit d'entrer "chmod +x nom_du_fichier.cpp" dans le terminal, puis "make run" ou "make plot", selon le fichier concerné, afin d'accorder les autorisations nécessaires.
