/*=========================================
*
*    oO  Affichage pour TP Labyrinthe    Oo
*
*==========================================
*
* File : affichage.c
* Date : 5 octobre 10
* Author : Hilaire Thibault, Louise Huot
* 
* Contient les fonctions d'affichage pour le
* labyrinthe
*
*===========================================
*/

#include <X11/Xlib.h>			/* pour XLib */
#include <X11/keysym.h>			/* pour les touches XLib */
#include <X11/Xatom.h>			/* pour le message WM_DELETE_WINDOW */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "affichage.h"


/* taille des cases du labyrinthe, en pixels */
#define CASE	10


/* Variables X11 */
Display* display;
int screen;
GC gc;
Window w;
Colormap cmap;
Atom wmDeleteMessage;

/* Pixmap */
Pixmap pix;		/* pixmap général sur lequel on écrit tout */
Pixmap fond;	/* pixmap avec juste le quadrillage */

/* couleurs */
unsigned int couleurs[9];
unsigned int coulFond;
unsigned int coulMur;
unsigned int coulExploration[151];
#define BLANC 0
#define NOIR 1
#define BLEU 2
#define ROUGE 3
#define VERT 4
#define JAUNE 5
#define VIOLET 6

/* Divers */
int init=0;				/* permet de savoir si on a lancé InitAffichage */
int aff=0;			/* permet de savoir si on a fait un 1er affichage ou pas */
int down=0;

/* Fonction "alloueCouleur"
   Allocation d'une couleur dans la ColorMap
Paramètres : red,green,blue : quantité (16bits) de rouge,vert et bleu qui compose la couleur
Retour: renvoit un entier non signé identifiant la couleur dans la ColorMap
*/
unsigned int alloueCouleur( int red, int green, int blue)
{
	XColor coul;
	coul.green = green;
	coul.red = red;
	coul.blue = blue;
	XAllocColor( display, cmap, &coul);
	return coul.pixel;
}




/* Fonction 'initAffichage'
But: Permet d'initialiser tout l'affichage X11, de créer la fenêtre, etc...
Retour: renvoit 1 si tout c'est bien passé, et 0 en cas d'erreur */
int initAffichage()
{
	int x,y;
	
	/* Vérifications */
	if (init)
	{
		fprintf( stderr, "Erreur (initAffichage) : la fonction initAffichage ne doit être lancée qu'une seule fois!\n");
		return 0;
	}

	/* Initialisation X11 */
	if ( (display=XOpenDisplay(NULL)) == NULL )
	{
		fprintf( stderr, "Erreur (initAffichage) : connection avec X impossible : X est-il lancé ??\n");
		return 0;
	}

	/* crée la fenetre de taille TAILLE_X*CASE+1 , TAILLE_Y*CASE+1 */
	screen = DefaultScreen(display);
	gc = DefaultGC (display, screen);
	cmap = DefaultColormap( display, screen);
	w = XCreateSimpleWindow( display, RootWindow(display,screen), 0,0, TAILLE_X*CASE+1,TAILLE_Y*CASE+1,2 ,BlackPixel(display,screen), WhitePixel(display,screen));
	XStoreName( display, w, "TP Labyrinthe");

	/* crée la table de couleur */
	couleurs[BLANC] = alloueCouleur( 0xFFFF, 0xFFFF, 0xFFFF);
	couleurs[NOIR] = alloueCouleur( 0,0,0);
	couleurs[BLEU] = alloueCouleur( 0,0,0xFFFF);
	couleurs[ROUGE] = alloueCouleur( 0xFFFF,0,0);
	couleurs[VERT] = alloueCouleur( 0,0xFFFF,0);
	couleurs[JAUNE] = alloueCouleur( 0xFFFF, 0xFFFF,0);
	couleurs[VIOLET] = alloueCouleur( 0xCCCC, 0x4444, 0xFFFF);
	coulFond = alloueCouleur( 50000,50000,50000);			// gris
	coulMur = alloueCouleur( 0x1000, 0x1000, 0xFFFF);
	for( x=0; x<151; x++)
	{
		y = (30000/150)*x;
		coulExploration[x] = alloueCouleur( 65535-y/4,65535-2*y,0);
	}

		
	/* crée le pixmap dans lequel on va tout dessiner */
	int prof = XDefaultDepth (display, screen);
	pix = XCreatePixmap (display, RootWindow(display,screen), TAILLE_X*CASE+1,TAILLE_Y*CASE+1, prof);

	/* crée le pixmap de fond (dans lequel on rajoutera les obstacles ) */
	fond = XCreatePixmap (display, RootWindow(display,screen), TAILLE_X*CASE+1,TAILLE_Y*CASE+1, prof);
	XSetForeground( display, gc, WhitePixel(display,screen) );
	XFillRectangle( display,fond,gc, 0,0, TAILLE_X*CASE+1,TAILLE_Y*CASE+1);
	XSetForeground( display, gc, coulFond);
	for(y=0;y<=TAILLE_Y;y++)
		XDrawLine( display,fond,gc, 0,y*CASE,TAILLE_X*CASE+1,y*CASE);
	for(x=0;x<=TAILLE_X;x++)
		XDrawLine( display,fond,gc, x*CASE,0,x*CASE,TAILLE_Y*CASE+1);
	
	/* copie du fond sur pix */
	XCopyArea( display, fond, pix, gc, 0,0, TAILLE_X*CASE+1,TAILLE_Y*CASE+1, 0,0);

	/* sélectionne les évennements que l'on va gérer (renouvellement de l'affichage et gestion des touches, fermeture de fenêtre) */
	XSelectInput (display, w, ExposureMask | KeyPressMask | ButtonPressMask);
	wmDeleteMessage = XInternAtom( display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols( display, w, &wmDeleteMessage, 1);

	/* affiche la fenetre */
	XMapWindow(display, w);

	/* on en profite pour initialiser le générateur de nb aléatoire, avec la date courante */
	srand(time(0));

	/* mise à jour d'init (qui indique si on est passé dans initAffichage) */
  	init = 1;

	return 1;
}




/* Fonction afficheLabyrinthe
But: Afficher le labyrinthe et gèrer tous les évènements graphiques (déplacement de fenêtre, touches pressées, etc...). 
Paramètres:
 - laby : le tableau représentant le labyrinthe
 - depart, arrivee: coordonnées du départ et de l'arrivée
 - tempo : temporisation en ms. Mettre à zéro pour que cette fonction soit bloquante et attend que l'utilisateur appuie sur la touche 'espace' pour continuer
Retour: renvoie une valeur dépendant de la touche pressée par l'utilisateur
		0 si aucune touche n'a été pressée
		1 si l'utilisateur a pressé la touche 'espace'
 */
int afficheLabyrinthe( int laby[TAILLE_X][TAILLE_Y], int depart[2], int arrivee[2], int tempo)
{
	int fin_attente = 0;
	int x,y;
	int couleur;
	
	if (down)
		return 1;
	
	
	/* init ? */
	if (!init)
	{
		fprintf( stderr, "Warning (afficheLabyrinthe) : initAffichage n'a pas encore été exécuté\n");
		exit(EXIT_FAILURE);
	}
		

	/* affichage de chaque case */
	for( x=0; x<TAILLE_X; x++)
		for( y=0; y<TAILLE_Y; y++)
		{
			
			if (laby[x][y]==-1)
				couleur = coulMur;
			if (laby[x][y]>0)
				couleur = coulExploration[ laby[x][y]>300?150:laby[x][y]/2 ];
			if (laby[x][y]==-2)
				couleur = couleurs[VIOLET];
			/* dessin du rectangle sur pix */
			if (laby[x][y]!=0)
			{
				XSetForeground( display, gc, couleur);
				XFillRectangle( display, pix, gc, x*CASE+1,y*CASE+1, CASE-1, CASE-1);
			}
		}
		
	/* affichage du départ et de l'arrivée */
	XSetForeground( display, gc, couleurs[ROUGE]);
	XFillRectangle( display, pix, gc, depart[0]*CASE+1,depart[1]*CASE+1, CASE-1, CASE-1);
	XSetForeground( display, gc, couleurs[VERT]);
	XFillRectangle( display, pix, gc, arrivee[0]*CASE+1,arrivee[1]*CASE+1, CASE-1, CASE-1);
	
	/* vérification des coordonnées de départ/arrivée */
	if ( depart[0]<0 || depart[0]>=TAILLE_X || depart[1]<0 || depart[1]>=TAILLE_Y)
	{
		fprintf( stderr, "Warning (afficheLabyrinthe) : les coordonnées du départ ne sont pas valides (depart={%d,%d})\n", depart[0], depart[1] );
		exit(EXIT_FAILURE);
	}
	if ( arrivee[0]<0 || arrivee[0]>=TAILLE_X || arrivee[1]<0 || arrivee[1]>=TAILLE_Y)
	{
		fprintf( stderr, "Warning (afficheLabyrinthe) : les coordonnées de l' arrivee ne sont pas valides (depart={%d,%d})\n", depart[0], depart[1] );
		exit(EXIT_FAILURE);
	}
	if ( laby[ depart[0] ][ depart[1] ] == -1)
	{
		fprintf( stderr, "Warning (afficheLabyrinthe) : le départ se trouve sur un obstacle!\n");
		exit( EXIT_FAILURE);
	}
	if ( laby[ arrivee[0] ][ arrivee[1] ] == -1)
	{
		fprintf( stderr, "Warning (afficheLabyrinthe) : l'arrivée se trouve sur un obstacle!\n");
		exit( EXIT_FAILURE);
	}
	
	
	/* vérification des valeurs du tableau */
	for( x=0; x<TAILLE_X; x++)
		for( y=0; y<TAILLE_Y; y++)
			if ( laby[x][y]<-2 || laby[x][y]>TAILLE_X*TAILLE_Y)
			{
				fprintf( stderr, "Warning (afficheLabyrinthe) : Valeur du tableau incohérente (Laby[%d][%d]=%d\n", x, y, laby[x][y]);
				exit( EXIT_FAILURE);
			}



	/* affiche pix à l'écran */
	XCopyArea( display, pix, w, gc, 0,0, TAILLE_X*CASE+1,TAILLE_Y*CASE+1, 0,0);


	/* attente et gestion des évenements */
	time_t time=clock();
	while(!fin_attente)
	{
		XEvent e;
		if (XPending(display))		/* y'a-t-il un event qui nous attend */
		{
			XNextEvent(display,&e);
			switch (e.type)
			{
				case Expose :				/* réaffichage demandé */
					XCopyArea( display, pix, w, gc, 0,0, TAILLE_X*CASE+1 , TAILLE_Y*CASE+1, 0,0);
					break;
				case KeyPress:			/* touche pressée */
					if (XLookupKeysym(&e.xkey, 0) == XK_space)
						fin_attente=2;
					/*else if (XLookupKeysym(&e.xkey, 0) == XK_Escape)
					{
						XCloseDisplay( display);
						return 1;
					}*/
					break;
				case ButtonPress:		/* clic */
					x = e.xbutton.x/CASE;
					y = e.xbutton.y/CASE;
					if (x>=0 && x<TAILLE_X && y>=0 && y<TAILLE_Y)
						fprintf( stderr, " laby[%d][%d]=%d\n", x, y, laby[x][y]);
					break;
				case ClientMessage:	/* fermeture fenêtre */
				if (e.xclient.data.l[0] == wmDeleteMessage)
				{
					exit(0);
					/*XPeekEvent( display, &e );
					XCloseDisplay(display);
					return 1;*/
				}
			}
		}
		/* vérification si on a attendu suffisamment longtemps
		évidemment, on 'devrait' gérer cela mieux avec une interruption, plutôt que de lire sans arrêt le temps et vérifier si on n'a pas attendu trop longtemps */
		if ( tempo && ((clock()-time)*1000/CLOCKS_PER_SEC) > tempo)
			fin_attente=1;
 	}

	/* copie du fond sur pix pour le tour suivant */
	if (aff)
		XCopyArea( display, fond, pix, gc, 0,0, TAILLE_X*CASE+1,TAILLE_Y*CASE+1, 0,0);

	aff = 1;

	return (fin_attente==2);
}


/* Fonction alea
But: tire une valeur aléatoire comprise entre a et b (inclus)
Paramtère:
- a et b, les deux bornes
Retour:
- la valeur aléatoire */
int alea( int a, int b)
{
	if (a>b)
	{
		int temp;
		temp = a;
		a = b;
		b = temp;
	}
	return (int) (( rand()/(float)RAND_MAX ) * (b-a) + a);
}
