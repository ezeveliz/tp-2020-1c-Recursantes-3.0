#!/bin/bash

clear

CONFIG=configs/

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
				./broker $CONFIG
				;;

			"2" ) 
				fifo_a_lru $CONFIG
				./broker $CONFIG
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


opcion="ninguna"
while [ $opcion != "0"  ]
	do 
	
	echo -e "\nPruebas Broker"
	echo -e "\t1) Consolidacion"
	echo -e "\t2) Compactacion"
	echo -e "\t3) Buddy System"
	echo -e "\t4) Prueba Final"
	echo -e "\n\tb) Buildear Broker\n"
	echo -e "\t0) Salir de la herramienta\n"
	echo -e "\tOpcion: \c"

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
			echo -e "\n\n"
			buddy
			./broker $CONFIG
			return
			;;

		[bB] )
			echo -e "\n\n"
			./build_broker.sh
			echo -e "\n\n=^..^=   =^..^=   =^..^=    =^..^=    =^..^=    =^..^=    =^..^=\n"
			;;   
		
		"0" ) 
			exit
			;;

		* )
			clear  
			;;
	esac
done
	/usr/bin/clear
