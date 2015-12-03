#include <stdio.h>
#include <stdlib.h>
#include <SDL_net.h>
#include <SDL_ttf.h>

#include "gestion.h"
#include "constantes.h"
#include "parserJson.h"
#include "affichage.h"

void affichage(SDL_Surface *ecran, objetJson *session, objetJson *permissions, TCPsocket sock)
{
    int tempsActuel = 0, tempsPrecedent = 0;
    int continuer = 1;
    SDL_Event event;
    positionSouris souris;

    // Gestion du temps pour boucler sur l'affichage
    tempsPrecedent = SDL_GetTicks();
    while (continuer)
    {
        // Gestion des actions à capturer
        gestionActions(&souris, &continuer, &event);

        tempsActuel = SDL_GetTicks();
        if (tempsActuel - tempsPrecedent > TEMPS_AFFICHAGE) /* Si TEMPS_AFFICHAGE ms se sont écoulées depuis le dernier tour de boucle */
        {
            // Mise à jour des données à envoyuer à l'écran
            objetJson *reponseSession = malloc(sizeof(objetJson));
            reponseSession->premier = NULL;
            requeteLANListeHotes(sock, reponseSession, session);

            // Affichage dans les logs du résultat
            lectureparsingJson(reponseSession, "");

            // Mise à jour de l'écran dans ce cas précis, sinon l'écran n'est pas rafraichi
            majEcran(ecran, reponseSession);

            libererParsingJson(reponseSession);

            tempsPrecedent = tempsActuel; /* Le temps "actuel" devient le temps "precedent" pour nos futurs calculs */
        }
        else /* Si ça fait moins de TEMPS_AFFICHAGE ms depuis le dernier tour de boucle, on endort le programme le temps qu'il faut */
        {
           SDL_Delay(TEMPS_AFFICHAGE - (tempsActuel - tempsPrecedent));
        }
    }
}

void gestionActions(positionSouris *souris, int *continuer, SDL_Event *event)
{

    if (SDL_PollEvent(event)) /* On utilise PollEvent et non WaitEvent pour ne pas bloquer le programme */
    {
        switch(event->type)
        {
            case SDL_QUIT:
                *continuer = 0;
                break;
            case SDL_MOUSEMOTION:
                souris->x = event->motion.x;
                souris->y = event->motion.y;
                break;
            case SDL_MOUSEBUTTONUP:
                souris->x = event->button.x;
                souris->y = event->button.y;

                // Action récupérée
                switch (event->button.button)
                {
                case SDL_BUTTON_LEFT:
                    // Clic bouton gauche pour déterminer une action

                    break;
                case SDL_BUTTON_WHEELUP:

                    break;
                case SDL_BUTTON_WHEELDOWN:

                    break;
                }

                break;
            case SDL_KEYDOWN:
                switch (event->key.keysym.sym)
                {
                    case SDLK_ESCAPE: /* Appui sur la touche Echap, on arrête le programme */
                        *continuer = 0;
                        break;
                    case SDLK_UP:

                        break;
                    case SDLK_DOWN:

                        break;
                    case SDLK_LEFT:

                        break;
                    case SDLK_RIGHT:

                        break;
                }
                break;
        }
    }

}

void majEcran(SDL_Surface *ecran, objetJson *donnees)
{
    SDL_Surface *texte = NULL;
    SDL_Rect position;
    TTF_Font *policePetite = NULL, *police = NULL;
    SDL_Color couleurNoire = {0, 0, 0}, couleurBlanche = {255, 255, 255}, couleurCyan = {0, 255, 255};
    char texteAAfficher[1024] = "";

    /* Chargement de la police */
    policePetite = TTF_OpenFont(FONT_POLICE, TAILLE_PETITE_POLICE);
    police = TTF_OpenFont(FONT_POLICE, TAILLE_POLICE);

    // Initialisation de l'écran
    SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
    // SDL_LockSurface(ecran); // Blocage d'écran pour colorier pixel par pixel

    // SDL_UnlockSurface(ecran); // Déblocage de l'écran

    // Position d'origine
    position.x = 0;
    position.y = 0;

    char *succes = NULL;
    rechercheValeur(donnees, "success", &succes);
    if (!strcmp(succes, "true"))
    {
        objetJson *resultat = NULL;
        rechercheValeur(donnees, "result", &resultat);

        //Parcours de la liste, on récupère le premier élément du tableau qu'on va parcourir
        paireJson *elt = resultat->premier;

        while (elt != NULL)
        {
            // Récupération de l'adresse IP
            char *id = NULL;
            rechercheValeur((objetJson *)elt->valeur, "id", &id);

            char *nom = NULL;
            rechercheValeur((objetJson *)elt->valeur, "primary_name", &nom);

            // Génération de la surface de texte
            texte = TTF_RenderText_Shaded(policePetite, nom, couleurBlanche, couleurNoire); /* On écrit la chaîne texte dans la SDL_Surface */

            // On ajoute le texte
            position.y += TAILLE_AFFICHAGE_LISTE;

            SDL_BlitSurface(texte, NULL, ecran, &position); /* Blit du texte */

            SDL_FreeSurface(texte); /* On supprime la surface précédente */

            elt = elt->suivant;
        }

    }

    SDL_Flip(ecran); // Mise à jour de l'écran

    TTF_CloseFont(policePetite);
    TTF_CloseFont(police);

}
