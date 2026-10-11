//*****************************************************************
//DECODIFICACION_HUFFMAN.C
//*****************************************************************
//*****************************************************************
//Grupo: 3CM4
//Curso: Análisis y diseño de algoritmos
//(C) Noviembre 2024
//ESCOM-IPN
//Práctica 03: Codificación voraz de Huffman. Reconstruye el árbol de
//Huffman a partir de la tabla de frecuencias y recupera el archivo
//original.
//Compilación: "gcc -std=gnu11 -Wall -Wextra Decodificacion_Huffman.c tiempo.c -o Decodificacion_Huffman"
//Ejecución: "./Decodificacion_Huffman <archivo_codificado> <archivo_frecuencias>" (Linux y MAC OS)
//Ejemplo: "./Decodificacion_Huffman ejemplo_codificacion.dat ejemplo_frecuencias.txt"
//genera ejemplo_frecuencias_decodificado.txt
//*****************************************************************


//*****************************************************************
//Librerías incluidas
//*****************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "tiempo.h"

//*****************************************************************
//Nodo (Estructura)
//*****************************************************************
//Descripción: Asocia un byte con su frecuencia. Cada línea válida de
//la tabla de frecuencias se guarda en un Nodo.
//
//Campos:
//  - caracter (tipo char): Byte leído de la tabla.
//  - frecuencia (tipo long long): Número de apariciones del byte en el
//    archivo original.
//*****************************************************************
typedef struct
{
    char caracter;
    long long frecuencia;
} Nodo;

//*****************************************************************
//Arbol (Estructura)
//*****************************************************************
//Descripción: Nodo del árbol de Huffman. Una hoja representa un byte y
//guarda su frecuencia; un nodo interno guarda la suma de las
//frecuencias de sus dos hijos. Las hojas se reconocen porque no tienen
//hijos y no por su caracter, ya que el byte 0 también es un símbolo
//válido.
//
//Campos:
//  - frecuencia (tipo long long): Frecuencia de la hoja o suma de las
//    frecuencias del subárbol.
//  - caracter (tipo char): Byte de la hoja; en un nodo interno es '\0'.
//  - izq (tipo struct Arbol *): Hijo izquierdo, al que se baja con un
//    bit 0; NULL en una hoja.
//  - der (tipo struct Arbol *): Hijo derecho, al que se baja con un bit
//    1; NULL en una hoja.
//*****************************************************************
typedef struct Arbol
{
    long long frecuencia;
    char caracter;
    struct Arbol *izq, *der;
} Arbol;

//*****************************************************************
//Declaración de funciones
//*****************************************************************
void InicializarFrecuencias(Nodo *frecuencias);
void Intercambiar(Arbol** a, Arbol** b);
int Particion(Arbol** arboles, int inicio, int fin);
void QuickSortArboles(Arbol** arboles, int inicio, int fin);
Arbol* ConstruirArbolHuffman(Nodo *frecuencias, int n);
Arbol* CrearNodoArbol(char caracter, long long frecuencia);
Arbol* FusionarArboles(Arbol* arbol1, Arbol* arbol2);
void LiberarArbol(Arbol *raiz);
int LeerArchivo(char *archivo_frecuencia, Nodo *frecuencias, char *extension, size_t extension_len);
void DecodificarArchivo(Arbol* raiz, char* archivo_codificado, char* archivo_salida);
void NombreBase(char *nombreArchivo, char **nombreSinExtension);

//*****************************************************************
//main (Definición)
//*****************************************************************
//Descripción: Programa principal del decodificador. Valida los
//argumentos, lee la tabla de frecuencias y forma el nombre de salida
//<base>_decodificado.<extensión>, donde base es la ruta de la tabla
//sin extensión; si el archivo original no tenía extensión, el nombre
//termina en _decodificado. Reconstruye el árbol de Huffman y recupera
//el archivo. Solo la llamada a DecodificarArchivo se mide con uswtime;
//se imprimen la ruta y el tamaño del archivo recuperado y los tiempos
//real, user, sys y CPU/Wall en notación decimal y científica, y al
//final se libera la memoria utilizada.
//
//Recibe:
//  - argc (tipo int): Número de argumentos; debe ser 3.
//  - argv (tipo char **): argv[1] es el archivo codificado (.dat) y
//    argv[2] la tabla de frecuencias (.txt).
//Devuelve:
//  - (tipo int): 0 si el proceso termina correctamente. Si faltan
//    argumentos o falla la apertura o lectura de un archivo, el
//    programa termina con exit(1).
//*****************************************************************
int main(int argc, char** argv)
{
    double utime0, stime0, wtime0, utime1, stime1, wtime1;
    char *archivo_codificado, *archivo_frecuencia;
    char extension[64]={0};
    Nodo *frecuencias=(Nodo*)malloc(256*sizeof(Nodo));
    char *salida;
    int n;

    if(argc!=3){
        printf("Uso: %s <archivo_codificado> <archivo_frecuencias>\n", argv[0]);
        exit(1);
    }else{
        archivo_codificado=(char*)malloc(strlen(argv[1])+1);
        strcpy(archivo_codificado, argv[1]);
        archivo_frecuencia=(char*)malloc(strlen(argv[2])+1);
        strcpy(archivo_frecuencia, argv[2]);
    }

    InicializarFrecuencias(frecuencias);
    n=LeerArchivo(archivo_frecuencia, frecuencias, extension, sizeof(extension));

    char *base=NULL;
    NombreBase(archivo_frecuencia, &base);

    size_t salida_len=strlen(base)+sizeof("_decodificado.")+strlen(extension);
    salida=(char*)malloc(salida_len);
    if(extension[0]!='\0'){
        snprintf(salida, salida_len, "%s_decodificado.%s", base, extension);
    }else{
        snprintf(salida, salida_len, "%s_decodificado", base);
    }

    Arbol *arbolHuffman=ConstruirArbolHuffman(frecuencias, n);

    uswtime(&utime0, &stime0, &wtime0);

    DecodificarArchivo(arbolHuffman, archivo_codificado, salida);

    uswtime(&utime1, &stime1, &wtime1);

    printf("\nProceso completado exitosamente:\n");
    printf("1. Archivo decodificado guardado en: %s\n", salida);

    struct stat st;
    if(stat(salida, &st)==0){
        printf("2. Tamaño del archivo decodificado: %lld bytes\n", (long long)st.st_size);
    }else{
        printf("Error al obtener el tamaño del archivo.\n");
    }

    printf("\n");
    printf("real (Tiempo total) %.10f s\n", wtime1-wtime0);
    printf("user (Tiempo de procesamiento en CPU) %.10f s\n", utime1-utime0);
    printf("sys (Tiempo en acciones de E/S) %.10f s\n", stime1-stime0);
    printf("CPU/Wall   %.10f %% \n",100.0*(utime1-utime0+stime1-stime0)/(wtime1-wtime0));
    printf("\n");

    printf("\n");
    printf("real (Tiempo total) %.10e s\n", wtime1-wtime0);
    printf("user (Tiempo de procesamiento en CPU) %.10e s\n", utime1-utime0);
    printf("sys (Tiempo en acciones de E/S) %.10e s\n", stime1-stime0);
    printf("CPU/Wall   %.10f %% \n",100.0*(utime1-utime0+stime1-stime0)/(wtime1-wtime0));
    printf("\n");

    free(frecuencias);
    free(archivo_codificado);
    free(archivo_frecuencia);
    free(base);
    free(salida);
    LiberarArbol(arbolHuffman);

    return 0;
}

//*****************************************************************
//NombreBase (Definición)
//*****************************************************************
//Descripción: Obtiene la ruta sin extensión que sirve de base para el
//nombre de salida. Aplica la misma regla que el codificador: quita el
//texto desde el último punto del nombre del archivo (la parte
//posterior a la última '/'), salvo que el nombre no tenga punto o que
//ese punto sea su primer carácter (archivo oculto). La cadena se
//reserva con memoria dinámica y quien llama a la función debe
//liberarla.
//
//Recibe:
//  - nombreArchivo (tipo char *): Ruta del archivo.
//  - nombreSinExtension (tipo char **): Dirección donde se guarda la
//    ruta sin extensión.
//Devuelve: Nada.
//*****************************************************************
void NombreBase(char *nombreArchivo, char **nombreSinExtension)
{
    char *nombre=strrchr(nombreArchivo, '/');
    if(nombre){
        nombre++;
    }else{
        nombre=nombreArchivo;
    }
    char *punto=strrchr(nombre, '.');

    if(punto && punto!=nombre){
        int longitudNombre=punto-nombreArchivo;

        *nombreSinExtension=(char*)malloc(longitudNombre+1);
        strncpy(*nombreSinExtension, nombreArchivo, longitudNombre);
        (*nombreSinExtension)[longitudNombre]='\0';
    }else{
        *nombreSinExtension=strdup(nombreArchivo);
    }
}

//*****************************************************************
//InicializarFrecuencias (Definición)
//*****************************************************************
//Descripción: Prepara el arreglo de frecuencias: la posición i guarda
//el byte i (0 a 255) con frecuencia 0. LeerArchivo sobrescribe después
//las primeras posiciones con los símbolos de la tabla; las demás
//conservan la frecuencia 0 y ConstruirArbolHuffman las ignora.
//
//Recibe:
//  - frecuencias (tipo Nodo *): Arreglo de 256 posiciones a inicializar.
//Devuelve: Nada.
//*****************************************************************
void InicializarFrecuencias(Nodo *frecuencias)
{
    int i;
    for(i=0; i<256; i++){
        frecuencias[i].caracter=(char)i;
        frecuencias[i].frecuencia=0;
    }
}

//*****************************************************************
//DecodificarArchivo (Definición)
//*****************************************************************
//Descripción: Recupera el archivo original. Lee de la cabecera del
//archivo codificado el número de bytes originales (long long nativo)
//y recorre el árbol con cada bit, del más significativo al menos
//significativo de cada byte: con 0 baja al hijo izquierdo y con 1 al
//derecho. Al llegar a una hoja escribe su byte y regresa a la raíz. Se
//detiene en cuanto escribe el número de bytes de la cabecera, por lo
//que ignora el relleno del último byte. Si el árbol es una sola hoja,
//escribe ese byte el número de veces indicado sin leer bits. Termina
//el programa con exit(1) si no puede abrir un archivo, si falta la
//cabecera o si hay bytes por recuperar pero el árbol está vacío.
//
//Recibe:
//  - raiz (tipo Arbol *): Raíz del árbol reconstruido; es NULL si la
//    tabla no tiene símbolos (archivo original vacío).
//  - archivo_codificado (tipo char *): Ruta del archivo comprimido
//    (.dat).
//  - archivo_salida (tipo char *): Ruta del archivo recuperado.
//Devuelve: Nada.
//*****************************************************************
void DecodificarArchivo(Arbol* raiz, char* archivo_codificado, char* archivo_salida)
{
    FILE *entrada=fopen(archivo_codificado, "rb");
    if(!entrada){
        printf("Error al abrir el archivo codificado.\n");
        exit(1);
    }

    long long tamanoOriginal=0;
    if(fread(&tamanoOriginal, sizeof(long long), 1, entrada)!=1){
        printf("El archivo codificado está vacío o dañado.\n");
        fclose(entrada);
        exit(1);
    }

    if(raiz==NULL && tamanoOriginal>0){
        printf("No se encontraron frecuencias válidas en el archivo.\n");
        fclose(entrada);
        exit(1);
    }

    FILE *salida=fopen(archivo_salida, "wb");
    if(!salida){
        printf("Error al abrir el archivo de salida.\n");
        fclose(entrada);
        exit(1);
    }

    Arbol *nodoactual=raiz;
    int bit, i;
    long long escritos=0;

    if(raiz!=NULL && raiz->izq==NULL && raiz->der==NULL){
        while(escritos<tamanoOriginal){
            fputc(raiz->caracter, salida);
            escritos++;
        }
    }

    while(escritos<tamanoOriginal && (bit=fgetc(entrada))!=EOF){
        for(i=7; i>=0 && escritos<tamanoOriginal; i--){
            int bitactual=(bit>>i)&1;
            if(bitactual==0){
                nodoactual=nodoactual->izq;
            }else{
                nodoactual=nodoactual->der;
            }

            if(nodoactual->izq==NULL && nodoactual->der==NULL){
                fputc(nodoactual->caracter, salida);
                escritos++;
                nodoactual=raiz;
            }
        }
    }

    fclose(entrada);
    fclose(salida);
}

//*****************************************************************
//LeerArchivo (Definición)
//*****************************************************************
//Descripción: Lee la tabla de frecuencias generada por el codificador.
//Descarta la primera línea (encabezado), copia la segunda en extension
//sin el salto de línea y después interpreta cada línea con el formato
//"'c' frecuencia valor" (byte imprimible) o "0xHH frecuencia valor"
//(byte no imprimible). Las líneas válidas se guardan en posiciones
//consecutivas de frecuencias, desde la 0 y en el orden de la tabla;
//las que no tienen ninguno de los dos formatos se ignoran. Lee como
//máximo 256 símbolos. Termina el programa con exit(1) si no puede
//abrir la tabla o si le faltan las dos primeras líneas.
//
//Recibe:
//  - archivo_frecuencia (tipo char *): Ruta de la tabla de frecuencias.
//  - frecuencias (tipo Nodo *): Arreglo de 256 posiciones, inicializado
//    con InicializarFrecuencias, donde se guardan los símbolos leídos.
//  - extension (tipo char *): Búfer donde se guarda la extensión del
//    archivo original, sin punto; queda vacío si no tenía extensión.
//  - extension_len (tipo size_t): Tamaño en bytes del búfer extension.
//Devuelve:
//  - (tipo int): Número de símbolos leídos, de 0 a 256.
//*****************************************************************
int LeerArchivo(char *archivo_frecuencia, Nodo *frecuencias, char *extension, size_t extension_len)
{
    FILE *archivo=fopen(archivo_frecuencia, "r");
    if(archivo==NULL){
        printf("Error al abrir archivo.\n");
        exit(1);
    }
    char buffer[100];
    int i=0;

    if(fgets(buffer, sizeof(buffer), archivo)==NULL || fgets(extension, extension_len, archivo)==NULL){
        printf("La tabla de frecuencias está vacía o dañada.\n");
        fclose(archivo);
        exit(1);
    }
    extension[strcspn(extension, "\n")]='\0';

    while(i<256 && fgets(buffer, sizeof(buffer), archivo)){
        char caracter;
        long long frecuencia=0;
        int ascii=0;
        unsigned int hexadecimal=0;

        if(sscanf(buffer, "'%c' %lld %d", &caracter, &frecuencia, &ascii)==3){
            frecuencias[i].caracter=caracter;
            frecuencias[i].frecuencia=frecuencia;
            i++;
        }else if(sscanf(buffer, "0x%x %lld %d", &hexadecimal, &frecuencia, &ascii)==3){
            frecuencias[i].caracter=(char)hexadecimal;
            frecuencias[i].frecuencia=frecuencia;
            i++;
        }
    }
    fclose(archivo);
    return i;
}

//*****************************************************************
//Intercambiar (Definición)
//*****************************************************************
//Descripción: Intercambia los dos apuntadores a árbol cuyas
//direcciones recibe. La utiliza Particion durante el QuickSort.
//
//Recibe:
//  - a (tipo Arbol **): Dirección del primer apuntador.
//  - b (tipo Arbol **): Dirección del segundo apuntador.
//Devuelve: Nada.
//*****************************************************************
void Intercambiar(Arbol** a, Arbol** b)
{
    Arbol* temp=*a;
    *a=*b;
    *b=temp;
}

//*****************************************************************
//Particion (Definición)
//*****************************************************************
//Descripción: Partición de Lomuto para el QuickSort. Toma como pivote
//la frecuencia del árbol en la posición fin, mueve a la izquierda los
//árboles con frecuencia estrictamente menor y coloca el pivote justo
//después de ellos.
//
//Recibe:
//  - arboles (tipo Arbol **): Arreglo de apuntadores a árboles.
//  - inicio (tipo int): Índice inicial del subarreglo.
//  - fin (tipo int): Índice final del subarreglo y posición del pivote.
//Devuelve:
//  - (tipo int): Posición final del pivote, entre inicio y fin.
//*****************************************************************
int Particion(Arbol** arboles, int inicio, int fin)
{
    long long pivot=arboles[fin]->frecuencia;
    int i=inicio-1;

    for(int j=inicio; j<fin; j++){
        if(arboles[j]->frecuencia<pivot){
            i++;
            Intercambiar(&arboles[i], &arboles[j]);
        }
    }
    Intercambiar(&arboles[i+1], &arboles[fin]);
    return i+1;
}

//*****************************************************************
//QuickSortArboles (Definición)
//*****************************************************************
//Descripción: Ordena con QuickSort recursivo, de menor a mayor
//frecuencia, los árboles del subarreglo [inicio, fin]: lo divide con
//Particion y ordena por separado las partes a la izquierda y a la
//derecha del pivote.
//
//Recibe:
//  - arboles (tipo Arbol **): Arreglo de apuntadores a árboles.
//  - inicio (tipo int): Índice inicial del subarreglo.
//  - fin (tipo int): Índice final del subarreglo; si no es mayor que
//    inicio, no hay nada que ordenar.
//Devuelve: Nada.
//*****************************************************************
void QuickSortArboles(Arbol** arboles, int inicio, int fin)
{
    if(inicio<fin){
        int pivote=Particion(arboles, inicio, fin);
        QuickSortArboles(arboles, inicio, pivote-1);
        QuickSortArboles(arboles, pivote+1, fin);
    }
}

//*****************************************************************
//ConstruirArbolHuffman (Definición)
//*****************************************************************
//Descripción: Reconstruye el árbol de Huffman con el mismo
//procedimiento voraz del codificador. Crea una hoja por cada posición
//con frecuencia mayor que cero, en el orden de la tabla. Mientras
//quede más de un árbol, los ordena con QuickSortArboles y fusiona los
//dos de menor frecuencia: el primero queda como hijo izquierdo y el
//segundo como derecho. Como la tabla conserva el orden creciente de
//bytes que usó el codificador, el árbol resultante es el mismo aun
//cuando hay frecuencias iguales.
//
//Recibe:
//  - frecuencias (tipo Nodo *): Arreglo de 256 posiciones llenado por
//    LeerArchivo.
//  - n (tipo int): Número de símbolos leídos de la tabla; es la
//    capacidad del arreglo auxiliar de árboles.
//Devuelve:
//  - (tipo Arbol *): Raíz del árbol de Huffman, o NULL si la tabla no
//    tiene símbolos (archivo original vacío).
//*****************************************************************
Arbol* ConstruirArbolHuffman(Nodo *frecuencias, int n)
{
    Arbol **arboles=(Arbol**)malloc(n*sizeof(Arbol*));
    int numArboles=0;

    for(int i=0; i<256; i++){
        if(frecuencias[i].frecuencia>0){
            arboles[numArboles++]=CrearNodoArbol(frecuencias[i].caracter, frecuencias[i].frecuencia);
        }
    }

    if(numArboles==0){
        free(arboles);
        return NULL;
    }

    while(numArboles>1){
        QuickSortArboles(arboles, 0, numArboles-1);

        Arbol *arbolFusionado=FusionarArboles(arboles[0], arboles[1]);

        arboles[0]=arbolFusionado;
        for(int i=1; i<numArboles-1; i++){
            arboles[i]=arboles[i+1];
        }
        numArboles--;
    }

    Arbol *arbolHuffman=arboles[0];
    free(arboles);
    return arbolHuffman;
}

//*****************************************************************
//CrearNodoArbol (Definición)
//*****************************************************************
//Descripción: Reserva memoria para una hoja del árbol de Huffman y le
//asigna el byte y su frecuencia; sus dos hijos quedan en NULL.
//
//Recibe:
//  - caracter (tipo char): Byte que representa la hoja.
//  - frecuencia (tipo long long): Número de apariciones del byte.
//Devuelve:
//  - (tipo Arbol *): Apuntador a la hoja creada.
//*****************************************************************
Arbol* CrearNodoArbol(char caracter, long long frecuencia)
{
    Arbol *nuevoNodo=(Arbol*)malloc(sizeof(Arbol));
    nuevoNodo->caracter=caracter;
    nuevoNodo->frecuencia=frecuencia;
    nuevoNodo->izq=nuevoNodo->der=NULL;
    return nuevoNodo;
}

//*****************************************************************
//FusionarArboles (Definición)
//*****************************************************************
//Descripción: Crea un nodo interno cuyos hijos son los dos árboles
//recibidos y cuya frecuencia es la suma de las frecuencias de ambos.
//El nodo interno no representa ningún byte, por eso su caracter es
//'\0'.
//
//Recibe:
//  - arbol1 (tipo Arbol *): Árbol que queda como hijo izquierdo.
//  - arbol2 (tipo Arbol *): Árbol que queda como hijo derecho.
//Devuelve:
//  - (tipo Arbol *): Apuntador al nuevo nodo interno.
//*****************************************************************
Arbol* FusionarArboles(Arbol* arbol1, Arbol* arbol2)
{
    Arbol *nuevoNodo=(Arbol*)malloc(sizeof(Arbol));
    nuevoNodo->frecuencia=arbol1->frecuencia+arbol2->frecuencia;
    nuevoNodo->caracter='\0';
    nuevoNodo->izq=arbol1;
    nuevoNodo->der=arbol2;
    return nuevoNodo;
}

//*****************************************************************
//LiberarArbol (Definición)
//*****************************************************************
//Descripción: Libera recursivamente la memoria del árbol en postorden:
//primero los subárboles izquierdo y derecho y al final la raíz.
//
//Recibe:
//  - raiz (tipo Arbol *): Raíz del árbol a liberar; si es NULL, no
//    hace nada.
//Devuelve: Nada.
//*****************************************************************
void LiberarArbol(Arbol *raiz)
{
    if(!raiz) return;
    LiberarArbol(raiz->izq);
    LiberarArbol(raiz->der);
    free(raiz);
}
