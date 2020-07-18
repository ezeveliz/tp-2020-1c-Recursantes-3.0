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
CONFIG="team"


# Datos del Broker
echo "Ingrese las posiciones de entrenadores"
	echo -n "> "
	read POSICIONES_ENTRENADORES

echo "POSICIONES_ENTRENADORES=$POSICIONES_ENTRENADORES" > "$CONFIG"

echo "Ingrese los pokemons que tienen los entrenadores"
	echo -n "> "
	read POKEMON_ENTRENADORES

echo "POKEMON_ENTRENADORES=POKEMON_ENTRENADORES" >> "$CONFIG"


# Datos proceso tema
echo "Ingrese la ip del Team"
	echo -n "> "
	read IP_TEAM

echo "IP_TEAM=$IP_TEAM" >> "$CONFIG"

echo "Ingrese el puerto del Team"
	echo -n "> "
	read PUERTO_TEAM

echo "PUERTO_TEAM=$PUERTO_TEAM" >> "$CONFIG"


#Datos proceso gamecard

echo "Ingrese la ip del Gamecard"
	echo -n "> "
	read IP_GAMECARD

echo "IP_GAMECARD=$IP_GAMECARD" >> "$CONFIG"

echo "Ingrese el puerto del Gamecard"
	echo -n "> "
	read PUERTO_GAMECARD

echo "PUERTO_GAMECARD=$PUERTO_GAMECARD" >> "$CONFIG"

#Datos propios
echo "Ingrese el id del proceso"
	echo -n "> "
	read MAC

echo "MAC=$MAC" >> "$CONFIG"





