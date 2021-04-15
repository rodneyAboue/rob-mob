
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>

// exemple de séquence de commandes
// [v1 = time][v1 += 1000][boucle1:][mr f 255][ml f 255][if time < v1 goto boucle1][mr f 0][ml f 0]
/*
[s 90]

[s1 = 0]
[s1_incr = 10]
[b1:]
[s s1][dist][print]
[t1 = time][t1 += 1000]
[b2:][if time < t1 b2]
[s1 += s1_incr]
[if s1 > 180 b3]
[if s1 < 0 b4]
[goto b1]
[b3:][s1 = 180][s1_incr = -10][s1 += s1_incr][goto b1]
[b4:][s1 = 0][s1_incr = 10][s1 += s1_incr][goto b1]
*/


#define MAXSEQ 250	// taille max d'une séquence de commandes
#define MAXCMD 30	// taille max d'une commande
#define MAXITEM 10	// taille max d'un élément de commande

/********************************************************************/
/**************************** variables arduino *********************/
/********************************************************************/

//char ln[MAXSEQ]; // stockage des caractères reçus sur Serial ou XBee

/********************************************************************/
/************************ variables du moniteur *********************/
/********************************************************************/


long v1 = -1; // variable pour stocker une donnée pendant l'exécution
long v2 = -1; // variable pour l'exécution d'une séquence de commandes
long v3 = -1; // variable pour l'exécution d'une séquence de commandes
long v4 = -1; // variable pour l'exécution d'une séquence de commandes
long v5 = -1; // variable pour l'exécution d'une séquence de commandes

long w = -1; // variable pour la gestion des wait

int debug = 0; // O : off ; 1 : on

char cmdLn[MAXSEQ]; // séquence de commandes à exécuter

char *cmdCur = NULL; // pointeur sur la commande courante
char *cmdNext = NULL; // pointeur sur la commande suivante

char cmd[MAXCMD] = {0}; // commande courante
char cmdItem1[MAXITEM]; // item 1 commande courante
char cmdItem2[MAXITEM]; // item 2 commande courante
char cmdItem3[MAXITEM]; // item 3 commande courante
char cmdItem4[MAXITEM]; // item 4 commande courante
char cmdItem5[MAXITEM]; // item 5 commande courante
char cmdItem6[MAXITEM]; // item 6 commande courante

char label[MAXITEM]; // variable pour gérer les goto

char xbee_buffer_in[MAXSEQ]; // lecture nouvelle commande
char xbee_buffer_out[MAXSEQ]; // envoi resultat

/*******************************************************************/
/********************* moteurs - shield DRI0009 ********************/
/*******************************************************************/


//Arduino PWM Speed Control：
int E1 = 5;  
int M1 = 4; 
int E2 = 6;                      
int M2 = 7;                        


void moteur_init() {
	pinMode(M1, OUTPUT);   
	pinMode(M2, OUTPUT); 
}

void moteur_gauche_avant(int speed) {
	digitalWrite(M1,LOW);
	analogWrite(E1, speed);   //PWM Speed Control
}

void moteur_gauche_arriere(int speed) {
	digitalWrite(M1,HIGH);
	analogWrite(E1, speed);   //PWM Speed Control
}

void moteur_droite_avant(int speed) {
 	digitalWrite(M2,LOW);
	analogWrite(E2, speed);   //PWM Speed Control
}

void moteur_droite_arriere(int speed) {
	digitalWrite(M2,HIGH);
	analogWrite(E2, speed);   //PWM Speed Control
}

void moteurs_stop() {
	analogWrite(E1, 0);   //PWM Speed Control
	analogWrite(E2, 0);   //PWM Speed Control
}

/*******************************************************************/
/********************************* servo ***************************/
/*******************************************************************/

#include <Servo.h>

Servo myservo;
long servo = -1;

void servo_init() {
	myservo.attach(8);
}

void servo_angle(int a) { // 0 .. 180
	myservo.write(a);
	servo = a;
	// Serial.print("servo ");
	// Serial.println(angle);
}

/*******************************************************************/
/********************************* VL53L1X *************************/
/*******************************************************************/

#include <Wire.h>
#include <ComponentObject.h>
#include <RangeSensor.h>
#include <SparkFun_VL53L1X.h>
#include <vl53l1x_class.h>
#include <vl53l1_error_codes.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

long vl53l1x_mm = -1;
long vl53l1x_rate = -1;
long vl53l1x_status = -1;

void vl53l1x_init() {
		Wire.begin();

    if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    //Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }

  distanceSensor.setDistanceModeShort();
  //distanceSensor.setDistanceModeLong();

  //Serial.println("Sensor online!");

}

void vl53l1x_lire() {
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();

  int signalRate = distanceSensor.getSignalRate();
  //Serial.print("\tSignal rate: ");
  //Serial.print(signalRate);

  byte rangeStatus = distanceSensor.getRangeStatus();
  //Serial.print("\tRange Status: ");
  //Serial.print(rangeStatus);
  //Serial.println();

 vl53l1x_mm = distance;
 vl53l1x_rate = signalRate;
 vl53l1x_status = rangeStatus; // 0 : OK ; 7 : NOK

//   Serial.print("Distance(mm): ");
//   Serial.print(vl53l1x_dist);

}

/********************************************************************/
/*************************** moniteur *******************************/
/********************************************************************/

/************* gestion des variables d'exécution Arduino ***********/

long getLong(char *s) {
	if (strcmp(s,"v1") == 0) {
		return v1;
	}
	else if (strcmp(s,"v2") == 0) {
		return v2;
	}
	else if (strcmp(s,"v3") == 0) {
		return v3;
	}
	else if (strcmp(s,"v4") == 0) {
		return v4;
	}
	else if (strcmp(s,"v5") == 0) {
		return v5;
	}
	else if (strcmp(s,"time") == 0) {
		// struct timespec now;
  		// timespec_get( &now, TIME_UTC );
		// long res = now.tv_sec * 1000 + now.tv_nsec / 1e6;
		// printf("time = %ld\n",res);
		return millis();
	}
	else if (strcmp(s,"servo") == 0) {
		return servo;
	}
	else if (strcmp(s,"mm") == 0) {
		return vl53l1x_mm;
	}
	else if (strcmp(s,"rate") == 0) {
		return vl53l1x_rate;
	}
	else if (strcmp(s,"status") == 0) {
		return vl53l1x_status;
	}
	else {
		return atol(s);
	}
}

void setLong(char *s, long v) {
	if (strcmp(s,"v1") == 0) {
		v1 = v;
	}
	else if (strcmp(s,"v2") == 0) {
		v2 = v;
	}
	else if (strcmp(s,"v3") == 0) {
		v3 = v;
	}
	else if (strcmp(s,"v4") == 0) {
		v4 = v;
	}
	else if (strcmp(s,"v5") == 0) {
		v5 = v;
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

	//printf("cmd = [%s]\n",cmd);

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
	if (strcmp(cmdItem1,"dist") == 0) {
		vl53l1x_lire();
		if (debug == 1) {
			Serial.println("dist");
		}
	}
	else if (strcmp(cmdItem1,"w") == 0) {
		if (w == -1) {
			w = getLong("time") + getLong(cmdItem2);
			cmdNext = cmdCur;
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [w %s] start",cmdItem2);
				Serial.println(xbee_buffer_out);
			}
		}
		else if (w < getLong("time")) {
			w = -1;
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [w %s] stop",cmdItem2);
				Serial.println(xbee_buffer_out);
			}
		}
		else {
			cmdNext = cmdCur;
		}

	}
	else if (strcmp(cmdItem1,"pdist") == 0) {
		// Serial.print("servo = ");
		// Serial.print(servo);
		// Serial.print(" mm = ");
		// Serial.print(vl53l1x_dist);
		// Serial.print(" rate = ");
		// Serial.print(vl53l1x_dist_rate);
		// Serial.println();
		if (debug == 1) {
			Serial.println("pdist");
		}
		sprintf(xbee_buffer_out,"{\"servo\" : %d, \"mm\" : %d, \"rate\" : %d}",
				(int)servo,(int)vl53l1x_mm,(int)vl53l1x_rate);
		Serial.println(xbee_buffer_out);
	}
	else if (strcmp(cmdItem1,"pvar") == 0) {
		// Serial.print("servo = ");
		// Serial.print(servo);
		// Serial.print(" mm = ");
		// Serial.print(vl53l1x_dist);
		// Serial.print(" rate = ");
		// Serial.print(vl53l1x_dist_rate);
		// Serial.println();
		if (debug == 1) {
			Serial.println("pvar");
		}
		sprintf(xbee_buffer_out,"{\"v1\" : %d, \"v2\" : %d, \"v3\" : %d, \"v4\" : %d, \"v5\" : %d}",
			(int)v1,(int)v2,(int)v3,(int)v4,(int)v5);
		//xbee_envoyer();
		Serial.println(xbee_buffer_out);

	}
	else if (strcmp(cmdItem1,"mr") == 0) {
		long speed = getLong(cmdItem2);
		if (speed >= 0) {
			moteur_droite_avant((int)speed);
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [mr %d]",(int)speed);
				Serial.println(xbee_buffer_out);
			}
		}
		else {
			moteur_droite_arriere((int)(-speed));
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [mr %d]",(int)speed);
				Serial.println(xbee_buffer_out);
			}
		}
		//printf("exec mr %s %d\n",cmdItem2,speed);
	}
	else if (strcmp(cmdItem1,"ml") == 0) {
		long speed = getLong(cmdItem2);
		if (speed >= 0) {
			moteur_gauche_avant((int)speed);
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [ml %d]",(int)speed);
				Serial.println(xbee_buffer_out);
			}
		}
		else {
			moteur_gauche_arriere((int)(-speed));
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [ml %d]",(int)speed);
				Serial.println(xbee_buffer_out);
			}
		}
		//printf("exec ml %s %d\n",cmdItem2,speed);
	}
	else if (strcmp(cmdItem1,"s") == 0) {

		long angle = getLong(cmdItem2);
		servo_angle((int)angle);
		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [s %d]",(int)angle);
			Serial.println(xbee_buffer_out);
		}
		//printf("exec ml %s %d\n",cmdItem2,speed);
	}
	else if (strcmp(cmdItem1,"if") == 0) {
		long v1 = getLong(cmdItem2);
		long v2 = getLong(cmdItem4);
		//printf("v1 = %ld v2 = %ld\n",v1,v2);
		int v = 0;
		if (strcmp(cmdItem3,"==") == 0) {
			if (v1 == v2) v = 1;
		}
		else if (strcmp(cmdItem3,"<") == 0) {
			if (v1 < v2) v = 1;
		}
		else if (strcmp(cmdItem3,"<=") == 0) {
			if (v1 <= v2) v = 1;
		}
		else if (strcmp(cmdItem3,">") == 0) {
			if (v1 > v2) v = 1;
		}
		else if (strcmp(cmdItem3,">=") == 0) {
			if (v1 >= v2) v = 1;
		}

		if (v == 1) {
					strcpy(label, "[");
					strcat(label,cmdItem5);
					strcat(label,":]");
					cmdNext = strstr(cmdLn,label);
		}
		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [if %s %s %s] go : %d",cmdItem2,cmdItem3,cmdItem4,(int)v);
			Serial.println(xbee_buffer_out);
		}
		
	}
	else if (strcmp(cmdItem1,"g") == 0) {
		strcpy(label, "[");
		strcat(label,cmdItem2);
		strcat(label,":]");
		cmdNext = strstr(cmdLn,label);
		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [go %s]",cmdItem2);
			Serial.println(xbee_buffer_out);
		}

	}
	else if (strcmp(cmdItem2,"=") == 0) {
		setLong(cmdItem1,getLong(cmdItem3));
		//printf("exec = : %s = %ld\n",cmdItem1,getLong(cmdItem1));
		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [%s = %s]",cmdItem1,cmdItem3);
			Serial.println(xbee_buffer_out);
		}
	}
	else if (strcmp(cmdItem2,"+=") == 0) {
		setLong(cmdItem1,getLong(cmdItem1)+getLong(cmdItem3));
		//printf("exec += : %s = %ld\n",cmdItem1,getLong(cmdItem1));
		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [%s += %s]",cmdItem1,cmdItem3);
			Serial.println(xbee_buffer_out);
		}
	}
	else if (strcmp(cmdItem1,"don") == 0) { // debug on
		debug = 1;
		Serial.println("Exec [don]");
	}
	else if (strcmp(cmdItem1,"doff") == 0) { // debug off
		debug = 0;
		Serial.println("Exec [don]");
	}
}

void setup() {
	Serial.begin(9600);
	moteur_init();
	servo_init();
	vl53l1x_init();

	delay(2000);
	//ln[0] = 0;
	
	Serial.println("Arduino OK");

	// strcpy(xbee_buffer_envoyer,"Arduino OK");
	// xbee_envoyer();

}

void loop() {


	//lecture nouvelle commande
	if (Serial.available()) {
		int c = Serial.read();
		if (c == '\n') {
			strcpy(cmdLn,xbee_buffer_in);
			xbee_buffer_in[0] = 0;
			cmdNext = cmdLn;
			sprintf(xbee_buffer_out,"Arduino %s",cmdLn);
			Serial.println(xbee_buffer_out);

			// reset variables
			debug = 0;
			w = -1; // reset wait
			v1 = -1;
			v2 = -1;
			v3 = -1;
			v4 = -1;
			v5 = -1;
		}
		else {
			int pos = strlen(xbee_buffer_in);
			xbee_buffer_in[pos] = c;
			xbee_buffer_in[pos+1] = 0;
		}
	}

	// exécution commande courante
	if (cmdNext != NULL) {
		cmdRead();
		if(cmdNext != NULL) cmdExec();
	}

}


