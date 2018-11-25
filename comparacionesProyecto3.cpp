// Universidad del Valle de Guatemala
// Programacion de Microprocesadores
// Proyecto 3 - Prediccion de Datos C++
// Andrea Elias 17048
// Kevin Macario 17369
// David Soto 17551

// Basado en el codigo de:
// Christian Medina

// compile with:
// g++ comparacionesProyecto3.cpp bme280.h -lwiringPi -o comparacionesProyecto3

//Se incluyen las librerias necesarias
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include "bme280.h"
#include <ctime>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#define VUELTAS 28800 // 28800 tomas de mediciones equivale a 48 horas de medicion con 3 segundos de intervalo entre cada una

using namespace std;

//Subrutina que calcula la media de cualquier tipo de dato recolectado
float media(float* arreglo, string tipo, string nombre, string nombre2) {
	//Se crea la variable de lectura de archivo y otras variables para las cuentas que brindaran la media
    fstream fs;
	float sumatoria = 0;
	float med = 0;

	//Se realiza la sumatoria de todos los datos
	for (int i = 0;i < VUELTAS;i++) {
		sumatoria = sumatoria + arreglo[i];
	}

	//Se calcula la media
	med = sumatoria / (float)VUELTAS;

	//Se hace la escritura de la media correspondiente al dato mandado y el archivo donde fueron tomadas la mediciones en un archivo de comparaciones
	fs.open(nombre, std::fstream::app);
	fs << "La media de las " << tipo << " es: " << med << " ,del archivo: " << nombre2 << endl;
	fs.close();

	//Se devuelve la media
	return med;
}

//Subrutina que calcula cual fue la mayor medicion en el dia con hora a la que fue medida
float mayor(float* arreglo, string tipo, string* hora, string nombre, string nombre2) {
	//Se crea la variable de lectura de archivo y otras variables para las cuentas que brindaran el dato mayor
    fstream fs;
	float may = arreglo[0];
	string hor = hora[0];

	//Se comparan los datos mandados para buscar el mayor
	for (int i = 0;i < VUELTAS;i++)
	{
		if (arreglo[i] > may) {
			may = arreglo[i];
			hor = hora[i];
		}

	}
	//Se hace la escritura del maximo correspondiente al dato mandado y el archivo donde fueron tomadas la mediciones en un archivo de comparaciones
	fs.open(nombre, std::fstream::app);
	fs << "La lectura mayor de las " << tipo << " es: " << may << " ,tomada a la hora: " << hor << " ,del archivo: " << nombre2 << endl;
	fs.close();

	//Se devuelve el mayor
	return may;
}

//Subrutina que calcula cual fue la menor medicion en el dia con hora a la que fue medida
float menor(float* arreglo, string tipo, string* hora, string nombre, string nombre2) {
	//Se crea la variable de lectura de archivo y otras variables para las cuentas que brindaran el dato menor
    fstream fs;
	float men = arreglo[0];
	string hor = hora[0];

	//Se comparan los datos mandados para buscar el menor
	for (int i = 0;i < VUELTAS;i++)
	{
		if (arreglo[i] < men) {
			men = arreglo[i];
			hor = hora[i];
		}

	}
	//Se hace la escritura del minimo correspondiente al dato mandado y el archivo donde fueron tomadas la mediciones en un archivo de comparaciones
	fs.open(nombre, std::fstream::app);
	fs <<"La lectura menor de las " << tipo << " es: " << men << " ,tomada a la hora: " << hor <<" ,del archivo: "<<nombre2<<endl;
	fs.close();

	//Se devuelve el menor
	return men;
}

void comparar(float* arregloOriginal, float* arreglo, string tipo ,string nombre, string raspCuda) {
	//Se crea la variable de lectura de archivo y otras variables para las cuentas que brindaran la media
	fstream fs;
	float sumatoria = 0.0;
	float sumatoria2 = 0.0;
	float porcentaje = 0.0;
	int contador = 0;
	int n = 600;
	int m = 48;
	for (int i = 0;i < m;i++) {
		sumatoria = 0.0;
		sumatoria2 = 0.0;
		//Se realiza la sumatoria de todos los datos
		for (int j = 0;j < n;j++) {
			sumatoria = sumatoria + arregloOriginal[j + n*contador];
			sumatoria2 = sumatoria2 + arreglo[j + n*contador];
		}
		sumatoria = sumatoria / 600;
		sumatoria2 = sumatoria2 / 600;
		porcentaje = abs(sumatoria - sumatoria2) * 100.0 / sumatoria;

		//Se hace la escritura de la media correspondiente al dato mandado y el archivo donde fueron tomadas la mediciones en un archivo de comparaciones
		fs.open(nombre, std::fstream::app);
		fs << "El porcentaje de error de la "<< tipo << " para la "<<i+1<<"°a media hora del dia es de:; "<< porcentaje<<" ; para las estimaciones en "<< raspCuda<< endl;
		fs.close();
		contador = contador + 1;
	}
}

//Inicio del programa
int main(int argv, char* argc[]) {
	//Variables para el control de archivos
	fstream fs1;
	fstream fs2;

	// Continuacion del Programa para el dia 1
	printf("\npulsa INTRO para continuar con las comparaciones del dia 1...");
	fflush(stdin);
	char tecla = getchar();

	//Se limpia el archivo de comparaciones para el primer dia
	ofstream ArchivoPrediccion("181113Comparaciones.csv");
	ArchivoPrediccion.close();

	// Se leen los datos del dia 1
	ifstream datos("181113.csv");
	string nombre = "181113Comparaciones.csv";
	string nombre2 = "181113.csv";
	string linea;
	float temperaturas[VUELTAS];
	float humedades[VUELTAS];
	float presiones[VUELTAS];
	float temperaturasO[VUELTAS];
	float humedadesO[VUELTAS];
	float presionesO[VUELTAS];
	string horas[VUELTAS];
	int contadorPosicion = 0;
	string::size_type sz;

	// Se obtienen los datos separados de cada linea guardada
	while (getline(datos, linea)) {
		string delimiter = ";";
		size_t pos = 0;
		string token;
		int cont = 0;
		while ((pos = linea.find(delimiter)) != std::string::npos) {
			token = linea.substr(0, pos);
			linea.erase(0, pos + delimiter.length());
			if (cont == 0) {
				horas[contadorPosicion] = token;
			}
			if (cont == 1) {
				temperaturasO[contadorPosicion] = temperaturas[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 2) {
				humedadesO[contadorPosicion] = humedades[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 3) {
				presionesO[contadorPosicion] = presiones[contadorPosicion] = stof(token, &sz);
			}
			cont = cont + 1;

		}
		contadorPosicion = contadorPosicion + 1;
	}

	// Calcula media de temperaturas
	float mediaT = media(temperaturas, "temperaturas", nombre, nombre2);

	// Calcula media de humedades
	float mediaH = media(humedades, "humedades", nombre, nombre2);

	// Calcula media de presiones
	float mediaP = media(presiones, "presiones", nombre, nombre2);

	// Calcula la mayor de las temperaturas
	float mayT = mayor(temperaturas, "temperaturas", horas, nombre, nombre2);

	// Calcula la mayor de las humedades
	float mayH = mayor(humedades, "humedades", horas, nombre, nombre2);

	// Calcula la mayor de las presiones
	float mayP = mayor(presiones, "presiones", horas, nombre, nombre2);

	// Calcula la menor de las temperaturas
	float menT = menor(temperaturas, "temperaturas", horas, nombre, nombre2);

	// Calcula la menor de las humedades
	float menH = menor(humedades, "humedades", horas, nombre, nombre2);

	// Calcula la menor de las presiones
	float menP = menor(presiones, "presiones", horas, nombre, nombre2);


	// Se leen los datos del dia 1 predecidos por la RBPI
	ifstream datos1("181113_estPI.csv");
	nombre = "181113Comparaciones.csv";
	nombre2 = "181113_estPI.csv";
	contadorPosicion = 0;

	// Se obtienen los datos separados de cada linea guardada
	while (getline(datos1, linea)) {
		string delimiter = ";";
		size_t pos = 0;
		string token;
		int cont = 0;
		while ((pos = linea.find(delimiter)) != std::string::npos) {
			token = linea.substr(0, pos);
			linea.erase(0, pos + delimiter.length());
			if (cont == 0) {
				horas[contadorPosicion] = token;
			}
			if (cont == 1) {
				temperaturas[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 2) {
				humedades[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 3) {
				presiones[contadorPosicion] = stof(token, &sz);
			}
			cont = cont + 1;

		}
		contadorPosicion = contadorPosicion + 1;
	}

	// Calcula media de temperaturas
	float mediaT1 = media(temperaturas, "temperaturas", nombre, nombre2);

	// Calcula media de humedades
	float mediaH1 = media(humedades, "humedades", nombre, nombre2);

	// Calcula media de presiones
	float mediaP1 = media(presiones, "presiones", nombre, nombre2);

	// Calcula la mayor de las temperaturas
	float mayT1 = mayor(temperaturas, "temperaturas", horas, nombre, nombre2);

	// Calcula la mayor de las humedades
	float mayH1 = mayor(humedades, "humedades", horas, nombre, nombre2);

	// Calcula la mayor de las presiones
	float mayP1 = mayor(presiones, "presiones", horas, nombre, nombre2);

	// Calcula la menor de las temperaturas
	float menT1 = menor(temperaturas, "temperaturas", horas, nombre, nombre2);

	// Calcula la menor de las humedades
	float menH1 = menor(humedades, "humedades", horas, nombre, nombre2);

	// Calcula la menor de las presiones
	float menP1 = menor(presiones, "presiones", horas, nombre, nombre2);

	//Comparaciones de datos medidos con los predecido de la RBPI
	float porcentajeErrorT = abs(mediaT - mediaT1) * 100 / mediaT;
	float porcentajeErrorH = abs(mediaH - mediaH1) * 100 / mediaH;
	float porcentajeErrorP = abs(mediaP - mediaP1) * 100 / mediaP;

	//Se imprime en pantalla los porcentajes de error obtenidos al comparar las medias medidas con las predecidas con la RBPI
	cout << "El porcentaje de error para las temperaturas en el dia 1 fue de: " << porcentajeErrorT << " para las predicciones en RBPI" << endl;
	cout << "El porcentaje de error para las humedades en el dia 1 fue de: " << porcentajeErrorH << " para las predicciones en RBPI" << endl;
	cout << "El porcentaje de error para las presiones en el dia 1 fue de: " << porcentajeErrorP << " para las predicciones en RBPI" << endl;

	//Se realiza la escritura de los porcentajes de error en el archivo de comparaciones del dia 1
	fs1.open("181113Comparaciones.csv", std::fstream::app);
	fs1 << "El porcentaje de error para las temperaturas en el dia 1 fue de: " << porcentajeErrorT << " para las predicciones en RBPI" << endl;
	fs1.close();
	fs1.open("181113Comparaciones.csv", std::fstream::app);
	fs1 << "El porcentaje de error para las humedades en el dia 1 fue de: " << porcentajeErrorH << " para las predicciones en RBPI" << endl;
	fs1.close();
	fs1.open("181113Comparaciones.csv", std::fstream::app);
	fs1 << "El porcentaje de error para las presiones en el dia 1 fue de: " << porcentajeErrorP << " para las predicciones en RBPI" << endl;
	fs1.close();

	// Comparacion entre datos originales y estimados RBPI del segundo dia
	comparar(temperaturasO, temperaturas, "temperaturas", nombre, "RBPI");
	comparar(humedadesO, humedades, "humedades", nombre, "RBPI");
	comparar(presionesO, presiones, "presiones", nombre, "RBPI");

	// Se leen los datos del dia 1 predecidos por CUDA
	ifstream datos4("181113_estCU.csv");
	nombre = "181113Comparaciones.csv";
	nombre2 = "181113_estCU.csv";
	contadorPosicion = 0;

	// Se obtienen los datos separados de cada linea guardada
	while (getline(datos4, linea)) {
		string delimiter = ";";
		size_t pos = 0;
		string token;
		int cont = 0;
		while ((pos = linea.find(delimiter)) != std::string::npos) {
			token = linea.substr(0, pos);
			linea.erase(0, pos + delimiter.length());
			if (cont == 0) {
				horas[contadorPosicion] = token;
			}
			if (cont == 1) {
				temperaturas[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 2) {
				humedades[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 3) {
				presiones[contadorPosicion] = stof(token, &sz);
			}
			cont = cont + 1;

		}
		contadorPosicion = contadorPosicion + 1;
	}

	// Calcula media de temperaturas
	float mediaT2 = media(temperaturas, "temperaturas", nombre,nombre2);

	// Calcula media de humedades
	float mediaH2 = media(humedades, "humedades", nombre,nombre2);

	// Calcula media de presiones
	float mediaP2 = media(presiones, "presiones", nombre,nombre2);

	// Calcula la mayor de las temperaturas
	float mayT2 = mayor(temperaturas, "temperaturas", horas, nombre,nombre2);

	// Calcula la mayor de las humedades
	float mayH2 = mayor(humedades, "humedades", horas, nombre,nombre2);

	// Calcula la mayor de las presiones
	float mayP2 = mayor(presiones, "presiones", horas, nombre,nombre2);

	// Calcula la menor de las temperaturas
	float menT2 = menor(temperaturas, "temperaturas", horas, nombre,nombre2);

	// Calcula la menor de las humedades
	float menH2 = menor(humedades, "humedades", horas, nombre,nombre2);

	// Calcula la menor de las presiones
	float menP2 = menor(presiones, "presiones", horas, nombre,nombre2);

	//Comparaciones entre los datos medidos con los predichos por CUDA
	porcentajeErrorT = abs(mediaT - mediaT2) * 100 / mediaT;
	porcentajeErrorH = abs(mediaH - mediaH2) * 100 / mediaH;
	porcentajeErrorP = abs(mediaP - mediaP2) * 100 / mediaP;

	//Se imprimen en pantalla los porcentajes de error de las medias obtenidas entre los datos medidos y los predecidos por CUDA
	cout << "El porcentaje de error para las temperaturas en el dia 1 fue de: " << porcentajeErrorT << " para las predicciones en CUDA" << endl;
	cout << "El porcentaje de error para las humedades en el dia 1 fue de: " << porcentajeErrorH << " para las predicciones en CUDA" << endl;
	cout << "El porcentaje de error para las presiones en el dia 1 fue de: " << porcentajeErrorP << " para las predicciones en CUDA" << endl;

	//Se hace la escritura de los porcentajes de error obtenidos en un archivo de comparaciones del dia 1
	fs1.open("181113Comparaciones.csv", std::fstream::app);
	fs1 << "El porcentaje de error para las temperaturas en el dia 1 fue de: " << porcentajeErrorT << " para las predicciones en CUDA" << endl;
	fs1.close();
	fs1.open("181113Comparaciones.csv", std::fstream::app);
	fs1 << "El porcentaje de error para las humedades en el dia 1 fue de: " << porcentajeErrorH << " para las predicciones en CUDA" << endl;
	fs1.close();
	fs1.open("181113Comparaciones.csv", std::fstream::app);
	fs1 << "El porcentaje de error para las presiones en el dia 1 fue de: " << porcentajeErrorP << " para las predicciones en CUDA" << endl;
	fs1.close();

	// Comparacion entre datos originales y estimados RBPI del segundo dia
	comparar(temperaturasO, temperaturas, "temperaturas", nombre, "CUDA");
	comparar(humedadesO, humedades, "humedades", nombre, "CUDA");
	comparar(presionesO, presiones, "presiones", nombre, "CUDA");

  // Continuacion del Programa para el dia 2
  printf("\npulsa INTRO para continuar con las comparaciones del dia 2...");
  fflush(stdin);
  tecla = getchar();

    //Se limpia el archivo donde se escribiran las comparaciones de las predicciones del dia 2
	ofstream ArchivoPrediccion2("181114Comparaciones.csv");
	ArchivoPrediccion2.close();

	// Se leen los datos del dia 2
	ifstream datos2("181114.csv");
	nombre = "181114Comparaciones.csv";
	nombre2 = "181114.csv";
	contadorPosicion = 0;

	// Se obtienen los datos separados de cada linea guardada
	while (getline(datos2, linea)) {
		string delimiter = ";";
		size_t pos = 0;
		string token;
		int cont = 0;
		while ((pos = linea.find(delimiter)) != std::string::npos) {
			token = linea.substr(0, pos);
			linea.erase(0, pos + delimiter.length());
			if (cont == 0) {
				horas[contadorPosicion] = token;
			}
			if (cont == 1) {
				temperaturasO[contadorPosicion] = temperaturas[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 2) {
				humedadesO[contadorPosicion] = humedades[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 3) {
				presionesO[contadorPosicion] = presiones[contadorPosicion] = stof(token, &sz);
			}
			cont = cont + 1;

		}
		contadorPosicion = contadorPosicion + 1;
	}

	// Calcula media de temperaturas
	mediaT = media(temperaturas, "temperaturas",nombre,nombre2);

	// Calcula media de humedades
	mediaH = media(humedades, "humedades",nombre,nombre2);

	// Calcula media de presiones
	mediaP = media(presiones, "presiones",nombre,nombre2);

	// Calcula la mayor de las temperaturas
	mayT = mayor(temperaturas, "temperaturas", horas,nombre,nombre2);

	// Calcula la mayor de las humedades
	mayH = mayor(humedades, "humedades", horas,nombre,nombre2);

	// Calcula la mayor de las presiones
	mayP = mayor(presiones, "presiones", horas,nombre,nombre2);

	// Calcula la menor de las temperaturas
	menT = menor(temperaturas, "temperaturas", horas, nombre,nombre2);

	// Calcula la menor de las humedades
	menH = menor(humedades, "humedades", horas,nombre,nombre2);

	// Calcula la menor de las presiones
	menP = menor(presiones, "presiones", horas,nombre,nombre2);


	// Se leen los datos del dia 2 de las predicciones de RBPI
	ifstream datos3("181114_estPI.csv");
	nombre = "181114Comparaciones.csv";
	nombre2 = "181114_estPI.csv";
	contadorPosicion = 0;

	// Se obtienen los datos separados de cada linea guardada
	while (getline(datos3, linea)) {
		string delimiter = ";";
		size_t pos = 0;
		string token;
		int cont = 0;
		while ((pos = linea.find(delimiter)) != std::string::npos) {
			token = linea.substr(0, pos);
			linea.erase(0, pos + delimiter.length());
			if (cont == 0) {
				horas[contadorPosicion] = token;
			}
			if (cont == 1) {
				temperaturas[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 2) {
				humedades[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 3) {
				presiones[contadorPosicion] = stof(token, &sz);
			}
			cont = cont + 1;

		}
		contadorPosicion = contadorPosicion + 1;
	}

	// Calcula media de temperaturas
	mediaT1 = media(temperaturas, "temperaturas", nombre,nombre2);

	// Calcula media de humedades
	mediaH1 = media(humedades, "humedades", nombre,nombre2);

	// Calcula media de presiones
	mediaP1 = media(presiones, "presiones", nombre,nombre2);

	// Calcula la mayor de las temperaturas
	mayT1 = mayor(temperaturas, "temperaturas", horas, nombre,nombre2);

	// Calcula la mayor de las humedades
	mayH1 = mayor(humedades, "humedades", horas, nombre,nombre2);

	// Calcula la mayor de las presiones
	mayP1 = mayor(presiones, "presiones", horas, nombre,nombre2);

	// Calcula la menor de las temperaturas
	menT1 = menor(temperaturas, "temperaturas", horas, nombre,nombre2);

	// Calcula la menor de las humedades
	menH1 = menor(humedades, "humedades", horas, nombre,nombre2);

	// Calcula la menor de las presiones
	menP1 = menor(presiones, "presiones", horas, nombre,nombre2);

	//Comparaciones entre los datos medidos y las predicciones realizadas por RBPI del dia 2
	porcentajeErrorT = abs(mediaT-mediaT1) * 100 / mediaT;
	porcentajeErrorH = abs(mediaH - mediaH1) * 100 / mediaH;
	porcentajeErrorP = abs(mediaP - mediaP1) * 100 / mediaP;

	//Se imprimen los porcentajes de error entre las medias de los datos medidos y las predicciones de RBPI para el dia 2
	cout <<"El porcentaje de error para las temperaturas en el dia 2 fue de: "<<porcentajeErrorT << " para las predicciones en RBPI" << endl;
	cout <<"El porcentaje de error para las humedades en el dia 2 fue de: "<<porcentajeErrorH << " para las predicciones en RBPI" << endl;
	cout <<"El porcentaje de error para las presiones en el dia 2 fue de: "<<porcentajeErrorP << " para las predicciones en RBPI" << endl;

	//Se hace escritura de los porcentajes de error en el archivo de comparaciones para el dia 2
	fs2.open("181114Comparaciones.csv", std::fstream::app);
	fs2 << "El porcentaje de error para las temperaturas en el dia 2 fue de: " << porcentajeErrorT << " para las predicciones en RBPI" << endl;
	fs2.close();
	fs2.open("181114Comparaciones.csv", std::fstream::app);
	fs2 << "El porcentaje de error para las humedades en el dia 2 fue de: " << porcentajeErrorH << " para las predicciones en RBPI" << endl;
	fs2.close();
	fs2.open("181114Comparaciones.csv", std::fstream::app);
	fs2 << "El porcentaje de error para las presiones en el dia 2 fue de: " << porcentajeErrorP << " para las predicciones en RBPI" << endl;
	fs2.close();

	// Comparacion entre datos originales y estimados RBPI del segundo dia
	comparar(temperaturasO,temperaturas, "temperaturas", nombre , "RBPI");
	comparar(humedadesO, humedades, "humedades", nombre, "RBPI");
	comparar(presionesO, presiones, "presiones", nombre, "RBPI");

	// Se leen los datos del dia 2 de las predicciones de CUDA
	ifstream datos5("181114_estCU.csv");
	nombre = "181114Comparaciones.csv";
	nombre2 = "181114_estCU.csv";
	contadorPosicion = 0;

	// Se obtienen los datos separados de cada linea guardada
	while (getline(datos5, linea)) {
		string delimiter = ";";
		size_t pos = 0;
		string token;
		int cont = 0;
		while ((pos = linea.find(delimiter)) != std::string::npos) {
			token = linea.substr(0, pos);
			linea.erase(0, pos + delimiter.length());
			if (cont == 0) {
				horas[contadorPosicion] = token;
			}
			if (cont == 1) {
				temperaturas[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 2) {
				humedades[contadorPosicion] = stof(token, &sz);
			}
			if (cont == 3) {
				presiones[contadorPosicion] = stof(token, &sz);
			}
			cont = cont + 1;

		}
		contadorPosicion = contadorPosicion + 1;
	}

	// Calcula media de temperaturas
	mediaT2 = media(temperaturas, "temperaturas", nombre, nombre2);

	// Calcula media de humedades
	mediaH2 = media(humedades, "humedades", nombre, nombre2);

	// Calcula media de presiones
	mediaP2 = media(presiones, "presiones", nombre, nombre2);

	// Calcula la mayor de las temperaturas
	mayT2 = mayor(temperaturas, "temperaturas", horas, nombre, nombre2);

	// Calcula la mayor de las humedades
	mayH2 = mayor(humedades, "humedades", horas, nombre, nombre2);

	// Calcula la mayor de las presiones
	mayP2 = mayor(presiones, "presiones", horas, nombre, nombre2);

	// Calcula la menor de las temperaturas
	menT2 = menor(temperaturas, "temperaturas", horas, nombre, nombre2);

	// Calcula la menor de las humedades
	menH2 = menor(humedades, "humedades", horas, nombre, nombre2);

	// Calcula la menor de las presiones
	menP2 = menor(presiones, "presiones", horas, nombre, nombre2);

	//Comparaciones entre los datos medidos y las predicciones de CUDA del dia 2
	porcentajeErrorT = abs(mediaT - mediaT2) * 100 / mediaT;
	porcentajeErrorH = abs(mediaH - mediaH2) * 100 / mediaH;
	porcentajeErrorP = abs(mediaP - mediaP2) * 100 / mediaP;

	//Se imprimen los porcentajes de error de las medias de los datos medidos y los predecidos por CUDA del dia 2
	cout << "El porcentaje de error para las temperaturas en el dia 2 fue de: " << porcentajeErrorT << " para las predicciones en CUDA" << endl;
	cout << "El porcentaje de error para las humedades en el dia 2 fue de: " << porcentajeErrorH << " para las predicciones en CUDA" << endl;
	cout << "El porcentaje de error para las presiones en el dia 2 fue de: " << porcentajeErrorP << " para las predicciones en CUDA" << endl;

	//Se hace escritura de los porcentajes de error en el archivo de comparaciones del dia 2
	fs2.open("181114Comparaciones.csv", std::fstream::app);
	fs2 << "El porcentaje de error para las temperaturas en el dia 2 fue de: " << porcentajeErrorT << " para las predicciones en CUDA" << endl;
	fs2.close();
	fs2.open("181114Comparaciones.csv", std::fstream::app);
	fs2 << "El porcentaje de error para las humedades en el dia 2 fue de: " << porcentajeErrorH << " para las predicciones en CUDA" << endl;
	fs2.close();
	fs2.open("181114Comparaciones.csv", std::fstream::app);
	fs2 << "El porcentaje de error para las presiones en el dia 2 fue de: " << porcentajeErrorP << " para las predicciones en CUDA" << endl;
	fs2.close();

	// Comparacion entre datos originales y estimados CUDA del segundo dia
	comparar(temperaturasO, temperaturas, "temperaturas", nombre, "CUDA");
	comparar(humedadesO, humedades, "humedades", nombre, "CUDA");
	comparar(presionesO, presiones, "presiones", nombre, "CUDA");
}
