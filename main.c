#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "parserJson.h"
#include "interfaceFreebox.h"
#include "constantes.h"
#include "gestion.h"
#include "affichage.h"

int main(int argc, char *argv[])
{
	TCPsocket sock;
	SDL_Surface *ecran = NULL;

	/* initialize SDL */
	if(SDL_Init(0)==-1)
	{
		printf("SDL_Init: %s\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	/* initialize SDL_net */
	if(SDLNet_Init()==-1)
	{
		printf("SDLNet_Init: %s\n",SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

    // Initialisation de la bibliothèque de l'affichage texte
    TTF_Init();

    // Ouverture de la fenêtre SDL
    // Mise en place de l'icône
    //SDL_WM_SetIcon(IMG_Load("images.jpg"), NULL);

    // Définition de l'écran
    ecran = SDL_SetVideoMode(TAILLE_ECRAN_LARGEUR, TAILLE_ECRAN_HAUTEUR, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption("Affichage Etat Freebox", NULL);

    // Récupération des valeurs de l'adresse IP et du port
    char *adresseIP = NULL;
    char *port = NULL;

    recupDonneesIdentifiants("ADRESSE_IP", &adresseIP);
    recupDonneesIdentifiants("PORT", &port);

	ouvrirSocket(&sock, adresseIP, port);

    free(adresseIP);
    free(port);

    // On demande une ouverture de session, si ça échoue, ça demande une ouverture et une validation, possible qu'en local
    if (!validerAuthentification(sock))
    {
        printf("Authentification du client\n");
        authentifierClient(sock);
    }

    printf("Ouverture de session\n");
    char *sessionToken = NULL, *permissions = NULL; // Récupération des données de la session
    ouvrirSession(sock, &sessionToken, &permissions);

    // Gestion de l'écran et des actions
    affichage(ecran, sessionToken, permissions, sock);

    // Se déconnecter
    objetJson *deconnexion = malloc(sizeof(objetJson));
    deconnexion->premier = NULL;
    requeteDeconnexion(sock, deconnexion, sessionToken);
    lectureparsingJson(deconnexion, "");

    libererParsingJson(deconnexion);

    free(sessionToken);
    free(permissions);

    fermerSocket(&sock);

	/* shutdown SDL_net */
	SDLNet_Quit();

	/* shutdown SDL */
	SDL_Quit();
    TTF_Quit();

    return EXIT_SUCCESS;
}

