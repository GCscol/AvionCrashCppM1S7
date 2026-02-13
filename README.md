# AvionCrashCppM1S7
L'avion vole puis décroche. Jusqu'à quand est il sauvable ?

Chose à faire :
- Modifier intégration dans avion afin d'ajouter classe Intégration euler et RK4 afin d'alléger le code et de comparer les 2.
- Vérifier la validité des moments où l'on fixe des bornes et si la valeur dépasse on applatit ou on force la valeur
    => En lien avec une meilleure gestion des cas limites ( Je pense qu'on pourrait faire sauter ces cobnditions pour le moment et voir à quel point c'est problématique afin d'aider à débuger notre code.
- Peut etre encore alléger avion en décallant certaines formules de calcule dans d'autres classes
