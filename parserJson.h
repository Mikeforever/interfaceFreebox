#ifndef PARSERJSON
#define PARSERJSON

typedef enum TYPEJSON TYPEJSON;
enum TYPEJSON {CHAINE, NOMBRE, OBJET, TABLEAU, AUTRE};

typedef struct paireJson paireJson;
struct paireJson
{
    char *cle;
    TYPEJSON type;
    void *valeur;
    paireJson *suivant;
};

typedef struct objetJson objetJson;
struct objetJson
{
    paireJson *premier;
};

void viderEspace(const char *chaine, int *position);
void avancerJusqua(const char *chaine, int *position, char separateur);
void separerTexte(const char *chaine, int position, char separateur, char *resultat);
void filtrageJson(const char *message, char *resultat);
int comptageTailleSeparateur(const char *chaine, int position, char separateur);
int parseurJson(const char *message, int *position, objetJson *contenu, TYPEJSON type);
void lectureparsingJson(objetJson *contenu, char *decalage);
void libererParsingJson(objetJson *liste);
void copierTexteDeplacement(const char *chaine, int *position, const char separateur, char **resultat);
void parsingValeurJson(char *message, int *position, void **valeur, TYPEJSON* typeJson);
void generationTexteJson(objetJson *contenu, char **reponse);
void rechercheValeur(objetJson *contenu, char *cle, void **valeur);
int recuperationJsonFichier(objetJson *contenu, char *nomFichier);

#endif // PARSERJSON
