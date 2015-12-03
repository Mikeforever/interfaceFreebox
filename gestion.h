#ifndef GESTION
#define GESTION

void authentifierClient (TCPsocket sock);
int validerAuthentification(TCPsocket sock);
static char *pt(unsigned char *md, unsigned int len);
void SupprimerCaractere(char* str, char c);
void ouvrirSession(TCPsocket sock, char **sessionToken, char **permissions);
void recupDonneesIdentifiants(char *id, char **valeur);

#endif // GESTION
