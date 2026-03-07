# AvionCrashCppM1S7
L'avion vole puis décroche. Jusqu'à quand est il sauvable ?

Utilisation : (succintement) du programme cpp avion_simulation 
Lors de la compilation, on compile l'ensemble des fonctionnalités du code.
Le code obtenu peut alors être utilisé pour n'importe quelle opération possible sur n'importe quel avion dans n'importe quel condition initiale avec n'importe quel modèle et n'importe quel préset de comportement du pilote. Les spécificités des modèles aérodynamiques et de comportement du pilote ne peuvent cependant pas être modifié après la compilation.
Ainsi si l'on souhaite étudier un autre avion (qu'un type A330 ici considéré), il est nécessaire en plus de changer les caractéristiques de l'appareil, de prendre en compte les modications du fuselage par des modifications du modèle aérodynamique.

Une fois compiler ("make" dans le terminal grâce au makefile), le fichier executable peut être lancer ("./avion_simulatoion" dans le terminal)
Afin de changer les paramètres, on va directement dans config.txt. Au sein de ce fichier, certaines variables sont considérées comme obligatoires :
    - Nom du fichier de sortie,
    - Operations à réaliser
Pour les autres, des présets sont renseignés et en leur absence, la valeur par défaut sera attribué si nécessaire.
Après avoir charger le fichier config.txt au sein du struct Config grâce au main, le programme exportera un fichier Config_full_simu.txt" reprenant l'ensemble des paramètres utilisés en notant explicitement les paramètres manquants complétés avec les valeurs par défauts.

L'ensemble des opérations réalisable sur l"avion sont listées dans le config.txt. 
    Plusieurs opérations peuvent être choisit et sélectionnées dans le fichier config (attention, elles doivent être séparés par des virgules sans utilisation d'espace)
Le pilote peut tenter des procédures de sauvatege en activant le sauvetage (variable :)
    3 comportemennts ont été crés à titre d'exemple : THRUST_FIRST, SIMULTANEOUS, ...
Pour les modèles aérodynamiques, nous recommadnant le modèle linéaire plus pertinent (useHysteresis=false) que le modèle d'hystérésis.

Pour chaque opération, le programme exportera ces résultats sous format d'un fichier csv et/ou imprimera un message dans le terminal (exemple, comparaison méthode rescue)  (Penser à mettre cette opération à la fin du main pour garder le message visible dans le terminal !!!!!!!)

=======================================
Code python : 
Le code python "Courbes affichage.py" considère le fichier "simulation_full.csv", l'analyse par un DataFrame et affiche la trajectoire de l'avion, puis l'altitude, les différents angles, la portance et la traction en fonction du temps.






=======================


Chose à faire :
- Vérifier la validité des moments où l'on fixe des bornes et si la valeur dépasse on applatit ou on force la valeur
    => En lien avec une meilleure gestion des cas limites ( Je pense qu'on pourrait faire sauter ces cobnditions pour le moment et voir à quel point c'est problématique afin d'aider à débuger notre code.
- Peut etre encore alléger avion en décallant certaines formules de calcule dans d'autres classes

