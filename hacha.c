#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

// 1. Recepción y Validación de Argumentos

void parse_args(int argc, char *argv[], char **nombre_archivo, int *tam_trozo) {
    if (argc != 3) {
        char msg[] = "Uso: hacha <archivo> <tamaño>\n";
        write(1, msg, strlen(msg)); 
        exit(1);
    }
    // Asignamos los valores a las direcciones de memoria proporcionadas
    *nombre_archivo = argv[1];
    *tam_trozo = atoi(argv[2]);
}

// 2. Obtención de información del archivo original

void inicializar_datos_archivo(char *nombre_archivo, int tam_trozo, int *tam_total, int *num_hijos) {
    struct stat info;
    if (stat(nombre_archivo, &info) < 0) { 
        perror("Error al obtener info del archivo");
        exit(1);
    }
    
    *tam_total = info.st_size; 
    *num_hijos = *tam_total / tam_trozo;
    
    if (*tam_total % tam_trozo != 0) {
        (*num_hijos)++; 
    }
}

// 3. Apertura del archivo original

int abrir_archivo_origen(char *nombre_archivo) {
    int fd = open(nombre_archivo, O_RDONLY);
    if (fd < 0) {
        perror("Error al abrir origen");
        exit(1);
    }
    return fd;
}

// 4. Lógica de creación del archivo fragmentado (Rol del Hijo)


void crear_y_escribir_fragmento(char *nombre_archivo, int num_trozo, int pipe_lectura) {
    char nombre_destino[256];
    char buffer[1024];
    int fd_destino, leidos;

    sprintf(nombre_destino, "%s.h%02d", nombre_archivo, num_trozo);

    fd_destino = creat(nombre_destino, 0666);
    if (fd_destino < 0) {
        perror("Error al crear destino");
        exit(1);
    }

    while ((leidos = read(pipe_lectura, buffer, sizeof(buffer))) > 0) {
        write(fd_destino, buffer, leidos);
    }

    close(fd_destino);
    close(pipe_lectura);
}

// 5. Lógica de envío de datos (Rol del Padre)

void leer_y_enviar_datos(int fd_origen, int bytes_a_transferir, int pipe_escritura) {
    char buffer[1024];
    int leidos, por_leer;
    int total_enviados = 0;

    while (total_enviados < bytes_a_transferir) {
        por_leer = sizeof(buffer);
        if ((bytes_a_transferir - total_enviados) < sizeof(buffer)) {
            por_leer = bytes_a_transferir - total_enviados;
        }

        leidos = read(fd_origen, buffer, por_leer);
        if (leidos <= 0) break;

        write(pipe_escritura, buffer, leidos);
        total_enviados += leidos;
    }
    
    close(pipe_escritura);
}

// 6. Generación de procesos y tuberías

void generar_hijos_y_fragmentar(char *nombre_archivo, int fd_origen, int tam_total, int tam_trozo, int num_hijos) {
    int tuberia[2];
    int bytes_para_hijo;

    for (int i = 0; i < num_hijos; i++) {
        if (pipe(tuberia) < 0) {
            perror("Error en pipe");
            exit(1);
        }

        if (i == num_hijos - 1 && tam_total % tam_trozo != 0) {
            bytes_para_hijo = tam_total % tam_trozo;
        } else {
            bytes_para_hijo = tam_trozo;
        }

        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Error fork");
            exit(1);
        }

        if (pid == 0) { 
            close(tuberia[1]);
            crear_y_escribir_fragmento(nombre_archivo, i, tuberia[0]);
            exit(0); 
        } else { 
            close(tuberia[0]);
            leer_y_enviar_datos(fd_origen, bytes_para_hijo, tuberia[1]);
            wait(NULL);
        }
    }
}

// 7. Orquestador principal

int main(int argc, char *argv[]) {

    char *nombre_archivo;
    int tam_trozo;
    int tam_total, num_hijos;

    parse_args(argc, argv, &nombre_archivo, &tam_trozo);
    
    inicializar_datos_archivo(nombre_archivo, tam_trozo, &tam_total, &num_hijos);
    int fd_origen = abrir_archivo_origen(nombre_archivo);
    
    generar_hijos_y_fragmentar(nombre_archivo, fd_origen, tam_total, tam_trozo, num_hijos);
    
    close(fd_origen);
    return 0;
}
