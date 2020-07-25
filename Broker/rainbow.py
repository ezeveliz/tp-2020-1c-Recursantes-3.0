#!/usr/bin/env python2
import sys
import random

colores = [
	"\033[0;31m",	#red
	"\033[0;33m",	#orange
	#"\033[1;33m",	#yellow
	"\033[0;32m",	#green
	"\033[0;36m",	#cyan
	"\033[0;34m",	#blue
	"\033[0;35m"	#purple
]
nc = "\033[0m"		#no color



def rainbow(string, cambio):
	cambio = int(cambio)
	rainbow_string = ""
	i = random.randint(0,100)
	linea = i
	for char in range(0, len(string)):
		if string[char] == '\n':
			linea = linea + 1
			i=linea

		indice = i%len(colores)
		color = colores[indice]
		rainbow_string = rainbow_string + color + string[char]
		if string[char] == '\n':
			continue
			
		if char%cambio==0:
			i = i + 1

			

	rainbow_string = rainbow_string + nc

	return rainbow_string


string = ' '.join(sys.argv[2:])

string = string.decode("utf-8").replace(u"\xe2",u"\u2588")

#if "\xe2" in string:
#	print string.decode("utf-8").replace(u"\xe2",u"\u2588")

print u"{}".format(rainbow(string, sys.argv[1]))