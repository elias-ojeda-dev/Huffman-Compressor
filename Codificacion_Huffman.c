//*****************************************************************
//CODIFICACION_HUFFMAN.C
//*****************************************************************
//*****************************************************************
//Grupo: 3CM4
//Curso: Análisis y diseño de algoritmos
//(C) Noviembre 2024
//ESCOM-IPN
//Práctica 03: Codificación voraz de Huffman. Comprime un archivo de
//cualquier tipo byte por byte con un árbol de Huffman y guarda aparte
//la tabla de frecuencias.
//Compilación: "gcc -std=gnu11 -Wall -Wextra Codificacion_Huffman.c tiempo.c -o Codificacion_Huffman"
//Ejecución: "./Codificacion_Huffman <archivo_original>" (Linux y MAC OS)
//Ejemplo: "./Codificacion_Huffman ejemplo.txt" genera
//ejemplo_codificacion.dat y ejemplo_frecuencias.txt
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
//Manejo de bits (Macros)
//*****************************************************************
//Descripción: Operaciones sobre el bit bpos de un byte, donde 0 es el
//bit menos significativo y 7 el más significativo.
//
//Macros:
//  - PESOBIT(bpos): Máscara con un 1 únicamente en la posición bpos.
//  - CONSULTARBIT(var,bpos): Valor (0 o 1) del bit bpos de var.
//  - PONE_1(var,bpos): Pone en 1 el bit bpos de la variable var.
//  - PONE_0(var,bpos): Pone en 0 el bit bpos de la variable var.
//*****************************************************************
#define PESOBIT(bpos) (1u<<(bpos))
#define CONSULTARBIT(var,bpos) (((unsigned char)(var)>>(bpos))&1)
#define PONE_1(var,bpos) do{ (var)=(unsigned char)((var)|(unsigned char)PESOBIT(bpos)); }while(0)
#define PONE_0(var,bpos) do{ (var)=(unsigned char)((var)&(unsigned char)~PESOBIT(bpos)); }while(0)

//*****************************************************************
//Nodo (Estructura)
//*****************************************************************
//Descripción: Asocia un byte con el número de veces que aparece en el
//archivo. El conteo usa un arreglo de 256 Nodo indexado por el valor
//del byte.
//
//Campos:
//  - caracter (tipo char): Byte al que corresponde la posición.
//  - frecuencia (tipo long long): Número de apariciones del byte.
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
//  - izq (tipo struct Arbol *): Hijo izquierdo, que corresponde al bit
//    0; NULL en una hoja.
//  - der (tipo struct Arbol *): Hijo derecho, que corresponde al bit 1;
//    NULL en una hoja.
//*****************************************************************
typedef struct Arbol
{
    long long frecuencia;
    char caracter;
    struct Arbol *izq, *der;
} Arbol;

//*****************************************************************
//CodigoHuffman (Estructura)
//*****************************************************************
//Descripción: Código de Huffman de un byte, guardado como una
//secuencia de bits empaquetada.
//
//Campos:
//  - codigo (tipo unsigned char *): Arreglo de 32 bytes (256 bits); el
//    primer bit del código es el más significativo de codigo[0].
//  - longitud (tipo long long): Número de bits del código; 0 si el byte
//    no aparece en el archivo.
//*****************************************************************
typedef struct
{
    unsigned char *codigo;
    long long longitud;
} CodigoHuffman;

//*****************************************************************
//BufferBits (Estructura)
//*****************************************************************
//Descripción: Acumula bits hasta completar un byte, que es la unidad
//mínima que se puede escribir en el archivo comprimido.
//
//Campos:
//  - buffer (tipo unsigned char): Byte en construcción; se llena del
//    bit más significativo al menos significativo.
//  - contador (tipo int): Número de bits ya colocados en buffer (0 a 7).
//  - archivo (tipo FILE *): Archivo donde se escriben los bytes.
//*****************************************************************
typedef struct
{
    unsigned char buffer;
    int contador;
    FILE *archivo;
} BufferBits;

//*****************************************************************
//tablaHuffman (Variable global)
//*****************************************************************
//Descripción: Tabla de códigos indexada por el valor del byte (0 a
//255). InicializarTablaHuffman reserva sus arreglos,
//GenerarCodigosHuffman la llena, CodificarArchivo la consulta y
//LiberarTablaHuffman libera su memoria.
//*****************************************************************
CodigoHuffman tablaHuffman[256];

//*****************************************************************
//Declaración de funciones
//*****************************************************************
void LeerArchivo_Frecuencia(char *archivo, Nodo *frecuencias);
void InicializarFrecuencias(Nodo *frecuencias);
void Intercambiar(Arbol** a, Arbol** b);
int Particion(Arbol** arboles, int inicio, int fin);
void QuickSortArboles(Arbol** arboles, int inicio, int fin);
Arbol* ConstruirArbolHuffman(Nodo *frecuencias, int n);
Arbol* CrearNodoArbol(char caracter, long long frecuencia);
Arbol* FusionarArboles(Arbol* arbol1, Arbol* arbol2);
void LiberarArbol(Arbol *raiz);
void InicializarTablaHuffman();
void LiberarTablaHuffman();
void GenerarCodigosHuffman(Arbol* raiz, unsigned char* codigoActual, int profundidad);
void InicializarBufferBits(BufferBits *bb, FILE *archivo);
void FlushBuffer(BufferBits *bb);
void EscribirBit(BufferBits *bb, int bit);
void EscribirBits(BufferBits *bb, unsigned char *codigo, int longitud);
void ManejarUltimoByte(BufferBits *bb);
void CodificarArchivo(char *archivoEntrada, char *archivoSalida, char *archivoFrecuencias);
void EscribirTablaFrecuencias(Nodo *frecuencias, const char *nombreArchivo, char *extension);
long long obtenerTamanoArchivo(const char *nombreArchivo);
void NombreBase(char *nombreArchivo, char **nombreSinExtension, char **extension);

//*****************************************************************
//main (Definición)
//*****************************************************************
//Descripción: Programa principal del codificador. Valida el argumento,
//cuenta la frecuencia de cada byte del archivo, escribe la tabla de
//frecuencias en <base>_frecuencias.txt, construye el árbol de Huffman
//y genera los códigos. Después escribe el archivo comprimido en
//<base>_codificacion.dat, donde base es la ruta de entrada sin
//extensión. Solo la llamada a CodificarArchivo se mide con uswtime; se
//imprimen los tiempos real, user, sys y CPU/Wall en notación decimal y
//científica, y al final se libera la memoria utilizada.
//
//Recibe:
//  - argc (tipo int): Número de argumentos; debe ser 2.
//  - argv (tipo char **): argv[1] es la ruta del archivo a comprimir.
//Devuelve:
//  - (tipo int): 0 si el proceso termina correctamente. Si falta el
//    argumento o falla la apertura de un archivo, el programa termina
//    con exit(1).
//*****************************************************************
int main(int argc, char** argv)
{
    double utime0, stime0, wtime0,utime1, stime1, wtime1;
    char *archivoEntrada=NULL;
    Nodo *frecuencias=(Nodo*)malloc(256*sizeof(Nodo));
    char *archivo_frecuencias, *archivo_codificacion;

    if(argc!=2){
        printf("\nIndique el nombre del archivo - Ejemplo: [user@equipo]$ %s archivo.txt\n", argv[0]);
        exit(1);
    }else{
        archivoEntrada=(char*)malloc(strlen(argv[1])+1);
        strcpy(archivoEntrada, argv[1]);
    }

    InicializarFrecuencias(frecuencias);
    LeerArchivo_Frecuencia(archivoEntrada, frecuencias);

    char *extension=NULL;
    char *base=NULL;
    NombreBase(archivoEntrada, &base, &extension);

    char *base1=(char*)malloc(strlen(base)+1);
    strcpy(base1, base);

    size_t base_len=strlen(base);
    archivo_frecuencias=(char*)malloc(base_len+sizeof("_frecuencias.txt"));
    snprintf(archivo_frecuencias, base_len+sizeof("_frecuencias.txt"), "%s_frecuencias.txt", base);

    EscribirTablaFrecuencias(frecuencias, archivo_frecuencias, extension);

    Arbol *arbolHuffman=ConstruirArbolHuffman(frecuencias, 256);

    InicializarTablaHuffman();
    unsigned char* codigoTemp=(unsigned char*)calloc(256/8, 1);
    GenerarCodigosHuffman(arbolHuffman, codigoTemp, 0);

    size_t base1_len=strlen(base1);
    archivo_codificacion=(char*)malloc(base1_len+sizeof("_codificacion.dat"));
    snprintf(archivo_codificacion, base1_len+sizeof("_codificacion.dat"), "%s_codificacion.dat", base1);

    uswtime(&utime0, &stime0, &wtime0);

    CodificarArchivo(archivoEntrada, archivo_codificacion, archivo_frecuencias);

    uswtime(&utime1, &stime1, &wtime1);

    printf("\nProceso completado exitosamente:\n");
    printf("1. Archivo codificado guardado en: %s\n", archivo_codificacion);
    printf("2. Tabla de frecuencias guardada en: %s\n", archivo_frecuencias);
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
    LiberarArbol(arbolHuffman);
    free(codigoTemp);
    free(frecuencias);
    free(archivoEntrada);
    free(base);
    free(base1);
    free(extension);
    free(archivo_frecuencias);
    free(archivo_codificacion);
    LiberarTablaHuffman();

    return 0;
}

//*****************************************************************
//LeerArchivo_Frecuencia (Definición)
//*****************************************************************
//Descripción: Abre el archivo en modo binario y lo recorre byte por
//byte. Cada byte leído, con valor de 0 a 255, incrementa en uno la
//frecuencia de la posición del arreglo que corresponde a ese valor.
//Si el archivo no se puede abrir, termina el programa con exit(1).
//
//Recibe:
//  - archivo (tipo char *): Ruta del archivo a comprimir.
//  - frecuencias (tipo Nodo *): Arreglo de 256 posiciones, inicializado
//    con InicializarFrecuencias, donde se acumulan las frecuencias.
//Devuelve: Nada.
//*****************************************************************
void LeerArchivo_Frecuencia(char *archivo, Nodo *frecuencias)
{
    FILE *file=fopen(archivo, "rb");
    int byte;

    if(file==NULL){
        printf("Error al abrir archivo\n");
        exit(1);
    }

    while((byte=fgetc(file))!=EOF){
        frecuencias[byte].frecuencia++;
    }

    fclose(file);
}

//*****************************************************************
//InicializarFrecuencias (Definición)
//*****************************************************************
//Descripción: Prepara el arreglo de frecuencias: la posición i guarda
//el byte i (0 a 255) con frecuencia 0, de modo que el valor de cada
//byte leído sirve directamente como índice para contarlo.
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
//Descripción: Construye el árbol de Huffman con la estrategia voraz.
//Crea una hoja por cada byte con frecuencia mayor que cero, en orden
//creciente de valor. Mientras quede más de un árbol, los ordena con
//QuickSortArboles y fusiona los dos de menor frecuencia: el primero
//queda como hijo izquierdo y el segundo como derecho. El decodificador
//repite este procedimiento a partir de la tabla de frecuencias, que
//conserva el mismo orden, por lo que obtiene el mismo árbol aun cuando
//hay frecuencias iguales.
//
//Recibe:
//  - frecuencias (tipo Nodo *): Arreglo de 256 posiciones con la
//    frecuencia de cada byte.
//  - n (tipo int): Capacidad del arreglo auxiliar de árboles; el
//    codificador pasa 256.
//Devuelve:
//  - (tipo Arbol *): Raíz del árbol de Huffman, o NULL si ningún byte
//    tiene frecuencia mayor que cero (archivo vacío).
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

//*****************************************************************
//InicializarTablaHuffman (Definición)
//*****************************************************************
//Descripción: Reserva para cada uno de los 256 bytes posibles un
//arreglo de 32 bytes (256 bits) en ceros para su código y deja su
//longitud en 0, que indica que el byte todavía no tiene código.
//
//Recibe: Nada; trabaja sobre la variable global tablaHuffman.
//Devuelve: Nada.
//*****************************************************************
void InicializarTablaHuffman()
{
    for(int i=0; i<256; i++){
        tablaHuffman[i].codigo=(unsigned char*)malloc(256/8);
        memset(tablaHuffman[i].codigo, 0, 256/8);
        tablaHuffman[i].longitud=0;
    }
}

//*****************************************************************
//LiberarTablaHuffman (Definición)
//*****************************************************************
//Descripción: Libera el arreglo de código que InicializarTablaHuffman
//reservó para cada una de las 256 entradas de la tabla.
//
//Recibe: Nada; trabaja sobre la variable global tablaHuffman.
//Devuelve: Nada.
//*****************************************************************
void LiberarTablaHuffman()
{
    for(int i=0; i<256; i++){
        free(tablaHuffman[i].codigo);
    }
}

//*****************************************************************
//GenerarCodigosHuffman (Definición)
//*****************************************************************
//Descripción: Recorre el árbol en profundidad para asignar los
//códigos: al bajar al hijo izquierdo coloca un 0 y al bajar al derecho
//un 1 en la posición profundidad de codigoActual, del bit más
//significativo al menos significativo de cada byte. Al llegar a una
//hoja copia en tablaHuffman, en la entrada de su byte, los bytes de
//codigoActual que contienen el camino y guarda la profundidad como
//longitud. Si la raíz es una hoja (el archivo contiene un único valor
//de byte), le asigna el código 0 de longitud 1.
//
//Recibe:
//  - raiz (tipo Arbol *): Subárbol que se recorre; si es NULL, no hace
//    nada.
//  - codigoActual (tipo unsigned char *): Arreglo de 32 bytes (256
//    bits) con el camino desde la raíz; se reutiliza en toda la
//    recursión.
//  - profundidad (tipo int): Nivel de raiz, igual al número de bits ya
//    colocados en codigoActual; la primera llamada usa 0.
//Devuelve: Nada; los códigos quedan en la variable global tablaHuffman.
//*****************************************************************
void GenerarCodigosHuffman(Arbol* raiz, unsigned char* codigoActual, int profundidad)
{
    if(raiz==NULL) return;

    if(raiz->izq==NULL && raiz->der==NULL){
        memcpy(tablaHuffman[(unsigned char)raiz->caracter].codigo, codigoActual, (profundidad+7)/8);
        if(profundidad>0){
            tablaHuffman[(unsigned char)raiz->caracter].longitud=profundidad;
        }else{
            tablaHuffman[(unsigned char)raiz->caracter].longitud=1;
        }
        return;
    }

    if(raiz->izq){
        PONE_0(codigoActual[profundidad/8], 7-(profundidad%8));
        GenerarCodigosHuffman(raiz->izq, codigoActual, profundidad+1);
    }

    if(raiz->der){
        PONE_1(codigoActual[profundidad/8], 7-(profundidad%8));
        GenerarCodigosHuffman(raiz->der, codigoActual, profundidad+1);
    }
}

//*****************************************************************
//InicializarBufferBits (Definición)
//*****************************************************************
//Descripción: Deja el búfer de bits vacío (byte y contador en 0) y lo
//asocia con el archivo donde se escribirán los bytes completos.
//
//Recibe:
//  - bb (tipo BufferBits *): Búfer a inicializar.
//  - archivo (tipo FILE *): Archivo de salida abierto en modo binario.
//Devuelve: Nada.
//*****************************************************************
void InicializarBufferBits(BufferBits *bb, FILE *archivo)
{
    bb->buffer=0;
    bb->contador=0;
    bb->archivo=archivo;
}

//*****************************************************************
//FlushBuffer (Definición)
//*****************************************************************
//Descripción: Si el búfer contiene bits pendientes, escribe en el
//archivo el byte parcial tal como está (sus bits sin usar valen 0) y
//reinicia el búfer.
//
//Recibe:
//  - bb (tipo BufferBits *): Búfer cuyo contenido se escribe.
//Devuelve: Nada.
//*****************************************************************
void FlushBuffer(BufferBits *bb)
{
    if(bb->contador>0){
        fwrite(&bb->buffer, 1, 1, bb->archivo);
        bb->buffer=0;
        bb->contador=0;
    }
}

//*****************************************************************
//EscribirBit (Definición)
//*****************************************************************
//Descripción: Coloca un bit en la siguiente posición libre del búfer,
//empezando por el bit más significativo (posición 7 - contador). Al
//completar 8 bits escribe el byte en el archivo y reinicia el búfer.
//
//Recibe:
//  - bb (tipo BufferBits *): Búfer donde se acumula el bit.
//  - bit (tipo int): Bit a escribir; 0 escribe un 0 y cualquier otro
//    valor escribe un 1.
//Devuelve: Nada.
//*****************************************************************
void EscribirBit(BufferBits *bb, int bit)
{
    if(bit){
        PONE_1(bb->buffer, 7-bb->contador);
    }else{
        PONE_0(bb->buffer, 7-bb->contador);
    }

    bb->contador++;

    if(bb->contador==8){
        fwrite(&bb->buffer, 1, 1, bb->archivo);
        bb->buffer=0;
        bb->contador=0;
    }
}

//*****************************************************************
//EscribirBits (Definición)
//*****************************************************************
//Descripción: Escribe en el búfer, uno por uno con EscribirBit, los
//primeros longitud bits de un código de Huffman, leyéndolos del bit
//más significativo al menos significativo de cada byte del código.
//
//Recibe:
//  - bb (tipo BufferBits *): Búfer de salida.
//  - codigo (tipo unsigned char *): Código empaquetado en bits, tal
//    como lo guarda tablaHuffman.
//  - longitud (tipo int): Número de bits del código que se escriben.
//Devuelve: Nada.
//*****************************************************************
void EscribirBits(BufferBits *bb, unsigned char *codigo, int longitud)
{
    for(int i=0; i<longitud; i++){
        int bit=CONSULTARBIT(codigo[i/8], 7-(i%8));
        EscribirBit(bb, bit);
    }
}

//*****************************************************************
//ManejarUltimoByte (Definición)
//*****************************************************************
//Descripción: Al terminar la codificación, si quedó un byte incompleto
//en el búfer, rellena con ceros sus bits restantes y lo escribe en el
//archivo. El decodificador no interpreta este relleno porque se
//detiene al recuperar el número de bytes guardado en la cabecera.
//
//Recibe:
//  - bb (tipo BufferBits *): Búfer con el posible byte incompleto.
//Devuelve: Nada.
//*****************************************************************
void ManejarUltimoByte(BufferBits *bb)
{
    if(bb->contador>0){
        while(bb->contador<8){
            PONE_0(bb->buffer, 7-bb->contador);
            bb->contador++;
        }

        fwrite(&bb->buffer, 1, 1, bb->archivo);
    }
}

//*****************************************************************
//CodificarArchivo (Definición)
//*****************************************************************
//Descripción: Genera el archivo comprimido con los códigos de
//tablaHuffman, que ya deben estar generados. Primero escribe como
//cabecera el tamaño del archivo original (long long en la
//representación nativa de la máquina); después lee la entrada byte por
//byte, escribe el código de cada uno empaquetado en bits y completa
//con ceros el último byte. Al final imprime el tamaño del archivo
//original, del comprimido y de la tabla de frecuencias, y el
//porcentaje de compresión 100 * (1 - (comprimido + tabla) / original),
//que se omite si el archivo original está vacío. Termina el programa
//con exit(1) si no puede abrir un archivo o consultar el tamaño de la
//entrada.
//
//Recibe:
//  - archivoEntrada (tipo char *): Ruta del archivo original.
//  - archivoSalida (tipo char *): Ruta del archivo comprimido (.dat).
//  - archivoFrecuencias (tipo char *): Ruta de la tabla de frecuencias
//    ya escrita; solo se usa para sumar su tamaño en el porcentaje.
//Devuelve: Nada.
//*****************************************************************
void CodificarArchivo(char *archivoEntrada, char *archivoSalida, char *archivoFrecuencias)
{
    FILE *entrada=fopen(archivoEntrada, "rb");
    if(!entrada){
        printf("Error al abrir el archivo de entrada\n");
        exit(1);
    }

    long long tamanoOriginal=obtenerTamanoArchivo(archivoEntrada);
    if(tamanoOriginal<0){
        printf("Error al obtener el tamaño del archivo de entrada\n");
        fclose(entrada);
        exit(1);
    }

    FILE *salida=fopen(archivoSalida, "wb");
    if(!salida){
        printf("Error al abrir el archivo de salida\n");
        fclose(entrada);
        exit(1);
    }

    fwrite(&tamanoOriginal, sizeof(long long), 1, salida);

    BufferBits bb;
    InicializarBufferBits(&bb, salida);

    int byte;
    while((byte=fgetc(entrada))!=EOF){
        EscribirBits(&bb, tablaHuffman[byte].codigo, tablaHuffman[byte].longitud);
    }

    ManejarUltimoByte(&bb);

    fclose(entrada);
    fclose(salida);

    long long tamanoComprimido=obtenerTamanoArchivo(archivoSalida);
    long long tamanoFrecuencias=obtenerTamanoArchivo(archivoFrecuencias);

    if(tamanoComprimido>0 && tamanoFrecuencias>0){
        printf("Tamaño del archivo original: %lld bytes\n", tamanoOriginal);
        printf("Tamaño del archivo comprimido: %lld bytes\n", tamanoComprimido);
        printf("Tamaño de la tabla de frecuencias: %lld bytes\n", tamanoFrecuencias);
        if(tamanoOriginal>0){
            double porcentajeCompresion=100*(1-((double)(tamanoComprimido+tamanoFrecuencias)/tamanoOriginal));
            printf("Porcentaje de compresión (comprimido + tabla): %.2f%%\n", porcentajeCompresion);
        }
    }else{
        printf("Error al calcular el tamaño de los archivos.\n");
    }
}

//*****************************************************************
//EscribirTablaFrecuencias (Definición)
//*****************************************************************
//Descripción: Escribe la tabla de frecuencias que necesita el
//decodificador. La primera línea es el encabezado "C F ASCCI"
//(carácter, frecuencia y valor del byte), la segunda es la extensión
//del archivo original (vacía si no tiene) y después hay una línea por
//cada byte con frecuencia mayor que cero, en orden creciente de valor,
//con el formato "'c' frecuencia valor" si el byte es imprimible (32 a
//126) o "0xHH frecuencia valor" si no lo es. Si no puede crear el
//archivo, termina el programa con exit(1).
//
//Recibe:
//  - frecuencias (tipo Nodo *): Arreglo de 256 posiciones con la
//    frecuencia de cada byte.
//  - nombreArchivo (tipo const char *): Ruta del archivo de texto que
//    se crea.
//  - extension (tipo char *): Extensión del archivo original, sin punto.
//Devuelve: Nada.
//*****************************************************************
void EscribirTablaFrecuencias(Nodo *frecuencias, const char *nombreArchivo, char *extension)
{
    FILE *archivo=fopen(nombreArchivo, "w");
    if(!archivo){
        printf("Error al crear el archivo de frecuencias\n");
        exit(1);
    }

    fprintf(archivo, "C F ASCCI\n");
    fprintf(archivo, "%s\n", extension);
    for(int i=0; i<256; i++){
        if(frecuencias[i].frecuencia>0){
            if(frecuencias[i].caracter>=32 && frecuencias[i].caracter<=126){
                fprintf(archivo, "'%c' %lld %d\n",
                    frecuencias[i].caracter,
                    frecuencias[i].frecuencia,
                    (unsigned char)frecuencias[i].caracter);
            }else{
                fprintf(archivo, "0x%02X %lld %d\n",
                    (unsigned char)frecuencias[i].caracter,
                    frecuencias[i].frecuencia,
                    (unsigned char)frecuencias[i].caracter);
            }
        }
    }
    fclose(archivo);
}

//*****************************************************************
//obtenerTamanoArchivo (Definición)
//*****************************************************************
//Descripción: Consulta con stat el tamaño en bytes de un archivo.
//
//Recibe:
//  - nombreArchivo (tipo const char *): Ruta del archivo.
//Devuelve:
//  - (tipo long long): Tamaño del archivo en bytes, o -1 si stat falla.
//*****************************************************************
long long obtenerTamanoArchivo(const char *nombreArchivo)
{
    struct stat st;
    if(stat(nombreArchivo, &st)==0)
        return st.st_size;
    return -1;
}

//*****************************************************************
//NombreBase (Definición)
//*****************************************************************
//Descripción: Separa una ruta en nombre base y extensión. La extensión
//es el texto después del último punto del nombre del archivo (la parte
//posterior a la última '/'). Si el nombre no tiene punto, o si su
//último punto es el primer carácter (archivo oculto, como .bashrc), la
//ruta completa es el nombre base y la extensión queda vacía. El nombre
//base conserva el directorio. Ambas cadenas se reservan con memoria
//dinámica y quien llama a la función debe liberarlas.
//
//Recibe:
//  - nombreArchivo (tipo char *): Ruta del archivo.
//  - nombreSinExtension (tipo char **): Dirección donde se guarda la
//    ruta sin el punto ni la extensión.
//  - extension (tipo char **): Dirección donde se guarda la extensión
//    sin punto, o una cadena vacía.
//Devuelve: Nada.
//*****************************************************************
void NombreBase(char *nombreArchivo, char **nombreSinExtension, char **extension)
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

        *extension=strdup(punto+1);
    }else{
        *nombreSinExtension=strdup(nombreArchivo);
        *extension=strdup("");
    }
}
