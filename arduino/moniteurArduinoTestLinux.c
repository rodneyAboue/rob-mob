
// gcc pour compiler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <stdint.h>

/******************* variables d'exécution Arduino ******************/

long v1;	

/********** variables de décodage des commandes Arduino *************/

char cmdLn[200]; // ligne contenant une séquence de commandes

char *cmdCur; // pointeur sur la commande courante
char *cmdNext; // pointeur sur la commande suivante
char cmd[40] = {0}; // commande courante
char cmdItem1[10]; // item 1 commande courante
char cmdItem2[10]; // item 2 commande courante
char cmdItem3[10]; // item 3 commande courante
char cmdItem4[10]; // item 4 commande courante
char cmdItem5[10]; // item 5 commande courante
char cmdItem6[10]; // item 6 commande courante

char label[10]; // variable pour gérer les goto

/************* gestion des variables d'exécution Arduino ***********/

long l1;	// variable d'exécution Arduino

long getLong(char *s) {
	if (strcmp(s,"time") == 0) {
		struct timespec now;
  		timespec_get( &now, TIME_UTC );
		long res = now.tv_sec * 1000 + now.tv_nsec / 1e6;
		printf("time = %ld\n",res);
		return res;
	}
	else if (strcmp(s,"v1") == 0) {
		return v1;
	}
	else {
		return atol(s);
	}
}

void setLong(char *s, long v) {
	if (strcmp(s,"v1") == 0) {
		v1 = v;
	}
}

void addLong(char *s, long v) {
	if (strcmp(s,"v1") == 0) {
		v1 += v;
	}
}

/************************* decodage des lignes de commandes ********************/

char *cmdItem(char *srce, char *dest) {
	if (srce == NULL) {
		dest[0] = 0;
		return NULL;
	}
	char *s = strchr(srce,' ');
	if (s == NULL) {
		strcpy(dest,srce);
		return NULL;
	}
	strncpy(dest,srce,s-srce);
	dest[s-srce] = 0;
	return s+1;
}

void cmdRead() {

	cmdCur = cmdNext;

	char *pos1 = strchr(cmdNext,'[');
	char *pos2 = strchr(cmdNext,']');
	
	if ((pos1 == NULL) || (pos2 == NULL)) {
		cmdNext = NULL;
		return;
	}

	cmdCur = pos1;
	cmdNext = pos2+1;

	// commande courante
	strncpy(cmd,pos1+1,pos2-pos1-1);
	cmd[pos2-pos1-1] = 0;

	printf("cmd = [%s]\n",cmd);

	// decomposition de la commande courante
	char *s = cmd;
	s = cmdItem(s,cmdItem1);
	s = cmdItem(s,cmdItem2);
	s = cmdItem(s,cmdItem3);
	s = cmdItem(s,cmdItem4);
	s = cmdItem(s,cmdItem5);
	s = cmdItem(s,cmdItem6);

	// printf("cmdItem1 = [%s]\n",cmdItem1);
	// printf("cmdItem2 = [%s]\n",cmdItem2);
	// printf("cmdItem3 = [%s]\n",cmdItem3);
	// printf("cmdItem4 = [%s]\n",cmdItem4);
	// printf("cmdItem5 = [%s]\n",cmdItem5);
	// printf("cmdItem6 = [%s]\n",cmdItem6);

}

void cmdExec() {
	if (strcmp(cmdItem1,"mr") == 0) {
		int speed = atoi(cmdItem3);
		printf("exec mr %s %d\n",cmdItem2,speed);
	}
	else if (strcmp(cmdItem1,"ml") == 0) {
		int speed = atoi(cmdItem3);
		printf("exec ml %s %d\n",cmdItem2,speed);
	}
	else if (strcmp(cmdItem1,"if") == 0) {
		long v1 = getLong(cmdItem2);
		long v2 = getLong(cmdItem4);
		printf("v1 = %ld v2 = %ld\n",v1,v2);
		if (strcmp(cmdItem3,"<") == 0) {
			if (v1 < v2) {
				if (strcmp(cmdItem5,"goto") == 0) {
					strcpy(label, "[");
					strcat(label,cmdItem6);
					strcat(label,":]");
					cmdNext = strstr(cmdLn,label);
				}
			}
		}
		
	}
	else if (strcmp(cmdItem1,"goto") == 0) {
		
	}
	else if (strcmp(cmdItem2,"=") == 0) {
		setLong(cmdItem1,getLong(cmdItem3));
		printf("exec = : %s = %ld\n",cmdItem1,getLong(cmdItem1));
	}
	else if (strcmp(cmdItem2,"+=") == 0) {
		addLong(cmdItem1,getLong(cmdItem3));
		printf("exec += : %s = %ld\n",cmdItem1,getLong(cmdItem1));
	}
}

int main(int argc, char *argv[]) {

	strcpy(cmdLn,"");
	strcat(cmdLn,"[v1 = time]"); // variable t1 initialisée à time (nb de ms)
	strcat(cmdLn,"[v1 += 1000]");
	strcat(cmdLn,"[boucle:]");
	strcat(cmdLn,"[mr f 9]"); // moteur droit avant vitesse 9
	strcat(cmdLn,"[ml f 9]"); // moteur gauche arrière vitesse 9
	strcat(cmdLn,"[if time < v1 goto boucle]");

	//[if distr < 10000 goto stop][if dist < 300 goto stop][if time > 1000 goto stop][goto boucle][stop:][mr f 0][ml f 0]";

	printf("cmd = %s\n",cmdLn);

	cmdNext = cmdLn;

	printf("cmdNext = %s\n",cmdNext);
	for (int i=0 ;; i++) {
		cmdRead();
		if( cmdNext == NULL) break;
		cmdExec();
	}

}



