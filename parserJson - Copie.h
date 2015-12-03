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

typedef struct eltTableau eltTableau;
struct eltTableau
{
    void *valeur;
    TYPEJSON type;
    eltTableau *suivant;
};

typedef struct listeTableau listeTableau;
struct listeTableau
{
    eltTableau *premier;
};

void viderEspace(const char *chaine, int *position);
void avancerJusqua(const char *chaine, int *position, char separateur);
void separerTexte(const char *chaine, int position, char separateur, char *resultat);
void filtrageJson(const char *message, char *resultat);
int comptageTailleSeparateur(const char *chaine, int position, char separateur);
void parseurJson(const char *message, int *position, objetJson *contenu);
void parsingTableauJson(char *message, int *position, listeTableau *tableau);
void lectureparsingJson(objetJson *contenu, char *decalage);
void libererParsingJson(objetJson *liste);
void copierTexteDeplacement(const char *chaine, int *position, const char separateur, char **resultat);
void parsingValeurJson(char *message, int *position, void **valeur, TYPEJSON* typeJson);
void lectureparsingTableauJson(listeTableau *contenu, char *decalage);
void libererParsingTableauJson(listeTableau *tableau);

#endif // PARSERJSON
