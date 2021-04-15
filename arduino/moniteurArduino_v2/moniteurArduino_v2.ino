

#define MAXIN 200	// taille max d'une séquence de commandes
#define MAXOUT 200	// taille max d'un résultat

// variables pour stocker des données pendant l'exécution
long v1 = -1; 
long v2 = -1;
long v3 = -1;
long v4 = -1;
long v5 = -1;

// variable pour la gestion des wait
long w = -1;

int debug = 0; // O : off ; 1 : on

char xbee_buffer_in[MAXIN]; // lecture nouvelle séquence de commandes
char xbee_buffer_out[MAXOUT]; // envoi resultat

char lcmd[MAXIN]; // liste des commandes à exécuter

int len_lcmd = -1;	// longueur de la liste des commandes avant le traitement par traiter_lcmd
int pos_cmd = -1; 	// position de la commande courante


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

/************************* traiterLCmd() *****************************/

// remplacement espaces et ] par \0

void traiter_lcmd() {
	for (int i=0 ; i<len_lcmd ; i++) {
		if (lcmd[i] == ' ') lcmd[i] = 0;
		if (lcmd[i] == ']') lcmd[i] = 0;
	}
}

/**************************** get_pos_label **********************************/

// recherche d'une étiquette

int get_pos_label(char *label) {
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


/**************************** get_pos_param *******************************/

// lecture d'un paramètre de la commande
// pos : position de la commande courante
// num : numéro du paramètre (1, 2, ...)
// résultat : position du paramètre

int get_pos_param(int pos, int num) { 
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
			Serial.println("pas de [ au début de la commande\n");
			break;
		}
		else {
			Serial.println("Début commande");
			Serial.println(lcmd+pos+1);
			for (int i=1 ; i<=10 ; i++) {
				int p = get_pos_param(pos,i);
				if (p < 0) break;
				Serial.print("param ");
				Serial.print(i);
				Serial.print(" = ");
				Serial.println(lcmd+p);
			}
		}
		pos = get_next_cmd(pos);
	}
}

/****************************** cmd_exec *************************/

// exécution de la commande courante (position pos_cmd dans lcmd)

void cmd_exec() {
	if (pos_cmd < 0) {
		return;
	}
	if (lcmd[pos_cmd] != '[') {
		Serial.println("Execution impossible : pas de [ au début de la commande\n");
		return;
	}

	//if (w == -1) Serial.println("cmd_exec");
	if (strcmp(lcmd+pos_cmd+1,"dist") == 0) {
		vl53l1x_lire();
		if (debug == 1) {
				Serial.println("Exec [dist]");
		}
	}
	else if (strcmp(lcmd+pos_cmd+1,"w") == 0) {
		if (w == -1) {
			w = getLong("time") + getLong(lcmd+get_pos_param(pos_cmd,1));
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [w %s] start",lcmd+get_pos_param(pos_cmd,1));
				Serial.println(xbee_buffer_out);
			}
			return; // pour ne pas changer d'intruction courante
		}
		else if (w < getLong("time")) {
			w = -1;
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [w %s] stop",lcmd+get_pos_param(pos_cmd,1));
				Serial.println(xbee_buffer_out);
			}
		}
		else {
			return; // pour ne pas changer d'intruction courante
		}

	}
	else if (strcmp(lcmd+pos_cmd+1,"pdist") == 0) {
		// Serial.print("servo = ");
		// Serial.print(servo);
		// Serial.print(" mm = ");
		// Serial.print(vl53l1x_dist);
		// Serial.print(" rate = ");
		// Serial.print(vl53l1x_dist_rate);
		// Serial.println();
		if (debug == 1) {
			Serial.println("Exec [pdist]");
		}
		sprintf(xbee_buffer_out,"{\"servo\" : %d, \"mm\" : %d, \"rate\" : %d}",
				(int)servo,(int)vl53l1x_mm,(int)vl53l1x_rate);
		Serial.println(xbee_buffer_out);
	}
	else if (strcmp(lcmd+pos_cmd+1,"pvar") == 0) {
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
	else if (strcmp(lcmd+pos_cmd+1,"mr") == 0) {
		long speed = getLong(lcmd+get_pos_param(pos_cmd,1));
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
	else if (strcmp(lcmd+pos_cmd+1,"ml") == 0) {
		long speed = getLong(lcmd+get_pos_param(pos_cmd,1));
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
	else if (strcmp(lcmd+pos_cmd+1,"s") == 0) {

		long angle = getLong(lcmd+get_pos_param(pos_cmd,1));
		servo_angle((int)angle);
		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [s %d]",(int)angle);
			Serial.println(xbee_buffer_out);
		}
		//printf("exec ml %s %d\n",cmdItem2,speed);
	}
	
	else if (strcmp(lcmd+pos_cmd+1,"if") == 0) {
		long v1 = getLong(lcmd+get_pos_param(pos_cmd,1));
		long v2 = getLong(lcmd+get_pos_param(pos_cmd,3));

		int v = 0;
		if (strcmp(lcmd+get_pos_param(pos_cmd,2),"==") == 0) {
			if (v1 == v2) v = 1;
		}
		else if (strcmp(lcmd+get_pos_param(pos_cmd,2),"<") == 0) {
			if (v1 < v2) v = 1;
		}
		else if (strcmp(lcmd+get_pos_param(pos_cmd,2),"<=") == 0) {
			if (v1 <= v2) v = 1;
		}
		else if (strcmp(lcmd+get_pos_param(pos_cmd,2),">") == 0) {
			if (v1 > v2) v = 1;
		}
		else if (strcmp(lcmd+get_pos_param(pos_cmd,2),">=") == 0) {
			if (v1 >= v2) v = 1;
		}

		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [if %s %s %s] go : %d",lcmd+get_pos_param(pos_cmd,1),lcmd+get_pos_param(pos_cmd,2),lcmd+get_pos_param(pos_cmd,3),(int)v);
			Serial.println(xbee_buffer_out);
		}
		if (v == 1) {
			pos_cmd = get_pos_label(lcmd+get_pos_param(pos_cmd,4));
		}
		
	}
	else if (strcmp(lcmd+pos_cmd+1,"g") == 0) {
		pos_cmd = get_pos_label(lcmd+get_pos_param(pos_cmd,1));
		if (debug == 1) {
			sprintf(xbee_buffer_out,"Exec [go %s]",lcmd+pos_cmd);
			Serial.println(xbee_buffer_out);
		}

	}
	else if (strcmp(lcmd+pos_cmd+1,"don") == 0) { // debug on
		debug = 1;
		Serial.println("Exec [don]");
	}
	else if (strcmp(lcmd+pos_cmd+1,"doff") == 0) { // debug off
		debug = 0;
		Serial.println("Exec [don]");
	}
	else if (get_pos_param(pos_cmd,1) >= 0) { // commandes = et +=
		if (strcmp(lcmd+get_pos_param(pos_cmd,1),"=") == 0) {
			setLong(lcmd+pos_cmd+1,getLong(lcmd+get_pos_param(pos_cmd,2)));
			//printf("exec = : %s = %ld\n",cmdItem1,getLong(cmdItem1));
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [%s = %s]",lcmd+pos_cmd+1,lcmd+get_pos_param(pos_cmd,2));
				Serial.println(xbee_buffer_out);
			}
		}
		else if (strcmp(lcmd+get_pos_param(pos_cmd,1),"+=") == 0) {
			setLong(lcmd+pos_cmd+1,getLong(lcmd+pos_cmd+1)+getLong(lcmd+get_pos_param(pos_cmd,2)));
			//printf("exec += : %s = %ld\n",cmdItem1,getLong(cmdItem1));
			if (debug == 1) {
				sprintf(xbee_buffer_out,"Exec [%s += %s] val = %d",lcmd+pos_cmd+1,lcmd+get_pos_param(pos_cmd,2),(int)getLong(lcmd+pos_cmd+1));
				Serial.println(xbee_buffer_out);
			}
		}
	}
	

	pos_cmd = get_next_cmd(pos_cmd);
	//Serial.print("commande suivante : ");
	//Serial.println(lcmd+pos_cmd);
}

/******************************** setup - loop **************************/

void setup() {
	Serial.begin(9600);
	moteur_init();
	servo_init();
	vl53l1x_init();

	delay(2000);
	//ln[0] = 0;

	xbee_buffer_in[0] = 0;
	xbee_buffer_out[0] = 0;
	
	/*
	Exemples de commandes
	[don][mr 255][w 2000][mr 0]
	[don][v1 = 0][s 180][mr 255][ml 255][:b1][dist][pdist][v1 += 1][w 200][if v1 < 10 b1][mr 0][ml 0][s 0]
	*/

	// strcpy(lcmd,"[don][v1 = 0][s 180]");
	// //strcat(lcmd,"[mr 255][ml 255]");
	// strcat(lcmd,"[:b1][dist][pdist][v1 += 1][w 200][if v1 < 10 b1]");
	// strcat(lcmd,"[mr 0][ml 0][s 0]");

	// len_lcmd = strlen(lcmd);
	// traiter_lcmd();
	// //print_lcmd();
	// pos_cmd = 0;

	Serial.println("Arduino OK");

	// strcpy(xbee_buffer_envoyer,"Arduino OK");
	// xbee_envoyer();

}

void loop() {


	//lecture nouvelle commande
	if (Serial.available()) {
		int c = Serial.read();
		if (c == '\n') {
			strcpy(lcmd,xbee_buffer_in);
			len_lcmd = strlen(lcmd);
			traiter_lcmd(); // remplacement espaces et ] par \0
			pos_cmd = 0; // déclenche l'exécution
			xbee_buffer_in[0] = 0;
			debug = 0;
			

			// reset variables
			debug = 0;
			w = -1; // reset wait
			v1 = -1;
			v2 = -1;
			v3 = -1;
			v4 = -1;
			v5 = -1;
		}
		else if (strlen(xbee_buffer_in) >= MAXIN) {
				Serial.println("Erreur : commande trop longue");
				xbee_buffer_in[0] = 0;
		}
		else {
			int pos = strlen(xbee_buffer_in);
			xbee_buffer_in[pos] = c;
			xbee_buffer_in[pos+1] = 0;
		}
	}

	// exécution commande courante
	if (pos_cmd >= 0) {
		cmd_exec();
	}

}
