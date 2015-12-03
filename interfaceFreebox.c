#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL_net.h>
#include <openssl/hmac.h>

#include "constantes.h"
#include "parserJson.h"
#include "interfaceFreebox.h"


void ouvrirSocket(TCPsocket *sock, char *hote, char *nombrePort)
{
    IPaddress ip;
    Uint16 port;
	Uint32 ipaddr;

	// Récupération du port à utiliser
	port=(Uint16) strtol(nombrePort, NULL, 0);

	// Récupération de l'adresse IP via DNS
	if(SDLNet_ResolveHost(&ip, hote, port)==-1)
	{
		printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	// Ouverture de la socket pour communiquer
	*sock=SDLNet_TCP_Open(&ip);
	if(!(*sock))
	{
		printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

    // Affichage pour info de l'adresse IP et du numéro de port récupéré
    ipaddr=SDL_SwapBE32(ip.host);
    printf("Connection to %d.%d.%d.%d port %hu\n",
        ipaddr>>24,
        (ipaddr>>16)&0xff,
        (ipaddr>>8)&0xff,
        ipaddr&0xff,
        ip.port);
}

void fermerSocket(TCPsocket *sock)
{
    SDLNet_TCP_Close(*sock);
}

void envoyerRequete(TCPsocket sock, const char *requete)
{
    int len, result;

    len=strlen(requete);
	if(len)
	{
		// Affichage du message à envoyer
		printf("Sending: %.*s\n", len, requete);

		result=SDLNet_TCP_Send(sock, requete, len); /* add 1 for the NULL */
		if(result<len)
			printf("SDLNet_TCP_Send: %s\n",SDLNet_GetError());
	}
}

void recevoirRequeteLigne(TCPsocket sock, char **reponse)
{
    *reponse = calloc(1, sizeof(char)); // Un seul caractère, celui de la fin de chaine
    **reponse = '\0'; // Chaine vide pour le départ
    char charRecu = '0';

    // récupération du flux retour
    while(charRecu != '\n')
    {
        SDLNet_TCP_Recv(sock, &charRecu,1);

        realloc_s(reponse, (strlen(*reponse) + 2 )*sizeof(char));
        (*reponse)[strlen(*reponse) + 1] = '\0';
        (*reponse)[strlen(*reponse)] = charRecu;
    }
}

int recevoirReponseJson(TCPsocket sock, char **reponse, int nbCaracteres)
{
    *reponse = calloc(nbCaracteres + 1, sizeof(char)); // Un seul caractère, celui de la fin de chaine
    (*reponse)[nbCaracteres] = '\0'; // Fin de chaine

    // récupération du flux retour
    return SDLNet_TCP_Recv(sock, *reponse, nbCaracteres);
}

void* realloc_s (void **ptr, size_t taille)
{
    void *ptr_realloc = realloc(*ptr, taille);

    if (ptr_realloc != NULL)
        *ptr = ptr_realloc;

    /* Même si ptr_realloc est nul, on ne vide pas la mémoire. On laisse l'initiative au programmeur. */
    return ptr_realloc;
}

void receptionReponse(TCPsocket sock, char **repHttp, char **serveur, char **date, char **contentType, char **reponse)
{
    char *reponseLigne = calloc(1, sizeof(char));
    char *reponseLignePrecedente = calloc(1, sizeof(char));
    int flagLigneEntete = 1;
    *reponse = calloc(1, sizeof(char));

    do
    {
        // On mémorise la ligne précédente

        free(reponseLignePrecedente);
        reponseLignePrecedente = calloc(strlen(reponseLigne) + 1, sizeof(char));
        strcpy(reponseLignePrecedente, reponseLigne);

        // On reçoit la ligne suivante
        free(reponseLigne);
        recevoirRequeteLigne(sock, &reponseLigne);

        printf("%s", reponseLigne);

        // Si on passe une ligne vide, on quitte la zone d'entête
        if (!strcmp(reponseLigne, "\r\n"))
        {
            flagLigneEntete = 0;
        }
        else
        {
            // On ne récupère que la réponse dans un premier temps
            if (flagLigneEntete)
            {
                // Gestion des entêtes
                // La structure étant la même de façon systématique, on parse après l'espace
                char *nomEntete = calloc(strlen(reponseLigne) + 1, sizeof(char));
                strcpy(nomEntete, reponseLigne);
                int parcoursLigne = 0;

                // On réduit la taille pour enlever les 2 caractères d'échappement
                nomEntete[strlen(nomEntete)-2] = '\0';

                // On va jusqu'au séparateur
                avancerJusqua(nomEntete, &parcoursLigne, ' ');

                // On arrête la chaine de caractères à ce niveau
                nomEntete[parcoursLigne] = '\0';

                // Comparaison et action
                if (!strcmp(nomEntete, LIGNE_HTTP))
                {
                    *repHttp = calloc(strlen(reponseLigne) - strlen(nomEntete) + 1, sizeof(char));
                    strcpy(*repHttp, &nomEntete[parcoursLigne+1]);
                }

                if (!strcmp(nomEntete, LIGNE_SERVEUR))
                {
                    *serveur = calloc(strlen(reponseLigne) - strlen(nomEntete) + 1, sizeof(char));
                    strcpy(*serveur, &nomEntete[parcoursLigne+1]);
                }

                if (!strcmp(nomEntete, LIGNE_DATE))
                {
                    *date = calloc(strlen(reponseLigne) - strlen(nomEntete) + 1, sizeof(char));
                    strcpy(*date, &nomEntete[parcoursLigne+1]);
                }

                if (!strcmp(nomEntete, LIGNE_CONTENT_TYPE))
                {
                    *contentType  = calloc(strlen(reponseLigne) - strlen(nomEntete) + 1, sizeof(char));
                    strcpy(*contentType, &nomEntete[parcoursLigne+1]);
                }

                free(nomEntete);
            }
            else
            {
                // En fonction du type de contenu, on recherche le premier délimiteur
                char *typeDuContenu = calloc(strlen(*contentType) + 1, sizeof(char));
                separerTexte(*contentType, 0, ';', typeDuContenu);

                if (!strcmp(typeDuContenu, CONTENU_TYPE_JSON)) // Dans ce cas, on va agréger tout le contenu dans la réponse
                {
                    // On récupère le nombre de caractères
                    int nbCaracteres = strtol(reponseLigne, NULL, 16);
                    int len; // Longueur récupérée

                    // Tant qu'il reste des caractères à récupérer, on les récupère
                    while (nbCaracteres)
                    {
                        free(reponseLigne);
                        len = recevoirReponseJson(sock, &reponseLigne, nbCaracteres);
                        printf("%s", reponseLigne);
                        printf("\nNombre caracteres : %d\nLongueur réponse : %d\n", nbCaracteres, len);
                        nbCaracteres-=len;

                        realloc_s(reponse, (strlen(*reponse) + strlen(reponseLigne) + 1) * sizeof(char));
                        strcat(*reponse, reponseLigne);
                    }

                }
            }
        }
    } while ((strcmp(reponseLigne, "\r\n")) || (strcmp(reponseLignePrecedente,"0\r\n")));

    // On libère la mémoire
    free(reponseLigne);
    free(reponseLignePrecedente);
}

void echangeReponseJson(TCPsocket sock, const char *message, objetJson *reponseJson)
{
    char *repHttp, *serveur, *date, *contentType, *reponse, *contenu;
	int position = 0;

    // Envoi de la requête au serveur préalablement initialisé
    envoyerRequete(sock, message);

    // Réception globale
    receptionReponse(sock, &repHttp, &serveur, &date, &contentType, &reponse);

    // Récupération du contenu Json
    contenu = calloc(strlen(reponse) + 1, sizeof(char));
    filtrageJson(reponse, contenu);

    // Parsing du contenu Json, alimentation de la liste chainée
    parseurJson(contenu, &position, reponseJson, OBJET);

    free(reponse);
    free(repHttp);
    free(serveur);
    free(date);
    free(contentType);
    free(contenu);
}

void requeteAutorisation(TCPsocket sock, objetJson *reponseJson)
{
    char requete[1024] = "";
    char *appId = NULL;
    char *appName = NULL;
    char *appVersion = NULL;
    char *deviceName = NULL;

    recupDonneesIdentifiants("APP_ID", &appId);
    recupDonneesIdentifiants("APP_NAME", &appName);
    recupDonneesIdentifiants("APP_VERSION", &appVersion);
    recupDonneesIdentifiants("DEVICE_NAME", &deviceName);
    sprintf(requete, "{\"app_id\": \"%s\",\"app_name\": \"%s\",\"app_version\": \"%s\",\"device_name\": \"%s\"}", appId, appName, appVersion, deviceName);
    generationRequetePOST(sock, "/api/v3/login/authorize", requete, reponseJson);

    free(appId);
    free(appName);
    free(appVersion);
    free(deviceName);
}

void requeteVerification(TCPsocket sock, objetJson *reponseJson, char *trackid)
{
    char requete[256] = "/api/v3/login/authorize/";
    strcat(requete, trackid);
    generationRequeteGET(sock, requete, reponseJson);
}

void requeteLogin(TCPsocket sock, objetJson *reponseJson)
{
    generationRequeteGET(sock, "/api/v3/login/", reponseJson);
}

void requeteSession(TCPsocket sock, objetJson *reponseJson, char *password)
{
    char requete[256] = "/api/v3/login/session/";
    char infoJson[256] = "";
    char *appId = NULL;

    recupDonneesIdentifiants("APP_ID", &appId);
    strcat(infoJson, "{\"app_id\": \"");
    strcat(infoJson, appId);
    strcat(infoJson, "\",\"password\": \"");
    strcat(infoJson, password);
    strcat(infoJson, "\"}");

    generationRequetePOST(sock, requete, infoJson, reponseJson);

    free(appId);
}

void requeteSessionAuthentifie(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequeteGETSession(sock, "/api/v3/login/session", reponseJson, sessionToken);
}

void requeteFermeture(TCPsocket sock, objetJson *reponseJson)
{
    generationRequetePOST(sock, "/api/v3/login/logout/", "", reponseJson);
}

void requeteFreeplug(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequeteGETSession(sock, "/api/v3/freeplug/", reponseJson, sessionToken);
}

void requeteConnexionStatus(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequeteGETSession(sock, "/api/v3/connection/", reponseJson, sessionToken);
}

void requeteDeconnexion(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequetePOSTSession(sock, "/api/v3/login/logout/", "", reponseJson, sessionToken);
}

void requeteLogAppels(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequeteGETSession(sock, "/api/v3/call/log/", reponseJson, sessionToken);
}

void requeteConnexionXdsl(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequeteGETSession(sock, "/api/v3/connection/xdsl/", reponseJson, sessionToken);
}

void requeteLANInterfaces(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequeteGETSession(sock, "/api/v3/lan/browser/interfaces/", reponseJson, sessionToken);
}

void requeteLANListeHotes(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequeteGETSession(sock, "/api/v3/lan/browser/pub/", reponseJson, sessionToken);
}

void requeteMajLCDConfigurationB0(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequetePUTSession(sock, "/api/v3/lcd/config/", "{\"brightness\":0}", reponseJson, sessionToken);
}

void requeteMajLCDConfigurationB50(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequetePUTSession(sock, "/api/v3/lcd/config/", "{\"brightness\":50}", reponseJson, sessionToken);
}

void requeteMajLCDConfigurationB100(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequetePUTSession(sock, "/api/v3/lcd/config/", "{\"brightness\":100}", reponseJson, sessionToken);
}

void requeteMajLCDConfigurationO90(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequetePUTSession(sock, "/api/v3/lcd/config/", "{\"orientation\":90, \"orientation_forced\":true}", reponseJson, sessionToken);
}

void requeteMajLCDConfigurationO45(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequetePUTSession(sock, "/api/v3/lcd/config/", "{\"orientation\":45, \"orientation_forced\":true}", reponseJson, sessionToken);
}

void requeteMajLCDConfigurationOReinit(TCPsocket sock, objetJson *reponseJson, char *sessionToken)
{
    generationRequetePUTSession(sock, "/api/v3/lcd/config/", "{\"orientation\":0, \"orientation_forced\":false}", reponseJson, sessionToken);
}

void generationRequeteGET(TCPsocket sock, char *entete, objetJson *reponseJson)
{
    char requete[1024] = "";
    char *adresseIP = NULL;

    recupDonneesIdentifiants("ADRESSE_IP", &adresseIP);
    sprintf(requete, "GET %s HTTP/1.1\nHost: %s\n\n", entete, adresseIP);

    echangeReponseJson(sock, requete, reponseJson);

    free(adresseIP);
}

void generationRequeteGETSession(TCPsocket sock, char *entete, objetJson *reponseJson, char *sessionToken)
{
    char requete[1024] = "";
    SupprimerCaractere(sessionToken, '\\');
    char *adresseIP = NULL;

    recupDonneesIdentifiants("ADRESSE_IP", &adresseIP);
    sprintf(requete, "GET %s HTTP/1.1\nHost: %s\nX-Fbx-App-Auth: %s\n\n", entete, adresseIP, sessionToken);

    echangeReponseJson(sock, requete, reponseJson);

    free(adresseIP);
}

void generationRequetePOST(TCPsocket sock, char *entete, char *corps, objetJson *reponseJson)
{
    char messageCompose[1024] = "";
    char *adresseIP = NULL;

    recupDonneesIdentifiants("ADRESSE_IP", &adresseIP);
    sprintf(messageCompose, "POST %s HTTP/1.1\nHost: %s\nContent-Type: x-www-form-urlencoded\nContent-Length: %d\n\n%s", entete, adresseIP, strlen(corps), corps); // Ajoute l'élément nécessaire pour la bonne exécution de la requête

    echangeReponseJson(sock, messageCompose, reponseJson);
    free(adresseIP);
}

void generationRequetePOSTSession(TCPsocket sock, char *entete, char *corps, objetJson *reponseJson, char *sessionToken)
{
    char messageCompose[1024] = "";
    SupprimerCaractere(sessionToken, '\\');
    char *adresseIP = NULL;

    recupDonneesIdentifiants("ADRESSE_IP", &adresseIP);
    sprintf(messageCompose, "POST %s HTTP/1.1\nHost: %s\nX-Fbx-App-Auth: %s\nContent-Type: x-www-form-urlencoded\nContent-Length: %d\n\n%s",entete, adresseIP, sessionToken, strlen(corps), corps); // Ajoute l'élément nécessaire pour la bonne exécution de la requête

    echangeReponseJson(sock, messageCompose, reponseJson);

    free(adresseIP);
}

void generationRequetePUTSession(TCPsocket sock, char *entete, char *corps, objetJson *reponseJson, char *sessionToken)
{
    char messageCompose[1024] = "";
    char *adresseIP = NULL;

    recupDonneesIdentifiants("ADRESSE_IP", &adresseIP);
    SupprimerCaractere(sessionToken, '\\');
    sprintf(messageCompose, "PUT %s HTTP/1.1\nHost: %s\nX-Fbx-App-Auth: %s\nContent-Type: x-www-form-urlencoded\nContent-Length: %d\n\n%s", entete, adresseIP, sessionToken, strlen(corps), corps); // Ajoute l'élément nécessaire pour la bonne exécution de la requête

    echangeReponseJson(sock, messageCompose, reponseJson);

    free(adresseIP);
}
