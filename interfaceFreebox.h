#ifndef INTERFACEFREEBOX
#define INTERFACEFREEBOX

void ouvrirSocket(TCPsocket *sock, char *hote, char *nombrePort);
void fermerSocket(TCPsocket *sock);
void envoyerRequete(TCPsocket sock, const char *requete);
void recevoirRequeteLigne(TCPsocket sock, char **reponse);
int recevoirReponseJson(TCPsocket sock, char **reponse, int nbCaracteres);
void* realloc_s (void **ptr, size_t taille);
void receptionReponse(TCPsocket sock, char **repHttp, char **serveur, char **date, char **contentType, char **reponse);
void echangeReponseJson(TCPsocket sock, const char *message, objetJson *reponseJson);
void generationRequeteGET(TCPsocket sock, char *entete, objetJson *reponseJson);
void generationRequeteGETSession(TCPsocket sock, char *entete, objetJson *reponseJson, char *sessionToken);
void generationRequetePOST(TCPsocket sock, char *entete, char *corps, objetJson *reponseJson);
void generationRequetePOSTSession(TCPsocket sock, char *entete, char *corps, objetJson *reponseJson, char *sessionToken);
void generationRequetePUTSession(TCPsocket sock, char *entete, char *corps, objetJson *reponseJson, char *sessionToken);
void requeteAutorisation(TCPsocket sock, objetJson *reponseJson);
void requeteVerification(TCPsocket sock, objetJson *reponseJson, char *trackid);
void requeteLogin(TCPsocket sock, objetJson *reponseJson);
void requeteSession(TCPsocket sock, objetJson *reponseJson, char *password);
void requeteSessionAuthentifie(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteFreeplug(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteConnexionStatus(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteDeconnexion(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteLogAppels(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteConnexionXdsl(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteLANInterfaces(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteLANListeHotes(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteMajLCDConfigurationB0(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteMajLCDConfigurationB50(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteMajLCDConfigurationB100(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteMajLCDConfigurationO90(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteMajLCDConfigurationO45(TCPsocket sock, objetJson *reponseJson, char *sessionToken);
void requeteMajLCDConfigurationOReinit(TCPsocket sock, objetJson *reponseJson, char *sessionToken);

#endif // INTERFACEFREEBOX
