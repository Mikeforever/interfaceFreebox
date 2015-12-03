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

    // Initialisation de la biblioth�que de l'affichage texte
    TTF_Init();

    // Ouverture de la fen�tre SDL
    // Mise en place de l'ic�ne
    //SDL_WM_SetIcon(IMG_Load("images.jpg"), NULL);

    // D�finition de l'�cran
    ecran = SDL_SetVideoMode(TAILLE_ECRAN_LARGEUR, TAILLE_ECRAN_HAUTEUR, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption("Affichage Etat Freebox", NULL);

    // R�cup�ration des valeurs de l'adresse IP et du port
    char *adresseIP = NULL;
    char *port = NULL;

    recupDonneesIdentifiants("ADRESSE_IP", &adresseIP);
    recupDonneesIdentifiants("PORT", &port);

	ouvrirSocket(&sock, adresseIP, port);

    free(adresseIP);
    free(port);

    // On demande une ouverture de session, si �a �choue, �a demande une ouverture et une validation, possible qu'en local
    if (!validerAuthentification(sock))
    {
        printf("Authentification du client\n");
        authentifierClient(sock);
    }

    printf("Ouverture de session\n");
    char *sessionToken = NULL, *permissions = NULL; // R�cup�ration des donn�es de la session
    ouvrirSession(sock, &sessionToken, &permissions);

    // Gestion de l'�cran et des actions
    affichage(ecran, sessionToken, permissions, sock);

    // Se d�connecter
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

