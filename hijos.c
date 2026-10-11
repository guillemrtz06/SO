#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>


void validar_argumentos(int argc, char *argv[], int *x, int *y) {
    if (argc != 3) {
        char error[] = "Uso: hijos <x> <y>\n";
        write(1, error, strlen(error));
        exit(1);
    }
    *x = atoi(argv[1]);
    *y = atoi(argv[2]);
    
    if (*x < 1 || *y < 1) {
        char error[] = "Error: x e y deben ser mayores que 0\n";
        write(1, error, strlen(error));
        exit(1);
    }
}


int* crear_memoria(int x, int y, int *shmid) {

    int tamaño = (x + y) * sizeof(int);
    
    *shmid = shmget(IPC_PRIVATE, tamaño, IPC_CREAT | 0666);
    if (*shmid == -1) {
        perror("Error al crear memoria compartida");
        exit(1);
    }
    
    int *memoria = (int *) shmat(*shmid, 0, 0);
    if (memoria == (int *)-1) {
        perror("Error al vincular memoria");
        exit(1);
    }
    return memoria;
}


void liberar_memoria(int shmid, int *memoria) {
    if (shmdt((char *) memoria) < 0) {
        perror("Error al desligar memoria");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, 0) < 0) {
        perror("Error al borrar memoria");
        exit(1);
    }
}


void registrar_padre(int *memoria, int nivel) {
    memoria[nivel] = getpid();
}


void registrar_hoja(int *memoria, int x, int indice_hoja) {
    memoria[x + indice_hoja] = getpid();
}


void imprimir_superpadre(int *memoria, int x, int y) {
    printf("Soy el superpadre (%d): mis hijos finales son: ", getpid());
    for (int i = 0; i < y; i++) {
        printf("%d", memoria[x + i]);
        if (i < y - 1) printf(", ");
    }
    printf("\n");
}


void imprimir_subhijo(int *memoria, int x) {
    printf("Soy el subhijo %d, mi padres son: ", getpid());
    for (int i = 0; i < x; i++) {
        printf("%d", memoria[i]);
        if (i < x - 1) printf(", ");
    }
    printf("\n");
}


void ejecutar_tarea_hoja(int *memoria, int x, int indice_hoja) {
    registrar_hoja(memoria, x, indice_hoja);
    imprimir_subhijo(memoria, x);
    exit(0); //
}


void esperar_hijos_horizontales(int y) {
    for (int i = 0; i < y; i++) {
        wait(NULL); //
    }
}


void crear_hojas_horizontales(int *memoria, int x, int y) {
    for (int i = 0; i < y; i++) {
        pid_t pid = fork(); //
        if (pid < 0) {
            perror("Error fork hoja");
            exit(1);
        }
        if (pid == 0) {
            ejecutar_tarea_hoja(memoria, x, i);
        }
    }
    esperar_hijos_horizontales(y);
}


void crear_rama_vertical(int *memoria, int x, int y) {
    for (int nivel = 0; nivel < x - 1; nivel++) {
        registrar_padre(memoria, nivel);
        
        pid_t pid = fork(); //
        if (pid < 0) {
            perror("Error fork vertical");
            exit(1);
        }
        
        if (pid > 0) { 
            wait(NULL); 
            
            if (nivel == 0) {
                imprimir_superpadre(memoria, x, y);
            }
            return;
        }
    }
    

    registrar_padre(memoria, x - 1);
    crear_hojas_horizontales(memoria, x, y);
}


int main(int argc, char *argv[]) {
    int x, y, shmid;
    int *memoria;
    
    validar_argumentos(argc, argv, &x, &y);
    memoria = crear_memoria(x, y, &shmid);
    
    pid_t pid_original = getpid();
    

    crear_rama_vertical(memoria, x, y);
    
    if (getpid() == pid_original) {
        liberar_memoria(shmid, memoria);
    }
    
    return 0;
}