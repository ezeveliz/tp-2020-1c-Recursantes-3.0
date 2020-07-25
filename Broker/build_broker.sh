#!/bin/bash

CONFIG=configs/

encabezado(){
	clear

	./rainbow.py 5 "
	██████╗ ██████╗  ██████╗ ██╗  ██╗███████╗██████╗ 
	██╔══██╗██╔══██╗██╔═══██╗██║ ██╔╝██╔════╝██╔══██╗
	██████╔╝██████╔╝██║   ██║█████╔╝ █████╗  ██████╔╝
	██╔══██╗██╔══██╗██║   ██║██╔═██╗ ██╔══╝  ██╔══██╗
	██████╔╝██║  ██║╚██████╔╝██║  ██╗███████╗██║  ██║
	╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝

	                               By Recursantes 3.0
"
	echo -e "\nPruebas Broker"
	echo -e "\t1) Consolidacion"
	echo -e "\t2) Compactacion"
	echo -e "\t3) Buddy System / Prueba Final"
	echo -e "\t4) Prueba Final"
	echo -e "\n\tB) Buildear Broker\n"
	echo -e "\t0) Salir de la herramienta\n"
	echo -e "\tOpcion: \c"	
}


fifo_a_lru () {
   sed -i "s/ALGORITMO_REEMPLAZO=FIFO/ALGORITMO_REEMPLAZO=LRU/" $1
}

lru_a_fifo () {
   sed -i "s/ALGORITMO_REEMPLAZO=LRU/ALGORITMO_REEMPLAZO=FIFO/" $1
}


consolidacion (){
	CONFIG=configs/consolidacion.cfg
}

compactacion (){
	CONFIG=configs/compactacion.cfg
}

buddy (){
	CONFIG=configs/buddy.cfg
}

buildear (){
	mkdir build/
	cd build
	cmake ..
	make
	mv broker ..
	cd ..
	rm -r build/
}

separador(){
	echo -e "\n=^..^=   =^..^=   =^..^=    =^..^=    =^..^=    =^..^=    =^..^=\n"
}

mostrar_archivo(){
	separador
	echo -e "\t\t\tArchivo config:\n"
	cat $CONFIG
	separador
}

ejecutar(){
	if [ ! -f broker ]
	then
		separador
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
	fi
	mostrar_archivo
	EJECUTANDO="1"
	./broker $CONFIG
}

# trap ctrl-c and call ctrl_c()
EJECUTANDO="0"
function ctrl_c() {
	if [[ $EJECUTANDO == "1" ]]; then
		# killall broker
		EJECUTANDO="0"
		opcion1="0" #Para salir del segundo menu
		separador
	else
		exit
	fi
}
trap ctrl_c SIGINT

menu_tres(){
	opcion1="ninguna"
	while [ $opcion1 != "0" ]
		do 
		
		echo -e "\n\n\nAlgoritmo Memoria"
		echo -e "\t1) Particiones Dinamicas"
		echo -e "\t2) Buddy System"
		echo -e "\n\t0) Salir\n"
		echo -e "\tOpcion: \c"

		read -rn1 opcion1

		echo -e "\n\n"

		if [ -z $opcion1 ] ; then opcion1="ninguna" ; fi

		case $opcion1 in
			"1" )
				CONFIG=configs/final_particiones.cfg
				ejecutar
				;;

			"2" ) 
				CONFIG=configs/final_buddy.cfg
				ejecutar
				;;
	   
			
			"0" ) 
				clear
				return
				;;   
	   
			* )  
				;;

		esac
	done

}

menu_dos(){
	opcion1="ninguna"
	while [ $opcion1 != "0"  ]
		do 
		
		echo -e "\n\n\nAlgoritmo de Reemplazo"
		echo -e "\t1) FIFO"
		echo -e "\t2) LRU"
		echo -e "\n\t0) Salir\n"
		echo -e "\tOpcion: \c"

		read -rn1 opcion1

		echo -e "\n\n"

		if [ -z $opcion1 ] ; then opcion1="ninguna" ; fi

		case $opcion1 in
			"1" )
				lru_a_fifo $CONFIG
				ejecutar
				;;

			"2" ) 
				fifo_a_lru $CONFIG
				ejecutar
				;;
	   
			
			"0" ) 
				clear
				return
				;;   
	   
			* )  
				;;

		esac
	done

}

menu(){
	opcion="ninguna"
	while [ $opcion != "0"  ]
		do 
		
		encabezado 0
		

		read -rn1 opcion

		if [ -z $opcion ] ; then opcion="ninguna" ; fi

		case $opcion in 
			"1" )
				consolidacion
				menu_dos
	  			;;

			"2" ) 
				compactacion
				menu_dos
				;;
	   
			"3" ) 
				buddy
				menu_dos
				;;

			"4" )
				menu_tres
				;;   

			[bB] )
				echo -e "\n"
				separador
				buildear
				separador
				;;   
			
			[q0] ) 
				exit
				;;

			* )
				clear  
				;;
		esac
		separador
	done
		/usr/bin/clear
}

#clear
menu
