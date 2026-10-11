#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

pid_t pid_A_global; 
pid_t pid_B_global;  


void manejador_vacio(int sig) {
    
}


void manejador_z(int sig) {
    kill(pid_A_global, SIGUSR1);
}


void manejador_a(int sig) {
    pid_t pid_hijo = fork();
    if (pid_hijo == 0) {
        execlp("pstree", "pstree", NULL);
        perror("Error al ejecutar pstree");
        exit(1);
    } else if (pid_hijo > 0) {
        wait(NULL); 
        kill(pid_B_global, SIGUSR2);
    }
}


void parse_args(int argc, char *argv[], int *segundos) {
    if (argc < 2) { 
        printf("Uso: %s <segundos>\n", argv[0]);
        exit(1);
    }
    *segundos = atoi(argv[1]);
}


void ejecutar_hoja_z(pid_t p_A, pid_t p_ejec, int segundos) {
    printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n", getpid(), getppid(), p_A, p_ejec);

    signal(SIGALRM, manejador_z);
    signal(SIGUSR2, manejador_vacio); 
    
    alarm(segundos); 
    pause();
    
    pause();
    printf("Soy Z (%d) y muero\n", getpid());
    exit(0);
}


void ejecutar_hoja_y(pid_t p_A, pid_t p_ejec) {
    printf("Soy el proceso Y: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n", getpid(), getppid(), p_A, p_ejec);
    signal(SIGUSR2, manejador_vacio);
    pause(); 
    printf("Soy Y (%d) y muero\n", getpid());
    exit(0);
}


void ejecutar_hoja_x(pid_t p_A, pid_t p_ejec) {
    printf("Soy el proceso X: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n", getpid(), getppid(), p_A, p_ejec);
    signal(SIGUSR2, manejador_vacio);
    pause(); 
    printf("Soy X (%d) y muero\n", getpid());
    exit(0);
}


void crear_hojas_b(pid_t p_A, pid_t p_ejec, int segundos) {
    pid_t p_B = getpid();
    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n", p_B, p_A, p_ejec);

    signal(SIGUSR2, manejador_vacio); 

    // Creamos proceso X
    pid_t pid_X = fork();
    if (pid_X < 0) { perror("Error en fork X"); exit(1); }
    if (pid_X == 0) { ejecutar_hoja_x(p_A, p_ejec); }

    // Creamos proceso Y
    pid_t pid_Y = fork();
    if (pid_Y < 0) { perror("Error en fork Y"); exit(1); }
    if (pid_Y == 0) { ejecutar_hoja_y(p_A, p_ejec); }

    // Creamos proceso Z
    pid_t pid_Z = fork();
    if (pid_Z < 0) { perror("Error en fork Z"); exit(1); }
    if (pid_Z == 0) { ejecutar_hoja_z(p_A, p_ejec, segundos); }

    pause();
    
    kill(pid_Z, SIGUSR2);
    waitpid(pid_Z, NULL, 0); 
    
    kill(pid_Y, SIGUSR2);
    waitpid(pid_Y, NULL, 0); 
    
    kill(pid_X, SIGUSR2);
    waitpid(pid_X, NULL, 0); 

    printf("Soy B (%d) y muero\n", getpid());
    exit(0);
}


void crear_proceso_b(pid_t p_A, pid_t p_ejec, int segundos) {
    pid_t pid_B = fork();
    if (pid_B < 0) {
        perror("Error en fork B");
        exit(1);
    }
    
    if (pid_B == 0) {
        crear_hojas_b(p_A, p_ejec, segundos);
    }
    
    pid_B_global = pid_B; 
    wait(NULL); 
}


void crear_proceso_a(pid_t p_ejec, int segundos) {
    pid_t p_A = getpid();
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", p_A, p_ejec);

    pid_A_global = p_A; 
    signal(SIGUSR1, manejador_a); 

    crear_proceso_b(p_A, p_ejec, segundos);
    
    printf("Soy A (%d) y muero\n", getpid());
    exit(0);
}


int main(int argc, char *argv[]) {
    int segundos;
    parse_args(argc, argv, &segundos);

    pid_t pid_ejec = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pid_ejec);

    // Creamos el proceso A
    pid_t pid_A = fork();
    if (pid_A < 0) {
        perror("Error en fork A");
        exit(1);
    }
    
    if (pid_A == 0) {
        crear_proceso_a(pid_ejec, segundos);
    }

    wait(NULL);
    printf("Soy ejec (%d) y muero\n", getpid());
    
    return 0;
}