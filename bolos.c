#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef __USE_POSIX
    #define __USE_POSIX
#endif

#ifdef DEBUG
    #define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
    #define DEBUG_PRINT(...) do {} while (0)
#endif

#define writefacil(str) write(STDOUT_FILENO, (str), strlen(str))


/*
 * Esta función se encarga de manejar la lógica de los bolos una vez se han
 * creado.
 */
int mente(pid_t suBoloI, pid_t suBoloD, int *tirado_I, int *tirado_D);

/*
 * Manejadora del padre
 */
int padre();

/*
 * Engendra una ristra de bolos (o un solo bolo) a partir de un array de
 * tuplas de 3 elementos (nombre, pidI, pidD).
 *
 * Devuelve el pid del primer bolo de la ristra.
 *
 * n:    número de bolos que vamos a engendrar
 *
 * args: array de tuplas de 3 elementos, cada tupla son los argumentos que
 *       vamos a pasar a la llamada de execl de un bolo.
 *       En cada tupla:
 *        - tupla[0] es el nombre del bolo
 *        - tupla[1] es el pid del suBoloI (o -1 si no tiene)
 *        - tupla[2] es el pid del suBoloD (o -1 si no tiene)
 *
 * argv0_inicial: nombre del programa inicial (argv[0] para el padre.). Lo
 *        necesitamos para poder pasarle el nombre del programa a las llamadas
 *        de execl.
 *
 * La lista de tuplas se pasa en orden inverso (la tupla del padre es la última
 * de la lista, la de su hijo es la penúltima,...).
 */
int engendrar(int n, int *args, char *bolos, pid_t pid_P);

/*
 * Convierte un entero a un string
 */
char *to_string(int v);

/*
 * Función vacía para pasársela a sigaction
 */
void nonada(int signum);

/*
 * Utiliza gettimeofday para decidir qué va a hacer el bolo una vez lo han
 * tirado
 */
int elegir_accion(void);

/*
 * Ejecuta un ps
 */
pid_t ejecutar_ps(void);

/*
 * Imprime los bolos vivos por pantalla de manera bonita
 */
int imprimir_dibujo(pid_t pid_H, pid_t pid_I, pid_t pid_E, pid_t pid_B,
                     pid_t pid_C, int tirado_B, int tirado_C);


int main(int argc, char *argv[])
{
    int *args, retorno_mente, subI, subD, hijos_muertos, tirado_D, tirado_I, stat;
    pid_t pid_H, pid_I, pid_E, pid_B, pid_C;
    pid_t pid_ps;

    /* Bloqueamos todas las señales que se nos permite bloquear, para evitar
     * que se nos envíen mientras estamos creando el árbol de procesos.
     * Las desbloquearemos cuando estemos listos para recibirlas.
     */
    sigset_t conjunto_todo;

    sigfillset(&conjunto_todo);
    sigdelset(&conjunto_todo, SIGINT);
    if (sigprocmask(SIG_BLOCK, &conjunto_todo, NULL) == -1)
        return 1;

    /* Comprobar P mirando si el primer argumento acaba con "bolos" */
    if (strstr(argv[0], "bolos") != NULL)
    {
        /* Acaba en bolos, es P */
        switch (fork())
        {
            case -1:
                perror("Fallo en el primer fork (1)\n");
                exit(1);

            case 0:
                execl(argv[0], "A", argv[0], NULL);

            default:
                exit(padre());
        }
    }

    /* Comprobar el bolo dependiendo del número de argumentos que se le pasen */
    switch (argc)
    {
        case 1:
            /* 1 solo argumento debería ocurrir cuando se llama a P pero por
             * precaución lo hacemos aparte comprobando el argumento.
             */
            break;

        case 2:
            /* Tenemos 2 argumentos:
             *   - el nombre del bolo
             *   - el nombre del programa
             *  Esto sólo ocurre cuando el bolo es A.
             */
            DEBUG_PRINT("A: %d\n", getpid());
            break;

        case 4:
            /* Tenemos 4 argumentos:
             *   - nombre del bolo
             *   - argv[0] original
             *   - pid del bolo de la izquierda
             *   - pid del bolo de la derecha
             * Este es el caso para cualquier bolo que no sea A.
             */
            mente(atoi(argv[2]), atoi(argv[3]), &tirado_I, &tirado_D);

            /* Contamos niños muertos (si es que tenemos) */
            hijos_muertos = 0;
            /*
             * B y D esperarán a sus hijos si han matado a algo por la izquierda
             * y C y F lo harán si han matado algo por la derecha.
             */
            if (((argv[0][0] == 'B' || argv[0][0] == 'D') && tirado_I) ||
                ((argv[0][0] == 'C' || argv[0][0] == 'F') && tirado_D))
            {
                wait(&stat);
                hijos_muertos += WEXITSTATUS(stat);
            }

            DEBUG_PRINT("[%d %s] Se me han muerto %d hijos :)\n", getpid(), argv[0], hijos_muertos);

            exit(hijos_muertos + 1);
    }

    /*
     * Si hemos llegado aquí, el bolo es A (el único que sale del switch de
     * arriba).
     */
    /* === Engendramos a H e I, que son hijos de A, y se los tenemos que ===
     * === pasar a D, E y F.                                             ===
     */
    /* Reservamos espacio para los argumentos que le vamos a pasar a engendrar.
     * Estos bolos no tienen hijos, así que reservamos espacio para una tupla
     * de 3 argumentos.
     * El nombre del bolo, el pid del suBoloI y el pid de suBoloD.
     */
    args = malloc(sizeof(int) * 3);
    if (args == NULL)
    {
        perror("Error al alocar memoria en A\n");
        exit(2);
    }

    /*
     * NOTA: El primer elemento de args es un caracter, pero lo convertimos a
     * entero, y lo pasamos como entero, para que nos resulte más sencillo
     * de manejar (sólo tenemos que pasar una array).
     */
    args[0] = 'H'; args[1] = -1; args[2] = -1;
    pid_H = engendrar(1, args, argv[1], -1);

    args[0] = 'I'; args[1] = -1; args[2] = -1;
    pid_I = engendrar(1, args, argv[1], -1);

    /* === Engendramos a E con H e I como hijos                          ===
     */
    args[0] = 'E'; args[1] = pid_H; args[2] = pid_I;
    pid_E = engendrar(1, args, argv[1], -1);

    free(args); /* Liberamos la memoria del malloc anterior. */

    /* === Engendramos las ristras BDG y CFJ.                            ===
     * === Para BDG, le pasamos a engendrar los datos de B, y sus hijos, ===
     * === D y G.                                                        ===
     */
    args = malloc(3 * 3 * sizeof(int));
    if (args == NULL)
    {
        perror("Error al alocar memoria en A. Existen procesos creados!!\n");
        exit(2);
    }

    args[0] = 'G'; args[1] = -1; args[2] = -1;     // G no tiene suBolos
    args[3] = 'D'; args[4] = -1; args[5] = pid_H;  // D tiene H como suBoloD
    args[6] = 'B'; args[7] = -1; args[8] = pid_E;  // B tiene E como suBoloD
    pid_B = engendrar(3, args, argv[1], getppid());

    /* === Para CFJ, le pasamos a engendrar los datos de C, y sus hijos, ===
     * === F y J.                                                        ===
     */
    args[0] = 'J'; args[1] = -1; args[2] = -1;  // J no tiene suBolos
    args[3] = 'F'; args[4] = pid_I; args[5] = -1;  // F tiene I como suBoloI
    args[6] = 'C'; args[7] = pid_E; args[8] = -1;  // C tiene E como suBoloI
    pid_C = engendrar(3, args, argv[1], getppid());

    free(args); /* Liberamos la memoria del malloc anterior. */

    /* 
     * Ejecutamos mente para A, como si fuera un bolo normal, y luego
     * realizamos el comportamiento exclusivo de A.
     */
    retorno_mente = mente(pid_B, pid_C, &tirado_I, &tirado_D);
    if (retorno_mente != 0)   /* Si mente ha fallado, salimos */
        return retorno_mente;

    /* Dormir */
    DEBUG_PRINT("A duerme durante 4 segundos...\n");
    sleep(4);
    /* Imprimir dibujo */
    imprimir_dibujo(pid_H, pid_I, pid_E, pid_B, pid_C, tirado_I, tirado_D);

    /* Usar ps -fu */
    DEBUG_PRINT("Aquí tienes el ps -fu usuario, para verificar que todo está bien:\n");
    pid_ps = ejecutar_ps(); 
    if (pid_ps == -1)
    {
        perror("Error al ejecutar ps -fu $USER\n");
        return 1;
    }
    waitpid(pid_ps, NULL, 0);
    DEBUG_PRINT("Fin del ps -fu $USER\n");

    /* Matar */
    DEBUG_PRINT("Mato a todos los procesos hijos\n");
    kill(0, SIGINT);

    return 0;
}


int padre()
{
    /* 
     * Esperamos a que G y J (los últimos hijos en crearse) terminen.
     * Estos van a mandar una señal SIGUSR1 y SIGUSR2 (respectivamente)
     * a P cuando se creen.
     * El código de padre es casi idéntico al de mente.
     */

    sigset_t conjunto_sin_SIGUSR1, conjunto_sin_SIGUSR2, conjunto_vacio;
    struct sigaction accion_nueva;

    /*
     * Creamos una estructura de sigaction que bloquee todas las señales menos
     * SIGUSR1, y otro que haga lo mismo para SIGUSR2.
     */
    sigfillset(&conjunto_sin_SIGUSR1);
    sigdelset(&conjunto_sin_SIGUSR1, SIGUSR1);
    
    sigfillset(&conjunto_sin_SIGUSR2);
    sigdelset(&conjunto_sin_SIGUSR2, SIGUSR2);


    /*
     * Creamos una estructura de sigaction para poder manejar las señales
     * SIGUSR1 y SIGUSR2. Guardamos la acción vieja.
     */
    sigemptyset(&conjunto_vacio);
    accion_nueva.sa_handler = nonada;
    accion_nueva.sa_mask = conjunto_vacio;
    accion_nueva.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &accion_nueva, NULL) == -1)
        return 1;
    if (sigaction(SIGUSR2, &accion_nueva, NULL) == -1)
        return 1;

    /*
     * Esperamos a que nos llegue una señal de SIGUSR1. Significaría
     * que se ha creado G.
     */
    sigsuspend(&conjunto_sin_SIGUSR1);
    DEBUG_PRINT("[P] Se ha creado G\n");
    /*
     * Esperamos a que nos llegue una señal de SIGUSR2. Significaría
     * que se ha creado J.
     */
    sigsuspend(&conjunto_sin_SIGUSR2);
    DEBUG_PRINT("[P] Se ha creado J\n");

    DEBUG_PRINT("[P] muero\n");
    return 0;
}

void nonada(int signum) {}

int mente(pid_t suBoloI, pid_t suBoloD, int *tirado_I, int *tirado_D)
{
    sigset_t conjunto_sin_SIGTERM, conjunto_vacio;
    struct sigaction accion_nueva;

    /*
     * Creamos una estructura de sigaction que bloquee todas las señales menos
     * SIGTERM.
     */
    sigfillset(&conjunto_sin_SIGTERM);
    sigdelset(&conjunto_sin_SIGTERM, SIGTERM);
    sigdelset(&conjunto_sin_SIGTERM, SIGINT);

    /*
     * Creamos una estructura de sigaction para poder manejar la señal
     * SIGTERM. Guardamos la acción vieja.
     * La máscara de bloqueo de señales para esta acción va a ser el conjunto
     * vacío, lo que significa que cualquier otra señal puede interrumpir
     * la ejecución de esta acción
     */
    sigemptyset(&conjunto_vacio);
    accion_nueva.sa_handler = nonada;
    accion_nueva.sa_mask = conjunto_vacio;
    accion_nueva.sa_flags = SA_RESTART; /* SA_RESTART es porque Polar ha dicho
                                         * que es lo mejor.
                                         */
    if (sigaction(SIGTERM, &accion_nueva, NULL) == -1) return 1;

    /* Aquí ya tenemos código de verdad. */
    if (suBoloI != -1)
        DEBUG_PRINT("[%d] Tengo debajo a %d y %d\n", getpid(), suBoloI, suBoloD);
    else
        DEBUG_PRINT("[%d] No tengo bolos debajo\n", getpid());

    /*
     * Esperamos a que nos llegue una señal de SIGTERM. Significaría
     * que nos han tirado
     */
    sigsuspend(&conjunto_sin_SIGTERM);
    /* Me tiraron :( */
    DEBUG_PRINT("[%d] He sido tirado :(\n", getpid());

    /* Si tenemos bolos debajo de nosotros, procedemos a tirarlos. */
    if (suBoloI != -1 && suBoloD != -1)
    {
        switch (elegir_accion())
        {
            case 0:
                DEBUG_PRINT("[%d] No tiro a nadie\n", getpid());
                *tirado_I = 0;
                *tirado_D = 0;
                break;

            case 1:
                DEBUG_PRINT("[%d] Tiro al bolo de la izq (%d).\n", getpid(),
                       suBoloI);
                kill(suBoloI, SIGTERM);
                *tirado_I = 1;
                *tirado_D = 0;
                break;

            case 2:
                DEBUG_PRINT("[%d] Tiro al bolo de de la dcha (%d).\n", getpid(),
                       suBoloD);
                kill(suBoloD, SIGTERM);
                *tirado_I = 0;
                *tirado_D = 1;
                break;

            case 3:
                DEBUG_PRINT("[%d] Tiro ambos bolos (%d y %d)\n", getpid(), suBoloI,
                       suBoloD);
                kill(suBoloI, SIGTERM);
                kill(suBoloD, SIGTERM);
                *tirado_I = 1;
                *tirado_D = 1;
                break;
        }
    }
    else
    {
        DEBUG_PRINT("[%d] No tengo bolos debajo de mi, no tiro a nadie.\n",
               getpid());

        *tirado_I = 0;
        *tirado_D = 0;
    }

    /* Fin del código de verdad */

    return 0;
}

int imprimir_dibujo(pid_t pid_H, pid_t pid_I, pid_t pid_E, pid_t pid_B,
                     pid_t pid_C, int tirado_B, int tirado_C)
{
    int tirado[9] = {0};
    int num_caidos, stat;

    /*
     * El valor de retorno de los procesos es el número de bolos HIJOS que han
     * caído (incluído el que retorna). Por lo que si los bolos B y D caen, B
     * retornará 2. Así podemos saber el número de bolos que han caído en las
     * ristras.
     */
    /*
     * El código quedaba mejor haciendo estas dos no-bloqueantes :(
     * mala suerte supongo
     */
    if (tirado_B)
    {
        waitpid(pid_B, &stat, 0);
        num_caidos = WEXITSTATUS(stat);
        switch (num_caidos)
        {
            case 0:
                perror("num_caidos == 0 cuando tirado_B\n");
                return -1;

            /* switch truco 🤙*/
            case 3:
                tirado[5] = 1;
            case 2:
                tirado[2] = 1;
            case 1:
                tirado[0] = 1; 
        }
    }

    /*
     * Lo mismo pero para la ristra CFJ.
     */
    if (tirado_C)
    {
        waitpid(pid_C, &stat, 0);
        num_caidos = WEXITSTATUS(stat);

        switch (num_caidos)
        {
            case 0:
                perror("num_caidos == 0 cuando tirado_C\n");
                return -1;

            case 3:
                tirado[8] = 1;
            case 2:
                tirado[4] = 1;
            case 1:
                tirado[1] = 1;
        }
    }

    /* Para el resto de bolos, comprobamos si han devuelto o no, mediante la
     * versión no bloqueante de waitpid.
     * waitpid devuelve:
     *  -1 si no hay ningún hijo que haya terminado
     *  0 si el hijo no ha terminado
     *  pid del hijo si ha terminado
     */
    /* TODO: Comprobar que waitpid no sea -1 (error)? */
    if (waitpid(pid_E, NULL, WNOHANG) == pid_E) tirado[3] = 1;
    if (waitpid(pid_H, NULL, WNOHANG) == pid_H) tirado[6] = 1;
    if (waitpid(pid_I, NULL, WNOHANG) == pid_I) tirado[7] = 1;

    /* Imprimimos el dibujo */
    writefacil("\n");
    writefacil("   *\n"); /* A siempre está tirado */

    writefacil("  "); writefacil(tirado[0] ? "*" : "B"); writefacil(" "); 
    writefacil(tirado[1] ? "*" : "C"); writefacil("\n");

    writefacil(" "); writefacil(tirado[2] ? "*" : "D"); writefacil(" "); 
    writefacil(tirado[3] ? "*" : "E"); writefacil(" "); 
    writefacil(tirado[4] ? "*" : "F"); writefacil("\n");

    writefacil(tirado[5] ? "*" : "G"); writefacil(" ");
    writefacil(tirado[6] ? "*" : "H"); writefacil(" ");
    writefacil(tirado[7] ? "*" : "I"); writefacil(" ");
    writefacil(tirado[8] ? "*" : "J"); writefacil("\n");
    return 0;
}

pid_t ejecutar_ps(void)
{
    pid_t pid;
    switch (pid = fork()) {
        case -1:
            return -1;
        case 0:
            execlp("ps", "ps", "-fu", to_string(getuid()), NULL);
            perror("execlp ps");
            return -1;
        default:
            return pid;
    }
}

int elegir_accion(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return tv.tv_usec % 4;
}

int engendrar(int n, int *args, char *argv0_inicial, pid_t pid_P)
{
    // ai = (--n) * 3  =>> empiezo a contar por 1 (--n)
    // y cada bolo necesita 3 argumentos ( * 3 )
    int f;
    int ai = (--n) * 3; /* ai = argument index
                         * Índice del primer elemento de la última tupla (
                         * la del bolo que nos toca).
                         */

    pid_t suBoloI = args[ai + 1];
    pid_t suBoloD = args[ai + 2];
    char *name;

    switch ((f = fork()))
    {
        case -1:
            perror("Fallo al hacer fork para engendrar un hijo (3)\n");
            exit(3);

        case 0:
            /* Código del hijo, engendramos al siguiente bolo de la ristra
             * si es necesario
             */
            if (n > 0)
            {
                /* Si el subbolo de la izquierda es -1 (no existe), engendramos
                 * ese bolo.
                 * Si no, engendramos el subbolo de la derecha.
                 */
                if (suBoloI == -1)
                    suBoloI = engendrar(n, args, argv0_inicial, pid_P);
                else
                    suBoloD = engendrar(n, args, argv0_inicial, pid_P);
            } else {
                if (args[ai] == 'G') 
                    kill(pid_P, SIGUSR1);
                else if (args[ai] == 'J')
                    kill(pid_P, SIGUSR2);
            }

            /* Cuando un hijo es creado y termina de engendrar, ejecuta execl
             * para la función principal con el nombre cambiado y los siguientes
             * argumentos:
             *      [nombre, argv0_inicial, suBoloI, suBoloD]
             */
            name = malloc(2 * sizeof(char));
            if (name == NULL)
            {
                perror("Error al alocar memoria para nombre en engendrar");
                exit(2);
            }

            name[0] = args[ai];
            name[1] = 0;

            execl(argv0_inicial, name, argv0_inicial, to_string(suBoloI),
                  to_string(suBoloD), NULL);

            /* Si llegamos aquí, algo ha fallado */
            perror("Se ha llegado al tope de engendrar. (4)\n");
            exit(4);

        default:
            /* Somos el bolo padre (la primera tupla que se ha de engendrar),
             * devolvemos el pid del hijo que hemos creado.*/
            return f;
    }
}

char *to_string(int v)
{
    int len = (int)(ceil(log10(abs(v))) + 2);
    char *str = malloc(len * sizeof(char));
    if (str == NULL)
    {
        perror("Error al alocar memoria en to_string\n");
        exit(2);
    }

    sprintf(str, "%d", v);
    return str;
}

