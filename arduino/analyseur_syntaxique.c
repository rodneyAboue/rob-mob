#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define MAXIN 250	// taille max d'une séquence de commandes
#define MAXOUT 100	// taille max d'un résultat

char xbee_buffer_in[MAXIN]; // lecture nouvelle séquence de commandes
char xbee_buffer_out[MAXOUT]; // envoi resultat

char lcmd[MAXIN]; // liste des commandes à exécuter

int len_lcmd;	// longueur de la liste des commandes avant le traitement par traiter_lcmd
int pos_cmd; 	// position de la commande courante

/************************* traiterLCmd() *****************************/

// remplacement espaces et ] par \0

void traiter_lcmd() {
	for (int i=0 ; i<len_lcmd ; i++) {
		if (lcmd[i] == ' ') lcmd[i] = 0;
		if (lcmd[i] == ']') lcmd[i] = 0;
	}
}

/**************************** get_label **********************************/

// recherche d'une étiquette

int get_label(char *label) {
	int res = -1;
	for (int i=0 ; i<len_lcmd ; i++) {
		if (lcmd[i] == ':') {
			if (strcmp(label,lcmd+i+1) == 0) {
				res = i-1;
			}
		}
	}
	return res;
}

/**************************** get_next_cmd *******************************/

// lecture de la commande suivante (recherche [)
// pos : position de la commande courante
// résultat : position de la commande suivante

int get_next_cmd(int pos) {
	int res = -1;
	for (int i=pos+1 ; i<len_lcmd ; i++) {
		if (lcmd[i] == '[') {
			res = i;
			break;
		}
	}
	return res;
}


/**************************** get_param *******************************/

// lecture d'un paramètre de la commande
// pos : position de la commande courante
// num : numéro du paramètre (1, 2, ...)
// résultat : position du paramètre

int get_param(int pos, int num) { 
	int res = -1;
	int count = 0;
	for (int i=pos ; i<len_lcmd ; i++) {
		if (lcmd[i] == 0) {
			count++;
			if (count == num) {
				res = i+1;
				if (res >= len_lcmd) {
					res = -1;
					break;
				}
				if (lcmd[res] == '[') {
					res = -1;
					break;
				}
			}
		}
	}
	return res;
}

/************************* printLCmd() *****************************/

// affichage de la liste des commandes contenues dans lcmd

void print_lcmd() {
	
	int pos = 0;	// position commande
	for (;;) {
		if (pos < 0) {
			break;
		}
		if (lcmd[pos] != '[') {
			printf("pas de [ au début de la commande\n");
			break;
		}
		else {
			printf("%s\n","Début commande");
			printf("%s\n",lcmd+pos+1);
			for (int i=1 ; i<=10 ; i++) {
				int p = get_param(pos,i);
				if (p < 0) break;
				printf("param %d = %s\n",i,lcmd+p);
			}
		}
		pos = get_next_cmd(pos);
	}
}

int main(int argc, char *argv[]) {
	strcpy(lcmd,"[don][:b1][v1 = 0][v1 += 1][if v1 < 10 b1]");
	printf("lcmd = %s\n",lcmd);
	len_lcmd = strlen(lcmd);
	traiter_lcmd();
	print_lcmd();

	printf("label : pos = %d s = %s\n",get_label("b1"),lcmd+get_label("b1"));
}
