#ifndef AFFICHAGE
#define AFFICHAGE

void affichage(SDL_Surface *ecran, objetJson *session, objetJson *permissions, TCPsocket sock);
void gestionActions(positionSouris *souris, int *continuer, SDL_Event *event);

#endif // AFFICHAGE
