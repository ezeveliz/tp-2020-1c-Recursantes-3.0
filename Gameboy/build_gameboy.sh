#!/bin/bash
#
# Gamecard - Deployment
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
CONFIG="gameboy_config"


# Datos del Broker
echo "Ingrese la ip del Broker"
	echo -n "> "
	read IP_BROKER

echo "IP_BROKER=$IP_BROKER" > "$CONFIG"

echo "Ingrese el puerto del Broker"
	echo -n "> "
	read PUERTO_BROKER

echo "PUERTO_BROKER=$PUERTO_BROKER" >> "$CONFIG"


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

./gameboy




	
	
