#ifndef __CLIENT_H_
#define __CLIENT_H_
/*
 * Envoie de données lues dans un fichier ou dans le flux stdin vers un destinataire
 * La fonction :
 * - Crée un socket en IPV6 , UDP et se connecte à l'adresse de destination
 * - Envoie des données par packet et réagit suivant s'il recoit un acquittement
 *   un non-acquittement
 *
 * @hostname: L'addresse à laquelle on se connecte
 * @port: Le port auquel on se connecte
 * @file: Un nom de fichier contenant les données ou NULL si non spécifié
 * @post: Toutes les données du contenues dans le fichier ou sur le stdin on été envoyée
 *
 * @return:
 */
void send_data(char *hostname, int port, char* file);

#endif
