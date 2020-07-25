#!/bin/bash
#
# Gamecard - Deployment
# By Recursantes 3.0
# Buscanos en redes sociales
#


titulo(){
	../rainbow.py 3 "
 ██████╗  █████╗ ███╗   ███╗███████╗██████╗  ██████╗ ██╗   ██╗
 ██╔════╝ ██╔══██╗████╗ ████║██╔════╝██╔══██╗██╔═══██╗╚██╗ ██╔╝
 ██║  ███╗███████║██╔████╔██║█████╗  ██████╔╝██║   ██║ ╚████╔╝ 
 ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝  ██╔══██╗██║   ██║  ╚██╔╝  
 ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗██████╔╝╚██████╔╝   ██║ 
  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝╚═════╝  ╚═════╝    ╚═╝   
                                                              
                                         By Recursantes 3.0   
                                                              ";

                                     
}

titulo

#Bajo las pruebas 
git clone https://github.com/sisoputnfrba/delibird-pruebas.git
cd delibird-pruebas

# Compilo
cmake ..
make 

#Asigno una variable con el path del archivo de configuracion
CONFIG="gameboy_config"

#Datos fijos
PUERTO_BROKER="5002"
PUERTO_TEAM="5003"
PUERTO_GAMECARD="5004"
MAC="15"


# Datos obligatorios
echo "Default: 127.0.0.1"
echo "Ingrese la ip del Broker"
	echo -n "> "
	read IP_BROKER
	if [[ -z  $IP_BROKER ]]; then
		IP_BROKER="127.0.0.1"
	fi

echo "IP_BROKER=$IP_BROKER" > "$CONFIG"

echo "Default: 127.0.0.1"
echo "Ingrese la ip del Team"
	echo -n "> "
	read IP_TEAM
	if [[ -z  $IP_TEAM ]]; then
		IP_TEAM="127.0.0.1"
	fi

echo "IP_TEAM=$IP_TEAM" >> "$CONFIG"

echo "Default: 127.0.0.1"
echo "Ingrese la ip del Gamecard"
	echo -n "> "
	read IP_GAMECARD
	if [[ -z  $IP_GAMECARD ]]; then
		IP_GAMECARD="127.0.0.1"
	fi

echo "IP_GAMECARD=$IP_GAMECARD" >> "$CONFIG"

# Datos proceso tema
if [ "$1" = "-p" ]
then
  echo "Ingrese el puerto del Broker"
    echo -n "> "
    read PUERTO_BROKER

  echo "Ingrese el puerto del Team"
    echo -n "> "
    read PUERTO_TEAM

  echo "Ingrese el puerto del Gamecard"
    echo -n "> "
    read PUERTO_GAMECARD

  echo "Ingrese el id del proceso"
    echo -n "> "
    read MAC
fi


echo "PUERTO_BROKER=$PUERTO_BROKER" >> "$CONFIG"
echo "PUERTO_TEAM=$PUERTO_TEAM" >> "$CONFIG"
echo "PUERTO_GAMECARD=$PUERTO_GAMECARD" >> "$CONFIG"
echo "MAC=$MAC" >> "$CONFIG"
	
	
