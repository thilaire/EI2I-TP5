/*=========================================
*
*    oO  Affichage pour TP Labyrinthe    Oo
*
*==========================================
*
* File : affichage.h
* Date : 5 octobre 10
* Author : Hilaire Thibault, Louise Huot
* 
* Contient les prototypes des fonctions d'affichage
* pour le labyrinthe, ainsi que quelques constantes
*
*======================================================================
*
* Remarque: l'utilisation de affichage.o pour l'édition de lien
*           implique l'utilisation de l'option "-lX11 -L/usr/X11R6/lib"
*           à la fin de la commande gcc (-l indique le nom de la librairie,
*           ici X11, et	-L indique le chemin de la librairie). 
*
* Utilisation:
*    Avant de pouvoir afficher un labyrinthe, il faut initialiser l'affichage
*    en appelant la fonction 'initAffichage' (une seule et unique fois).
*    Ensuite, chaque appel à 'afficheLabyrinthe' affiche le labyrinthe passé
*    en paramètre.
*    En cas d'erreur (tableau labyrinthe non-conforme, etc.), un message
*    d'erreur est affiché (à la console).
*    De plus, pendant l'attente de la fonction 'afficheLabyrinthe', il est 
*    possible de connaitre la valeur d'une case du labyrinthe en cliquant
*    dessus : la valeur s'affiche alors dans la console. Cela peut-être
*    pratique pour le debuggage.
*/


#ifndef __AFFICHAGE_H__
#define __AFFICHAGE_H__


/* Définition des constantes de labyrinthe */
#define TAILLE_X	101
#define TAILLE_Y	61


/* Fonction 'initAffichage'
But: Permet d'initialiser tout l'affichage X11, de créer la fenêtre, etc...
Retour: renvoit 1 si tout c'est bien passé, et 0 en cas d'erreur */
int initAffichage();


/* Fonction afficheLabyrinthe
But: Afficher le labyrinthe et gèrer tous les évènements graphiques (déplacement
de fenêtre, touches pressées, etc...). Après l'affichage du labyrinthe, la
fonction attend un certain temps (indiqué par tempo) avant de se terminer. Cela
permet de par exemple d'afficher pas-à-pas l'évolution du labyrinthe, ou de
faire une pause pendant l'affichage (en attente que la touche 'espace' soit
pressée
Paramètres:
 - laby : le tableau représentant le labyrinthe
 - depart, arrivee: coordonnées du départ et de l'arrivée
 - tempo : temporisation en ms. Mettre à zéro pour que cette fonction soit 
 bloquante et attend que l'utilisateur appuie sur la touche 'espace' pour
 continuer
Retour: renvoie une valeur dépendant de la touche pressée par l'utilisateur
		0 si aucune touche n'a été pressée
		1 si l'utilisateur a pressé la touche 'espace'
 */
int afficheLabyrinthe( int laby[TAILLE_X][TAILLE_Y], int depart[2], int arrivee[2], int tempo);



#endif
	