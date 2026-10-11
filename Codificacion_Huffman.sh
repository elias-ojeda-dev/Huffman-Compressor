#!/bin/bash
#****************************************************************
# Codificacion_Huffman.sh
# V 1.0 14 Noviembre 2024
# Grupo: 3CM4
# Curso: Análisis y diseño de algoritmos
# (C) 14 de Noviembre de 2024
# ESCOM-IPN
# Ejemplo de medición de tiempo en C y recepción de parámetros en C bajo UNIX utilizando script
# Compilación: chmod +x Codificacion_Huffman.sh (Da permisos para ejecutar el script)
# Ejecución (Linux y MAC OS): "./Codificacion_Huffman.sh"
# Se ejecuta desde la carpeta que contiene los archivos .c y archivos_a_codificar.txt (un archivo por línea).
# En MAC OS se puede ocupar "caffeinate ./Codificacion_Huffman.sh" para mantener a la máquina activa mientras esté en ejecución el script.
# Este script está diseñado para ejecutar la codificación y decodificación de diferentes archivos de distintos tamaños,
# en donde posteriormente se realiza la determinación de las estadísticas como el tiempo de ejecución y el porcentaje de compresión.
#****************************************************************

file_list="archivos_a_codificar.txt"

if [ ! -f "$file_list" ]; then
    echo "No se encontró $file_list. Saliendo..."
    exit 1
fi

codificacion="codificacion.txt"
decodificacion="decodificacion.txt"

> "$codificacion"
> "$decodificacion"

gcc Codificacion_Huffman.c tiempo.c -o Codificacion_Huffman && gcc Decodificacion_Huffman.c tiempo.c -o Decodificacion_Huffman

if [ $? -ne 0 ]; then
    echo "Error en la compilación de los programas. Saliendo..."
    exit 1
fi

echo

cr=$(printf '\r')

while IFS= read -r archivo || [ -n "$archivo" ]
do
    archivo=${archivo%"$cr"}
    [ -z "$archivo" ] && continue

    nombre=${archivo##*/}
    base_name=$archivo
    extension=""
    case ${nombre#?} in
        *.*)
            base_name=${archivo%.*}
            extension=${archivo##*.}
            ;;
    esac

    cod_output="${base_name}_codificacion.dat"
    freq_output="${base_name}_frecuencias.txt"
    dec_output="${base_name}_frecuencias_decodificado${extension:+.$extension}"

    echo "Codificando el archivo: $archivo" | tee -a "$codificacion"
    
    ./Codificacion_Huffman "$archivo" >> "$codificacion" 2>&1
    
    if [ $? -ne 0 ]; then
        echo "Error al codificar el archivo: $archivo. Saliendo..." | tee -a "$codificacion"
        exit 1
    fi

    echo "Decodificando el archivo: $archivo" | tee -a "$decodificacion"

    
    ./Decodificacion_Huffman "$cod_output" "$freq_output" >> "$decodificacion" 2>&1

    if [ $? -ne 0 ]; then
        echo "Error al decodificar el archivo: $archivo. Saliendo..." | tee -a "$decodificacion"
        exit 1
    fi

    if ! cmp -s "$archivo" "$dec_output"; then
        echo "El archivo decodificado no coincide con el original: $archivo. Saliendo..." | tee -a "$decodificacion"
        exit 1
    fi

    echo "Archivo $archivo procesado con éxito."

done < "$file_list"

echo "Ejecución completa. Todos los archivos procesados."