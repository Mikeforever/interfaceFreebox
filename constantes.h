#ifndef CONSTANTES
#define CONSTANTES

#define LIGNE_HTTP "HTTP/1.1"
#define LIGNE_SERVEUR "Server:"
#define LIGNE_DATE "Date:"
#define LIGNE_CONTENT_TYPE "Content-Type:"
#define CONTENU_TYPE_JSON "application/json"

#define FICHIER_IDENTIFIANTS "identifiants.txt"
#define FICHIER_TOKEN "token.txt"

#define TEMPS_AFFICHAGE 500

#define TAILLE_ECRAN_LARGEUR 1024
#define TAILLE_ECRAN_HAUTEUR 768

#define TAILLE_AFFICHAGE_TEXTE 30
#define TAILLE_AFFICHAGE_LISTE 15
#define TAILLE_HAUTEUR_LISTE TAILLE_SPECTRE_HAUTEUR
#define FONT_POLICE "arial.ttf"
#define TAILLE_POLICE 25
#define TAILLE_PETITE_POLICE 10

typedef struct positionSouris positionSouris;
struct positionSouris
{
    int x, y;
};

#endif // CONSTANTES
