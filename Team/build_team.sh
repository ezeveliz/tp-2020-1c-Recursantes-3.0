#!/bin/bash
#
# Team - Deployment
# By Recursantes 3.0
# Buscanos en redes sociales
#

#Bajo las pruebas
git clone https://github.com/sisoputnfrba/delibird-pruebas.git
cd delibird-pruebas

# Compilo
cmake ..
make

#Asigno una variable con el path del archivo de configuracion
CONFIG="../team.config"


# Datos del Team
echo "Ingrese las posiciones de entrenadores"
	echo -n "> "
	read POSICIONES_ENTRENADORES

echo "POSICIONES_ENTRENADORES=$POSICIONES_ENTRENADORES" > "$CONFIG"

echo "Ingrese los pokemons que tienen los entrenadores"
	echo -n "> "
	read POKEMON_ENTRENADORES

echo "POKEMON_ENTRENADORES=$POKEMON_ENTRENADORES" >> "$CONFIG"

echo "Ingrese los objetivos de los entrenadores"
	echo -n "> "
	read OBJETIVOS_ENTRENADORES

echo "OBJETIVOS_ENTRENADORES=$OBJETIVOS_ENTRENADORES" >> "$CONFIG"

echo "Ingrese el tiempo de reconexion"
	echo -n "> "
	read TIEMPO_RECONEXION

echo "TIEMPO_RECONEXION=$TIEMPO_RECONEXION" >> "$CONFIG"

echo "Ingrese el retardo de ciclo de CPU"
	echo -n "> "
	read RETARDO_CICLO_CPU

echo "RETARDO_CICLO_CPU=$RETARDO_CICLO_CPU" >> "$CONFIG"

echo "Ingrese el algoritmo de planificacion"
	echo -n "> "
	read ALGORITMO_PLANIFICACION

echo "ALGORITMO_PLANIFICACION=$ALGORITMO_PLANIFICACION" >> "$CONFIG"

echo "Ingrese el Quantum"
	echo -n "> "
	read QUANTUM

echo "QUANTUM=$QUANTUM" >> "$CONFIG"

echo "Ingrese el Alpha"
	echo -n "> "
	read ALPHA

echo "ALPHA=$ALPHA" >> "$CONFIG"

echo "Ingrese la estimacion inicial"
	echo -n "> "
	read ESTIMACION_INICIAL

echo "ESTIMACION_INICIAL=$ESTIMACION_INICIAL" >> "$CONFIG"

# Datos Broker
echo "Ingrese la IP del Broker"
	echo -n "> "
	read IP_BROKER

echo "IP_BROKER=$IP_BROKER" >> "$CONFIG"

echo "Ingrese el Puerto del Broker"
	echo -n "> "
	read PUERTO_BROKER

echo "PUERTO_BROKER=$PUERTO_BROKER" >> "$CONFIG"

# Datos Team
echo "Ingrese el Puerto del Team"
	echo -n "> "
	read PUERTO_TEAM

echo "PUERTO_TEAM=$PUERTO_TEAM" >> "$CONFIG"

echo "Ingrese el path del Log File"
	echo -n "> "
	read LOG_FILE

echo "LOG_FILE=$LOG_FILE" >> "$CONFIG"

echo "Ingrese el Team ID"
	echo -n "> "
	read TEAM_ID

echo "TEAM_ID=$TEAM_ID" >> "$CONFIG"