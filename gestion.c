#include <stdio.h>
#include <stdlib.h>
#include <SDL_net.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

#include "gestion.h"
#include "constantes.h"
#include "parserJson.h"
#include "interfaceFreebox.h"


void authentifierClient (TCPsocket sock)
{
    // Demande de l'app token � la Freebox
    objetJson *retourJson = malloc(sizeof(objetJson));
    retourJson->premier = NULL;
    requeteAutorisation(sock, retourJson);

    char *succes = NULL;
    rechercheValeur(retourJson, "success", &succes);

    if (!strcmp(succes, "true"))
    {
        // On r�cup�re l'objet en 2� �l�ment seulement
        objetJson *result = NULL;
        rechercheValeur(retourJson, "result", &result);

        // Stockage de ces informations dans un fichier
        FILE* fichier = NULL;
        fichier = fopen(FICHIER_TOKEN, "w");

        if (fichier != NULL)
        {
            char *texteJson = NULL;

            int position = 0;
            generationTexteJson(result, &texteJson);

            while(texteJson[position] != '\0')
            {
                fputc(texteJson[position], fichier);
                position++;
            }

            // Fermeture du fichier
            fclose(fichier);
            free(texteJson);
        }
    }

    libererParsingJson(retourJson);
}

int validerAuthentification(TCPsocket sock)
{
    // R�cup�ration du trackid dans le fichier s'il existe
    objetJson *infoJson = malloc(sizeof(objetJson));
    infoJson->premier = NULL;
    int resultat = 0;

    if (recuperationJsonFichier(infoJson, FICHIER_TOKEN))
    {
        char *trackid = "";
        int id = 0;

        rechercheValeur(infoJson, "track_id", &trackid);

        objetJson *reponse = malloc(sizeof(objetJson));
        reponse->premier = NULL;
        requeteVerification(sock, reponse, trackid);

        char *succes = NULL;
        rechercheValeur(reponse, "success", &succes);

        if (!strcmp(succes, "true"))
        {
            objetJson *result = NULL;
            rechercheValeur(reponse, "result", &result);

            // On recherche le statut uniquement
            char *statut = NULL;
            rechercheValeur(result, "status", &statut);

            if (!strcmp(statut, "granted"))
                resultat = 1;
        }

        libererParsingJson(reponse);
    }
    else
    {
        printf("Erreur dans la validation\n");
    }

    libererParsingJson(infoJson);

    return resultat;
}

void ouvrirSession(TCPsocket sock, char **sessionToken, char **permissions)
{

    // Ouvrir une session
    objetJson *reponseJson = malloc(sizeof(objetJson));
    reponseJson->premier = NULL;
    char *succes = NULL;
    requeteLogin(sock, reponseJson);

    // R�cup�ration du challenge si non d�j� authentifi� (true ou false)
    rechercheValeur(reponseJson, "success", &succes);

    if (!strcmp(succes, "true"))
    {
        // On r�cup�re le r�sultat
        objetJson *resultat = NULL;
        char *login = NULL;

        rechercheValeur(reponseJson, "result", &resultat);
        rechercheValeur(resultat, "logged_in", &login);

        if (!strcmp(login, "false")) // Uniquement dans le cas o� une session n'est pas ouverte
        {
            char *challenge = NULL;

            rechercheValeur(resultat, "challenge", &challenge);

            // Constitution de la r�ponse pour ouverture de la session
            // password = hmac-sha1(app_token, challenge)

            // R�cup�ration du token
            objetJson *infoJson = malloc(sizeof(objetJson));
            infoJson->premier = NULL;
            if (recuperationJsonFichier(infoJson, FICHIER_TOKEN))
            {
                char *token = NULL;
                char password[1024] = "";
                int longueur = 0;
                rechercheValeur(infoJson, "app_token", &token);

                //On nettoie les chaines du caract�re \ antislash qui g�nent
                SupprimerCaractere(token, '\\');
                SupprimerCaractere(challenge, '\\');

                // G�n�ration du HMAC qui va bien
                HMAC(EVP_sha1(), token, strlen(token) * sizeof(char), challenge, strlen(challenge), password, &longueur);

                // Envoyer le password
                char *affichage = pt(password, longueur); // R�cup�ration de la repr�sentation hexadecimal

                objetJson *reponseSession = malloc(sizeof(objetJson));
                reponseSession->premier = NULL;
                requeteSession(sock, reponseSession, affichage);

                // Il est n�cessaire de r�cup�rer le session_token en cas de succ�s en faisant une copie
                char *succesSession = NULL;
                rechercheValeur(reponseSession, "success", &succesSession);

                if (!strcmp(succesSession, "true"))
                {
                    // R�cup�ration de la session_token
                    objetJson *result = NULL;
                    rechercheValeur(reponseSession, "result", &result);

                    char *sesstoken = NULL;
                    rechercheValeur(result, "session_token", &sesstoken);

                    *sessionToken = calloc(strlen(sesstoken), sizeof(char));
                    strcpy(*sessionToken, sesstoken);

                    // R�cup�ration des permissions
                    objetJson *perm = NULL;
                    rechercheValeur(result, "permissions", &perm);

                    *permissions = NULL;
                    generationTexteJson(perm, permissions); // Attention, il faut commencer � mettre NULL dans un premier temps
                }

                libererParsingJson(reponseSession);
            }
        }
    }

    libererParsingJson(reponseJson);
}

// Affichage de l'hexad�cimal du r�sultat en d�cimal
static char *pt(unsigned char *md, unsigned int len)
{
    unsigned int i;
    static char buf[80];

    for (i = 0; i < len; i++)
        sprintf(&(buf[i * 2]), "%02x", md[i]);
    return (buf);
}

void SupprimerCaractere(char* str, char c) //Enleve tous les c de str
{
   int id_read, id_write;
   id_read = 0;
   id_write = 0;

   while (str[id_read] != '\0')
   {
      if (str[id_read] != c)
      {
          str[id_write] = str[id_read];
          id_write++;
      }
      id_read++;
    }
    str[id_write] = '\0';
}

void recupDonneesIdentifiants(char *id, char **valeur)
{
    FILE *fichier = NULL;
    fichier  = fopen(FICHIER_IDENTIFIANTS, "r");

    if (fichier != NULL)
    {
        // On r�cup�re le contenu du fichier
        char contenuFichier[1024] = "";
        int position = 0;
        char caractereActuel = 0;
        char mot[1024] = "";

        do
        {
            caractereActuel = fgetc(fichier);

            contenuFichier[position] = caractereActuel;

            position++;
        } while ((caractereActuel != EOF) && (position < 1023));

        contenuFichier[position] = '\0';

        // Fermeture du fichier
        fclose(fichier);

        // On rep�re la chaine de caract�res pass�e en param�tres
        position = 0;

        // Tant que le mot ne correspond pas
        while (strcmp(mot, id))
        {
            // On d�place le curseur jusqu'au prochain mot sauf pour le premier mot
            while ((contenuFichier[position] != '\n') && (position != 0) && (contenuFichier[position] != '\0'))
                position++;

            // Si on est sur un saut de ligne, on avance d'un pas
            if (contenuFichier[position] == '\n')
                position++;

            int debut = position;

            // On remplit le mot
            while ((contenuFichier[position] != '=') && (contenuFichier[position] != '\0'))
            {
                mot[position - debut] = contenuFichier[position];
                position++;
            }

            // On finit le mot
            mot[position - debut] = '\0';
        }

        // Si le mot est non vide, on r�cup�re le r�sultat entre guillemets
        if (strcmp(mot, ""))
        {
            *valeur = malloc(1024*sizeof(char));

            // On se positionne apr�s le premier guillemet
            position+=2;
            int debut = position;

            while ((contenuFichier[position] != '"') && (contenuFichier[position] != '\0') && (contenuFichier[position] != '\n'))
            {
                (*valeur)[position - debut] = contenuFichier[position];
                position++;
            }

            // On finit la chaine de caract�res
            (*valeur)[position - debut] = '\0';
        }
    }
}
