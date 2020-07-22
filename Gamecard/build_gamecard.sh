#!/bin/bash
#
# Gamecard - Deployment
# By Recursantes 3.0
# Buscanos en redes sociales
#


clear
echo -e "\n"
echo "   ██████╗  █████╗ ███╗   ███╗███████╗ ██████╗ █████╗ ██████╗ ██████╗ ";
echo "  ██╔════╝ ██╔══██╗████╗ ████║██╔════╝██╔════╝██╔══██╗██╔══██╗██╔══██╗";
echo "  ██║  ███╗███████║██╔████╔██║█████╗  ██║     ███████║██████╔╝██║  ██║";
echo "  ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝  ██║     ██╔══██║██╔══██╗██║  ██║";
echo "  ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗╚██████╗██║  ██║██║  ██║██████╔╝";
echo "   ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝ ";
echo "                                                                            ";
echo "                                                         By Recursantes 3.0";
echo "                                                                     ";
echo "                                                                     ";



# Compilo
cmake CMakeLists.txt
make

#Asigno una variable con el path del archivo de configuracion
CONFIG="gamecard.config"


# Datos del Broker
echo "Ingrese la ip del Broker"
echo "Default: 127.0.0.1"
	echo -n "> "
	read IP_BROKER
	if [[ -z  $IP_BROKER ]]; then
		IP_BROKER="127.0.0.1"
	fi

echo -e "\nIP_BROKER=$IP_BROKER\n"
echo "IP_BROKER=$IP_BROKER" > "$CONFIG"

#Variables 
PUERTO_BROKER="5002"
PUNTO_MONTAJE_TALLGRASS="."
TIEMPO_RETARDO_OPERACION="5"
TIEMPO_DE_REINTENTO_OPERACION="5"
TIEMPO_DE_REINTENTO_CONEXION="10"
MAC="92"
PUERTO_GAMECARD="5004"
PUNTO_LOG="gamecard.log"
NIVEL_LOG="1"

# Configuracion personalizada 
if [ "$1" = "-p" ]
then 
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

	echo "PUERTO_GAMECARD=$PUERTO_GAMECARD" >> "$CONFIG"
else

	echo "PUERTO_BROKER=$PUERTO_BROKER" >> "$CONFIG"

	echo "PUNTO_MONTAJE_TALLGRASS=$PUNTO_MONTAJE_TALLGRASS" >> "$CONFIG"

	echo "TIEMPO_RETARDO_OPERACION=$TIEMPO_RETARDO_OPERACION" >> "$CONFIG"

	echo "TIEMPO_DE_REINTENTO_OPERACION=$TIEMPO_DE_REINTENTO_OPERACION" >> "$CONFIG"

	echo "TIEMPO_DE_REINTENTO_CONEXION=$TIEMPO_DE_REINTENTO_CONEXION" >> "$CONFIG"

	echo "MAC=$MAC" >> "$CONFIG"

	echo "PUERTO_GAMECARD=$PUERTO_GAMECARD" >> "$CONFIG"
	
	echo "PUNTO_LOG=$PUNTO_LOG" >> "$CONFIG"

	echo "NIVEL_LOG=$NIVEL_LOG" >> "$CONFIG"
fi

#./gamecard







	
	
