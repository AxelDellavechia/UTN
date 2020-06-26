## Compilación ##

Como parametros de compilación se debe colocar:

+ *-DFUSE_USE_VERSION=27*
+ *-D_FILE_OFFSET_BITS=64*

Como parametros de linkeo se debe colocar:

+ *-lpthread*
+ *-lfuse*

Como blibliteca se debe colocar:

fuse

## FUSE Parameters ##

+ **-f**: Desactiva la ejecución en modo background
+ **-s**: La biblioteca de FUSE se ejecuta en modo single thread
+ **-d**: Imprime información de debug

# Ejecutar de la siguiente manera

//Se debe agregar la variable de la biblioteca nuestra con el comando -> export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/Documentos/Branchs/AXEL/tp-2019-2c-Anoche-funcionaba/linuseCommons/Debug
//Validar con ldd SAC_Cli si se agrego correctamente
1) ./SAC_Cli -d -s -odirect_io linuseFS



# Para desmontar

fusermount -u linuseFS

# Extra para ver Memory Leaks

Por consola --> valgrind --show-leak-kinds=all --leak-check=full -v ./SAC_Cli -d -s -odirect_io linuseFS

Por Archivo --> valgrind --show-leak-kinds=all --leak-check=full -v --log-file="../memoryLeaks.txt" ./SAC_Cli -d -s -odirect_io linuseFS

