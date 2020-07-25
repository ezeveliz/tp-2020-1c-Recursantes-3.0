#!/bin/bash
#
# Team - Deployment
# By Recursantes 3.0
# Buscanos en redes sociales
#

clear

titulo(){
	echo ""
	echo " ████████╗███████╗ █████╗ ███╗   ███╗";
	echo " ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║";
	echo "    ██║   █████╗  ███████║██╔████╔██║";
	echo "    ██║   ██╔══╝  ██╔══██║██║╚██╔╝██║";
	echo "    ██║   ███████╗██║  ██║██║ ╚═╝ ██║";
	echo "    ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝";
	echo "                                     ";
}

titulo_1(){
	echo ""
	echo " ████████╗███████╗ █████╗ ███╗   ███╗     ██╗"
	echo " ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║    ███║"
	echo "    ██║   █████╗  ███████║██╔████╔██║    ╚██║"
	echo "    ██║   ██╔══╝  ██╔══██║██║╚██╔╝██║     ██║"
	echo "    ██║   ███████╗██║  ██║██║ ╚═╝ ██║     ██║"
	echo "    ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝     ╚═╝"
	echo ""
}

titulo_2(){
	echo ""
	echo " ████████╗███████╗ █████╗ ███╗   ███╗    ██████╗ "
	echo " ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║    ╚════██╗"
	echo "    ██║   █████╗  ███████║██╔████╔██║     █████╔╝"
	echo "    ██║   ██╔══╝  ██╔══██║██║╚██╔╝██║    ██╔═══╝ "
	echo "    ██║   ███████╗██║  ██║██║ ╚═╝ ██║    ███████╗"
	echo "    ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝    ╚══════╝"
	echo ""

}

TITULO="0"
encabezado(){

	case $TITULO in
			"1" )
				titulo_1
				;;

			"2" ) 
				titulo_2
				;;   
	   
			* )  
				titulo
				;;
		esac

	echo "                   By Recursantes 3.0";
	echo "                                     ";
	echo ""
	echo ""
	echo -e "\n\nSetear configuraciones para las Pruebas"
	echo -e "\t1) Team 1"
	echo -e "\t2) Team 2"
	echo -e "\t3) Pruebas Base"
	echo -e "\n"
	echo -e "\tD) Descargar repo de Pruebas"
	echo -e "\tB) Buildear"
	echo -e "\tP) Personalizar"
	echo -e "\n"
	echo -e "\tE) Ejecutar Team"
	echo -e "\n"
	echo -e "\t0) Salir\n"
	echo -e "\tOpcion: \c"
}




# Variables prueba Modulo o default
POSICIONES_ENTRENADORES="[1|3,2|3,3|2]"
POKEMON_ENTRENADORES="[Pikachu]"
OBJETIVOS_ENTRENADORES="[Pikachu|Squirtle,Pikachu|Gengar,Squirtle|Onix]"
TIEMPO_RECONEXION="30"
RETARDO_CICLO_CPU="5"
ALGORITMO_PLANIFICACION="FIFO"
QUANTUM="2"
ALPHA="0.5"
ESTIMACION_INICIAL="5"
PUERTO_TEAM="5005"
TEAM_ID="123"
IP_BROKER="127.0.0.1"
PUERTO_BROKER="5002"
LOG_FILE="team.log"

#Asigno una variable con el path del archivo de configuracion
CONFIG="./team.config"


conf1(){
	#Team 1
	POSICIONES_ENTRENADORES="[1|3,2|3,2|2]"
	POKEMON_ENTRENADORES="[Pikachu]"
	OBJETIVOS_ENTRENADORES="[Pikachu|Squirtle,Pikachu|Gengar,Squirtle|Onix]"
	TIEMPO_RECONEXION="30"
	RETARDO_CICLO_CPU="5"
	ALGORITMO_PLANIFICACION="FIFO"
	QUANTUM="0"
	ALPHA="0.5"
	ESTIMACION_INICIAL="5"
	PUERTO_TEAM="5005"
	TEAM_ID="123"
}


conf2(){
	#Team 2
	POSICIONES_ENTRENADORES="[2|3,6|5,9|9,9|2,2|9]"
	POKEMON_ENTRENADORES="[]"
	OBJETIVOS_ENTRENADORES="[Vaporeon,Jolteon,Flareon,Umbreon,Espeon]"
	TIEMPO_RECONEXION="30"
	RETARDO_CICLO_CPU="5"
	ALGORITMO_PLANIFICACION="RR"
	QUANTUM="1"
	ALPHA="0.5"
	ESTIMACION_INICIAL="5"
	PUERTO_TEAM="5006"
	TEAM_ID="125"
}

#Escribo en el archivo
escribir_config(){
	echo "POSICIONES_ENTRENADORES=$POSICIONES_ENTRENADORES" > "$CONFIG"
	echo "POKEMON_ENTRENADORES=$POKEMON_ENTRENADORES" >> "$CONFIG"
	echo "OBJETIVOS_ENTRENADORES=$OBJETIVOS_ENTRENADORES" >> "$CONFIG"
	echo "TIEMPO_RECONEXION=$TIEMPO_RECONEXION" >> "$CONFIG"
	echo "RETARDO_CICLO_CPU=$RETARDO_CICLO_CPU" >> "$CONFIG"
	echo "ALGORITMO_PLANIFICACION=$ALGORITMO_PLANIFICACION" >> "$CONFIG"
	echo "ESTIMACION_INICIAL=$ESTIMACION_INICIAL" >> "$CONFIG"
	echo "IP_BROKER=$IP_BROKER" >> "$CONFIG"
	echo "PUERTO_BROKER=$PUERTO_BROKER" >> "$CONFIG"
	echo "PUERTO_TEAM=$PUERTO_TEAM" >> "$CONFIG"
	echo "LOG_FILE=$LOG_FILE" >> "$CONFIG"
	echo "TEAM_ID=$TEAM_ID" >> "$CONFIG"
	echo "QUANTUM=$QUANTUM" >> "$CONFIG"
	echo "ALPHA=$ALPHA" >> "$CONFIG"
	
	echo -e "\t\t\tArchivo config:\n"
	cat $CONFIG
}


personalizar(){
# Si es personalizado las sobreescribo
	echo -e "\t\t\tPersonalizar config:\n"
	
	cat $CONFIG

	echo -e "\n[Enter] para saltear\n"

		# Datos Broker
	echo "Ingrese la IP del Broker"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			IP_BROKER=$X
		fi


	echo "Ingrese el Puerto del Broker"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			PUERTO_BROKER=$X
		fi


	# Datos Team
	echo "Ingrese el Puerto del Team"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			PUERTO_TEAM=$X
		fi


	echo -e "Ingrese las posiciones de entrenadores"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			POSICIONES_ENTRENADORES=$X
		fi


	echo "Ingrese los pokemons que tienen los entrenadores"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			POKEMON_ENTRENADORES=$X
		fi


	echo "Ingrese los objetivos de los entrenadores"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			OBJETIVOS_ENTRENADORES=$X
		fi


	echo "Ingrese el tiempo de reconexion"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			TIEMPO_RECONEXION=$X
		fi


	echo "Ingrese el retardo de ciclo de CPU"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			RETARDO_CICLO_CPU=$X
		fi


	echo "Ingrese el algoritmo de planificacion"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			ALGORITMO_PLANIFICACION=$X
		fi


	echo "Ingrese el Quantum"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			QUANTUM=$X
		fi


	echo "Ingrese el Alpha"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			ALPHA=X$
		fi


	echo "Ingrese la estimacion inicial"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			ESTIMACION_INICIAL=$X
		fi



	echo "Ingrese el path del Log File"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			LOG_FILE=$X
		fi


	echo "Ingrese el Team ID"
		echo -n "> "
		read X
		if [[ -n $X ]]; then
			TEAM_ID=$X
		fi
}


bajar_pruebas(){
	#Bajo las pruebas
	if [ ! -d delibird-pruebas ]
	then
    	git clone https://github.com/sisoputnfrba/delibird-pruebas.git
	else
		cd delibird-pruebas
		git pull
		cd ..
	fi
}

buildear(){
	# Compilo
	mkdir build/
	cd build
	cmake ..
	make
	mv team ..
	cd ..
	rm -r build/
}

separador(){
	echo -e "\n=^..^=   =^..^=   =^..^=    =^..^=    =^..^=    =^..^=    =^..^=\n"
}

# trap ctrl-c and call ctrl_c()
EJECUTANDO="0"
function ctrl_c() {
	if [[ $EJECUTANDO == "1" ]]; then
		# killall team
		EJECUTANDO="0"
		opcion1="ninguna" #Para reiniciar el menu
	else
		exit
	fi
}
trap ctrl_c SIGINT

ejecutar(){
	if [ ! -f team ]
	then
		echo "|-----------|
| FALTO     |
| BUILDEAR, |
| YO LO     |
| HAGO      |
|-----------|
(\__/) ||
(•ㅅ•) ||
/ 　 づ"
		separador
		sleep 2
		buildear
		separador
	fi
	encabezado
	echo ""
	separador
	sleep 1
	EJECUTANDO="1"
	./team
}

menu(){
	opcion1="ninguna"
	while [ $opcion1 != "0"  ]
		do 
		
		encabezado

		read -rn1 opcion1

		if [ -z $opcion1 ] ; then opcion1="ninguna" ; fi

		echo -e "\n"

		separador
		case $opcion1 in
			"1" )
				conf1
				TITULO="1"
				escribir_config
				separador
				ejecutar
				;;

			"2" ) 
				conf2
				TITULO="2"
				escribir_config
				separador
				ejecutar
				;;

			"3" )
				escribir_config
				separador
				ejecutar
				;;

			[bB] )
				buildear
				;;

			[dD] )
				bajar_pruebas
				;;

			[pP] )
				personalizar
				escribir_config
				;;
	   
	   		[eE] )
				echo -e "\t\t\tArchivo config:\n"
				cat $CONFIG
				separador
				ejecutar
				;;
	   
			
			
			"0" )
				return
				;;   
	   
			* )  
				echo -e "Opcion \"$opcion1\" no reconocida"
				;;
		esac
		separador
	done
}

menu
