#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){

    if (argc != 3) {
        printf("Uso: ./malla <filas_x> <columnas_y>\n");
        exit(1);
    }

    // Convertimos los argumentos 'x' e 'y en numeros enteros
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);

    pid_t pid;

    // Creamos 'y' columnas
    for(int i = 0; i < y; i++){
        pid = fork();   
    
        if(pid < 0){    
            perror("Error en el fork");
            exit(1);
        }
    
        if(pid == 0){
            break;      
        }
    }


    if (pid > 0) {
        
        for (int j = 0; j < y; j++) { 
            wait(NULL);
        }
        exit(0); 
    }

    // El proceso hijo crea 'x' filas
    for(int i = 1 ; i < x; i++){ 
        pid = fork();

        if(pid < 0){ 
            perror("Error en el fork vertical");
            exit(1);
        }

        if(pid > 0){ // El proceso hijo espera a que su hijo termine antes de salir
            wait(NULL);
            exit(0);
        }
    }


    alarm(30); 
    pause();   

    exit(0);
}