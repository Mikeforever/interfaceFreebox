#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parserJson.h"
#include "constantes.h"

// Supprime tout type d'espace
void viderEspace(const char *chaine, int *position)
{
    while ((chaine[*position] == ' ') || (chaine[*position] == '\n') || (chaine[*position] == '\r')|| (chaine[*position] == '\t'))
        (*position)++;
}

// D�placement jusqu'au s�parateur de la position dans la chaine
void avancerJusqua(const char *chaine, int *position, char separateur)
{
    // On avance jusqu'au prochain s�parateur ou jusqu'� la fin de la chaine
    while ((chaine[*position] != separateur) && (chaine[*position] != '\0'))
        (*position)++;
}

// R�cup�ration du texte dans la variable r�sultat
void separerTexte(const char *chaine, int position, char separateur, char *resultat)
{
    // Il est pr�suppos� que resultat est d�j� allou�
    int i = position;
    while ((chaine[i] != separateur) && (chaine[i] != '\0'))
    {
        resultat[i - position] = chaine[i];
        i++;
    }
    resultat[i-position] = '\0'; // On ferme la chaine de caract�res
}

// Compte le nombre de caract�res pour aller jusqu'au s�parateur
int comptageTailleSeparateur(const char *chaine, const int position, const char separateur)
{
    int i = position;
    while ((chaine[i] != separateur) && (chaine[i] != '\0'))
    {
        i++;
    }

    return (i - position);
}

// R�cup�re le r�sultat et se d�place jusqu'au s�parateur
void copierTexteDeplacement(const char *chaine, int *position, const char separateur, char **resultat)
{
    *resultat = calloc(comptageTailleSeparateur(chaine, *position, separateur)+1, sizeof(char));
    separerTexte(chaine, *position, separateur, *resultat);

    // On se d�place jusqu'� l'�l�ment s�parateur suivant
    avancerJusqua(chaine, position, separateur);
}

// R�cup�ration du message Json
void filtrageJson(const char *message, char *resultat)
{
    int position = 0, i = 0;
    int compteAccolage = 0;

    // On se d�place jusqu'au premier s�parateur
    avancerJusqua(message, &position, '{');
    resultat[i] = message[position];
    compteAccolage++;
    i++;

    // On va compter les accolades, car il faut aller jusqu'� la derni�re
    while (compteAccolage != 0)
    {
        resultat[i] = message[position + i];

        if (message[position+i] == '{')
            compteAccolage++;

        if (message[position+i] == '}')
            compteAccolage--;

        i++;
    }

    // On termine la chaine de caract�res
    resultat[i] = '\0';
}

// On parcourt et on alimente la liste chain�e du Json
int parseurJson(const char *message, int *position, objetJson *contenu, TYPEJSON type)
{
    // On commence par supprimer les caract�res qui ne servent � rien
    viderEspace(message, position);

    // On identifie le caract�re suivant qui d�termine le type
    if ((message[*position] != '{') && (message[*position] != '[') && ((message[*position] != ',' ) || (contenu->premier == NULL)))
    {
        printf("Erreur sur le parsing de la chaine de caracteres\n");
        return 0;
    }

    // On commence la paire Json
    paireJson *elt = calloc(1, sizeof(paireJson));

    (*position)++; // On avance d'un cran et on supprime les espaces et sauts de ligne
    viderEspace(message, position);

    if (type == OBJET)
    {
        //On r�cup�re la cl�
        // On doit trouver une chaine de caract�re
        if (message[*position] != '"') // Cas anormal
        {
            printf("Erreur dans le parsing dans la cl� chaine de caracteres\n");
            exit(EXIT_FAILURE);
        }

        (*position)++; // On passe le caract�re "
        copierTexteDeplacement(message, position, '"', &(elt->cle)); // R�cup�ration de la cl�
        (*position)++; // On passe le caract�re "
        avancerJusqua(message, position, ':'); // S�parateur entre la cl� et la valeur
        (*position)++;
        viderEspace(message, position);
    }
    else if (type == TABLEAU)
    {
        elt->cle = NULL;
    }
    else
    {
        printf("Erreur de typage � l'appel de fonction\n");
        exit(EXIT_FAILURE);
    }

    // Il est n�cessaire d'identifier dans quel cas on est pour la valeur
    parsingValeurJson(message, position, &(elt->valeur), &(elt->type));

    // On ajoute l'�l�ment dans la liste chain�e � la fin
    // Parcours de la liste chaine
    elt->suivant = NULL; // On fixe � NULL le suivant car dernier �l�ment
    paireJson *eltParcours = contenu->premier;
    if (eltParcours != NULL)
    {
        while(eltParcours->suivant != NULL)
        {
            eltParcours = eltParcours->suivant;
        }
        eltParcours->suivant = elt;
    }
    else
    {
        contenu->premier = elt;
    }

    // Il faut continuer avec l'�l�ment suivant s'il y en a un (soit une virgule, soit une accolade)
    viderEspace(message, position);

    if (message[*position] == ',')
        parseurJson(message, position, contenu, type);
    else if ((message[*position] != '}') && (message[*position] != ']'))
    {
        printf("Erreur dans la fin du json\n");
        exit(EXIT_FAILURE);
    }
    else
        (*position)++;

    return 1;
}

void parsingValeurJson(char *message, int *position, void **valeur, TYPEJSON *typeJson)
{
    switch (message[*position])
    {
    case '"': // Cas d'une chaine de caract�re
        // On avance d'un pas
        (*position)++;
        *typeJson = CHAINE;
        copierTexteDeplacement(message, position, '"', valeur);
        (*position)++;
        break;
    case '{': // Cas d'un objet json
        *typeJson = OBJET;
        // Cr�ation d'un nouvel objet Json
        *valeur = (objetJson *)calloc(1, sizeof(objetJson));
        ((objetJson *)*valeur)->premier = NULL;

        parseurJson(message, position, *valeur, OBJET); // On reboucle sur l'int�rieur de la valeur json
        break;
    case '[': // Cas d'un tableau, on cr�e un objet de type tableau avec le premier �l�ment
        *typeJson = TABLEAU;
        *valeur = (objetJson *)calloc(1, sizeof(objetJson));
        ((objetJson *)*valeur)->premier = NULL;

        parseurJson(message, position, *valeur, TABLEAU);

        break;
    default: ;// Dans un autre cas, il doit s'agir d'un nombre
        // Soit il s'agit d'un nombre
        // Soit il s'agit de true ou false
        // On r�cup�re le contenu et on �value le type

        // Initialisation
        int separateurAuPlusPres = 0, separateurSuivant = 0;
        char eltSeparateur = 0;

        // Premier �l�ment
        separateurAuPlusPres = comptageTailleSeparateur(message, *position, ',');
        eltSeparateur = ',';

        // El�ments suivants
        separateurSuivant = comptageTailleSeparateur(message, *position, ' ');
        if (separateurSuivant < separateurAuPlusPres)
        {
            separateurAuPlusPres = separateurSuivant;
            eltSeparateur = ' ';
        }

        separateurSuivant = comptageTailleSeparateur(message, *position, '}');
        if (separateurSuivant < separateurAuPlusPres)
        {
            separateurAuPlusPres = separateurSuivant;
            eltSeparateur = '}';
        }

        separateurSuivant = comptageTailleSeparateur(message, *position, ']');
        if (separateurSuivant < separateurAuPlusPres)
        {
            separateurAuPlusPres = separateurSuivant;
            eltSeparateur = '}';
        }

        copierTexteDeplacement(message, position, eltSeparateur, valeur);

        // Soit true, soit false, soit un nombre
        if ((!strcmp(*valeur, "true")) || (!strcmp(*valeur, "false")))
            *typeJson = AUTRE;
        else
            *typeJson = NOMBRE; // A COMPLETER

        break;
    }
}

void lectureparsingJson(objetJson *contenu, char *decalage)
{
    paireJson *elt = contenu->premier;

    while (elt != NULL)
    {
        // Affichage de la cl� si existante
        if (elt->cle != NULL)
            printf("%s%s->", decalage, elt->cle);

        char *decalons = NULL;

        //Affichage du type
        switch(elt->type)
        {
        case CHAINE:
            printf("(CHAINE)");
            break;
        case NOMBRE:
            printf("(NOMBRE)");
            break;
        case AUTRE:
            printf("(AUTRE)");
            break;
        case OBJET:
            printf("%s(OBJET)", decalage);
            break;
        case TABLEAU:
            printf("%s(TABLEAU)", decalage);
            break;
        }

        // Affichage de la valeur
        switch (elt->type)
        {
        case CHAINE:
        case NOMBRE:
        case AUTRE:
            printf("%s\n", (char *)elt->valeur);
            break;
        case OBJET:
            printf("\n%s{\n", decalage);
            decalons = malloc((strlen(decalage) + 3)*sizeof(char));
            strcpy(decalons, decalage);
            decalons[strlen(decalage)] = ' ';
            decalons[strlen(decalage)+1] = ' ';
            decalons[strlen(decalage)+2] = '\0';
            lectureparsingJson(elt->valeur, decalons);
            printf("%s}\n", decalage);
            break;
        case TABLEAU:
            printf("\n%s[\n", decalage);
            decalons = malloc((strlen(decalage) + 3)*sizeof(char));
            strcpy(decalons, decalage);
            decalons[strlen(decalage)] = ' ';
            decalons[strlen(decalage)+1] = ' ';
            decalons[strlen(decalage)+2] = '\0';
            lectureparsingJson(elt->valeur, decalons);
            printf("%s]\n", decalage);
            break;
        default:
            // Ne doit pas arriver
            printf("Erreur dans la lecture\n");
            exit(EXIT_FAILURE);
            break;
        }

        elt = elt->suivant;
    }
}

void libererParsingJson(objetJson *liste)
{
    // Parcours de tous les objets
    paireJson *elt = liste->premier;

    while (elt != NULL)
    {
        paireJson *suivant = elt->suivant;
        free(elt->cle); // Lib�ration de la m�moire de la cl�
        // Lib�ration de la m�moire de la valeur en fonction des cas
        switch(elt->type)
        {
        case CHAINE:
        case NOMBRE:
        case AUTRE:
            free(elt->valeur);
            break;
        case OBJET:
        case TABLEAU:
            libererParsingJson(elt->valeur);
            break;
        default:
            printf("Erreur dans la lib�ration m�moire\n");
            break;
        }
        // Lib�ration de l'�l�ment de liste chain�e
        free(elt);

        // Passage � l'�l�ment suivant
        elt = suivant;
    }

    free(liste);
}

// Permet de g�n�rer le texte du Json r�cup�r�
void generationTexteJson(objetJson *contenu, char **reponse)
{
    // Initialisation, ne se reproduit pas lors des r�cursivit�s
    if (*reponse == NULL)
    {
        *reponse = malloc(2*sizeof(char));
        (*reponse)[0] = '{';
        (*reponse)[1] = '\0';
    }

    paireJson *elt = contenu->premier;

    while (elt != NULL)
    {
        // Affichage de la cl� si existante
        if (elt->cle != NULL)
        {
            realloc_s(reponse, strlen(*reponse) + strlen(elt->cle)+4);

            strcat(*reponse, "\"");
            strcat(*reponse, elt->cle);
            strcat(*reponse, "\":");
        }

        // Affichage de la valeur
        switch (elt->type)
        {
        case CHAINE:
            realloc_s(reponse, strlen(*reponse) + strlen((char *)elt->valeur)+3);
            strcat(*reponse, "\"");
            strcat(*reponse, (char *)elt->valeur);
            strcat(*reponse, "\"");
            break;
        case NOMBRE:
        case AUTRE:
            realloc_s(reponse, strlen(*reponse) + strlen((char *)elt->valeur)+1);
            strcat(*reponse, (char *)elt->valeur);
            break;
        case OBJET:
            realloc_s(reponse, strlen(*reponse) + 2);
            strcat(*reponse, "{");
            generationTexteJson(elt->valeur, reponse);
            break;
        case TABLEAU:
            realloc_s(reponse, strlen(*reponse) + 2);
            strcat(*reponse, "[");
            generationTexteJson(elt->valeur, reponse);
            break;
        default:
            // Ne doit pas arriver
            printf("Erreur dans la lecture\n");
            exit(EXIT_FAILURE);
            break;
        }
        if (elt->suivant != NULL)
        {
            realloc_s(reponse, strlen(*reponse) + 2);
            strcat(*reponse, ",");
        }
        else
        {
            if (elt->cle != NULL)
            {
                realloc_s(reponse, strlen(*reponse) + 2);
                strcat(*reponse, "}");
            }
            else
            {
                realloc_s(reponse, strlen(*reponse) + 2);
                strcat(*reponse, "]");
            }
        }

        elt = elt->suivant;
    }

    // On ajoute le dernier crochet
    realloc_s(reponse, strlen(*reponse) + 2);
    strcat(*reponse, "}");

    // On supprime le tout dernier caract�re qui est une virgule
    (*reponse)[strlen(*reponse)-1] = '\0';
}

// Attention, la recherche va permettre de pointer directement sur l'�l�ment sans en faire une copie
// A n'utiliser qu'en lecture
void rechercheValeur(objetJson *contenu, char *cle, void **valeur)
{
    paireJson *elt = contenu->premier;

    while ((elt!= NULL) && (strcmp(elt->cle, cle)))
    {
        elt = elt->suivant;
    }

    if (elt == NULL)
    {
        *valeur = NULL;
    }
    else
    {
        *valeur = elt->valeur;
    }
}

int recuperationJsonFichier(objetJson *contenu, char *nomFichier)
{
    FILE* fichier = NULL;
    fichier = fopen(nomFichier, "r");

    if (fichier == NULL)
    {
        return 0;
    }
    else
    {
        // On r�cup�re le contenu du fichier
        char contenuFichier[1024] = "";
        int position = 0;
        char caractereActuel = 0;

        do
        {
            caractereActuel = fgetc(fichier);

            contenuFichier[position] = caractereActuel;

            position++;
        } while ((caractereActuel != EOF) && (position < 1023));

        contenuFichier[position] = '\0';

        // On cr�e la chaine chain�e Json
        position = 0;

        // Fermeture du fichier avant de quitter la fonction
        fclose(fichier);

        return parseurJson(contenuFichier, &position, contenu, OBJET);
    }
}
