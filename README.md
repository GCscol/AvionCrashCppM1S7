# AvionCrashCppM1S7
L'avion vole puis décroche. Jusqu'à quand est il sauvable ?

Chose à faire :
- Vérifier la validité des moments où l'on fixe des bornes et si la valeur dépasse on applatit ou on force la valeur
    => En lien avec une meilleure gestion des cas limites ( Je pense qu'on pourrait faire sauter ces cobnditions pour le moment et voir à quel point c'est problématique afin d'aider à débuger notre code.
- Peut etre encore alléger avion en décallant certaines formules de calcule dans d'autres classes
- Mettre toutes les constantes de la simulation dans un fichier (genre dt dans fichier constant.h ou un nouveau fichier param.h) afin de juste modifier ce fichier et nom le main (voir modifier pour pouvoir l'injecter directement dans le fichier executable au moment de l'execution)
- Validité conservation ?
