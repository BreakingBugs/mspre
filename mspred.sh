#!/bin/bash

### BEGIN INIT INFO
# Provides:          mspre
# Required-Start:    mspred
# Required-Stop:     mspred
# Default-Start:     
# Default-Stop:      
# Short-Description: asegura que el inicio del daemon mspre
### END INIT INFO

SERVER=mspre
PID=$(pidof $SERVER);
SDIR=""
args=("$@")

start() {
	if [ -z "$PID" ]; then
		if [ -n "${args[1]}" ]; then
			SDIR="${args[1]}"
			if [ -d "$SDIR" ]; then
				cd "$SDIR"
			else
				echo "El directorio no es valido"
				return
			fi
		fi
		if [ -f $SERVER ]; then
			LAUNCH=./$SERVER 
			echo "Iniciando el servidor"
#			Para ejecutar en el fondo
			${LAUNCH} > /dev/null 2>&1 &
#			${LAUNCH}
			echo "Servidor iniciado"
		else
			echo "No se encontro el ejecutable $SERVER"
		fi
	else
		echo "El servidor ya esta corriendo"
	fi
}

stop() {
	if [ -z "$PID" ]; then
		echo "Server isn't running"
	else
		echo "Deteniendo el servidor"
		kill -SIGINT "$PID"
		while pgrep "^$SERVER$" > /dev/null; do sleep 1; done
		PID=0;
		echo "El servidor ha sido detenido"
	fi
}

restart() {
	echo "Reiniciando..."
	stop
	if [ -n "${args[1]}" ]; then
		SDIR="${args[1]}"
		if [ -d "$SDIR" ]; then
			cd "$SDIR"
		else
			echo "El directorio no es valido"
			return
		fi
	fi
	if [ -f $SERVER ]; then
		LAUNCH=./$SERVER 
		echo "Iniciando el servidor"
#			Para ejecutar en el fondo
#			${LAUNCH} > /dev/null 2>&1 &
		${LAUNCH}
		echo "Servidor reiniciado"
	else
		echo "No se encontro el ejecutable $SERVER"
	fi
}

status() {
	if [ -z "$PID" ]; then
		echo "El servidor esta apagado"
	else
		echo "El servidor esta corriendo"
		echo "PID: $PID"
	fi
}

reload() {
	if [ -z "$PID" ]; then
		echo "El servidor esta apagado"
	else
		echo "Recargando el servidor"
				kill -SIGHUP "$PID"
		echo "Servidor recargado"
	fi
}

usage() {
	echo "Usage: $SERVER.sh COMMAND"
	echo "   or: $SERVER.sh COMMAND SERVER_DIR"
	echo
}

hilfe() {
	usage
	echo "-h,help     muestra esta ayuda."
	echo "start	    inicia el servidor. Si no se especifica SERVER_DIR,"
	echo "	      se asume que el ejecutable esta en el directorio actual."
	echo "stop        detiene el servidor."
	echo "restart     reinicia el servidor, esto implica un start y un stop."
	echo "reload      recarga la configuracion del servidor."
	echo "status      muestra el estado del servidor."
}

if [ "$#" -eq 1 -o "$#" -eq 2 ]; then
	case "$1" in
		"start") start;;
		"stop") stop;;
		"restart") restart;;
		"reload") reload;;
		"status") status;;
		"-h"|"help") hilfe;;
		*) echo "Comando invalido. Usar el comando help."; usage;;
	esac
else
	usage
fi
