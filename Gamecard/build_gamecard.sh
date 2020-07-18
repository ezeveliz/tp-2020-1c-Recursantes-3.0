#!/bin/bash
#
# Gamecard - Deployment
# By Recursantes 3.0
# Buscanos en redes sociales
#

# Compilo
cmake CMakeLists.txt
make

#Asigno una variable con el path del archivo de configuracion
CONFIG="gamecard.config"


# Datos del Broker
echo "Ingrese la ip del Broker"
	echo -n "> "
	read IP_BROKER

echo "IP_BROKER=$IP_BROKER" > "$CONFIG"

echo "Ingrese el puerto del Broker"
	echo -n "> "
	read PUERTO_BROKER

echo "PUERTO_BROKER=$PUERTO_BROKER" >> "$CONFIG"

# Datos de operacion
echo "Ingrese punto de montaje"
	echo -n "> "
	read PUNTO_MONTAJE_TALLGRASS

echo "PUNTO_MONTAJE_TALLGRASS=$PUNTO_MONTAJE_TALLGRASS" >> "$CONFIG"

echo "Ingrese tiempo de retardo de operacion"
	echo -n "> "
	read TIEMPO_RETARDO_OPERACION

echo "TIEMPO_RETARDO_OPERACION=$TIEMPO_RETARDO_OPERACION" >> "$CONFIG"

echo "Ingrese tiempo de reintento de operacion"
	echo -n "> "
	read TIEMPO_DE_REINTENTO_OPERACION

echo "TIEMPO_DE_REINTENTO_OPERACION=$TIEMPO_DE_REINTENTO_OPERACION" >> "$CONFIG"

echo "Ingrese tiempo de reintento de conexion"
	echo -n "> "
	read TIEMPO_DE_REINTENTO_CONEXION

echo "TIEMPO_DE_REINTENTO_CONEXION=$TIEMPO_DE_REINTENTO_CONEXION" >> "$CONFIG"

echo "Ingrese ID del proceso"
	echo -n "> "
	read MAC

echo "MAC=$MAC" >> "$CONFIG"

echo "Ingrese el puerto de gamecard"
	echo -n "> "
	read PUERTO_GAMECARD

echo "PUERTO_GAMECARD=$PUERTO_GAMECARD" >> "$CONFIG"PUERTO_GAMECARD

./gamecard







	
	
