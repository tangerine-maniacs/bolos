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

#define __USE_POSIX

/*
 * Función principal de todos los bolos una vez creados
 */
int mente(pid_t suBoloI, pid_t suBoloD, int *tiradoI, int *tiradoD);

int engendrar(int n, int *args, char *bolos);
/* Convierte un entero a un string */
char *toString(int v);
/* Función vacía para pasársela a sigaction */
void nonada(int signum);
/* Utiliza gettimeofday para decidir qué va a hacer el bolo una vez lo han
 * tirado */
int elegir_accion(void);
pid_t ejecutar_ps(void);
int imprimir_dibujo(pid_t pid_H, pid_t pid_I, pid_t pid_E, pid_t pid_B,
                     pid_t pid_C, int tirado_B, int tirado_C);

int main(int argc, char *argv[])
{
    int *args, retorno_mente, tirado_B, tirado_C;
    pid_t pid_H, pid_I, pid_E, pid_B, pid_C;
    pid_t pid_ps;

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
                printf("P muere\n");
                exit(0);
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
            printf("A: %d\n", getpid());
            break;

        case 4:
            /* Tenemos 4 argumentos:
             *   - nombre del bolo
             *   - argv[0] original
             *   - pid del bolo de la izquierda
             *   - pid del bolo de la derecha
             * Este es el caso para cualquier bolo que no sea A.
             */
            exit(mente(atoi(argv[2]), atoi(argv[3]), NULL, NULL));
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
    pid_H = engendrar(1, args, argv[1]);

    args[0] = 'I'; args[1] = -1; args[2] = -1;
    pid_I = engendrar(1, args, argv[1]);

    /* === Engendramos a E con H e I como hijos                          ===
     */
    args[0] = 'E'; args[1] = pid_H; args[2] = pid_I;
    pid_E = engendrar(1, args, argv[1]);

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
    pid_B = engendrar(3, args, argv[1]);

    /* === Para CFJ, le pasamos a engendrar los datos de C, y sus hijos, ===
     * === F y J.                                                        ===
     */
    args[0] = 'J'; args[1] = -1; args[2] = -1;  // J no tiene suBolos
    args[3] = 'F'; args[4] = pid_I; args[5] = -1;  // F tiene I como suBoloI
    args[6] = 'C'; args[7] = pid_E; args[8] = -1;  // C tiene E como suBoloI
    pid_C = engendrar(3, args, argv[1]);

    free(args); /* Liberamos la memoria del malloc anterior. */

    /* 
     * Ejecutamos mente para A, como si fuera un bolo normal, y luego
     * realizamos el comportamiento exclusivo de A.
     */
    retorno_mente = mente(pid_B, pid_C, &tirado_B, &tirado_C);
    if (retorno_mente != 0)   /* Si mente ha fallado, salimos */
        return retorno_mente;

    /* Dormir */
    printf("A duerme durante 4 segundos...\n");
    sleep(4);
    /* Imprimir dibujo */
    imprimir_dibujo(pid_H, pid_I, pid_E, pid_B, pid_C, tirado_B, tirado_C);

    /* Usar ps -fu */
    printf("Aquí tienes el ps -fu usuario, para verificar que todo está bien:\n");
    pid_ps = ejecutar_ps(); 
    if (pid_ps == -1)
    {
        perror("Error al ejecutar ps -fu $USER\n");
        return 1;
    }
    waitpid(pid_ps, NULL, 0);
    printf("Fin del ps -fu $USER\n");

    /* Matar */
    printf("Mato a todos los procesos hijos\n");
    kill(0, SIGINT);

    return 0;
}

void nonada(int signum) {}
/*
 * Esta función se encarga de manejar la lógica de los bolos una vez se han
 * creado.
 */
int mente(pid_t suBoloI, pid_t suBoloD, int *tiradoI, int *tiradoD)
{
    /*
     * Creamos un conjunto de bloqueo de señales con SIGTERM, y otro sin
     * SIGTERM. También guardamos el conjunto de señales viejo, para poder
     * restaurarlo cuando terminemos.
     */
    sigset_t conjunto_SIGTERM, conjunto_viejo, conjunto_sin_SIGTERM,
        conjunto_vacio;
    struct sigaction accion_nueva, accion_vieja;

    /*
     * Creamos una máscara de bloqueo de señales que tenga sólo SIGTERM.
     * Guardamos el conjunto de señales viejo para poder restaurarlo cuando
     * terminemos.
     */
    sigemptyset(&conjunto_SIGTERM);
    sigaddset(&conjunto_SIGTERM, SIGTERM);
    if (sigprocmask(SIG_BLOCK, &conjunto_SIGTERM, &conjunto_viejo) == -1)
        return 1;

    /*
     * Creamos una estructura de sigaction como la vieja pero sin SIGTERM.
     * Para ello, hacemos la copia del conjunto viejo y le quitamos la señal
     * de SIGTERM.
     */
    conjunto_sin_SIGTERM = conjunto_viejo;
    sigdelset(&conjunto_sin_SIGTERM, SIGTERM);

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
    if (sigaction(SIGTERM, &accion_nueva, &accion_vieja) == -1) return 1;

    /* Aquí ya tenemos código de verdad. */
    if (suBoloI != -1)
        printf("%d has %d and %d as sub-pins\n", getpid(), suBoloI, suBoloD);
    else
        printf("%d doesnt have sub-pins :(\n", getpid());

    /*
     * Esperamos a que nos llegue una señal de SIGTERM. Significaría
     * que nos han tirado
     */
    sigsuspend(&conjunto_sin_SIGTERM);
    /* Me tiraron :( */
    printf("[%d] He sido tirado :(\n", getpid());

    /* Si tenemos bolos debajo de nosotros, procedemos a tirarlos. */
    if (suBoloI != -1 && suBoloD != -1)
    {
        switch (elegir_accion())
        {
            case 0:
                printf("[%d] No tiro a nadie\n", getpid());
                break;
            case 1:
                printf("[%d] Tiro al bolo de la izq (%d).\n", getpid(),
                       suBoloI);
                kill(suBoloI, SIGTERM);
                *tiradoI = 1;
                break;
            case 2:
                printf("[%d] Tiro al bolo de de la dcha (%d).\n", getpid(),
                       suBoloD);
                kill(suBoloD, SIGTERM);
                *tiradoD = 1;
                break;
            case 3:
                printf("[%d] Tiro ambos bolos (%d y %d)\n", getpid(), suBoloI,
                       suBoloD);
                kill(suBoloI, SIGTERM);
                kill(suBoloD, SIGTERM);
                *tiradoI = 1;
                *tiradoD = 1;
                break;
        }
    }
    else
    {
        printf("[%d] No tengo bolos debajo de mi, no tiro a nadie.\n",
               getpid());
    }

    /* Fin del código de verdad */

    /*
     * Restauramos el conjunto de señales viejo, y la acción vieja de SIGTERM.
     */
    if (sigaction(SIGTERM, &accion_vieja, NULL) == -1) return 1;
    if (sigprocmask(SIG_SETMASK, &conjunto_viejo, NULL) == -1) return 1;

    return 0;
}

int imprimir_dibujo(pid_t pid_H, pid_t pid_I, pid_t pid_E, pid_t pid_B,
                     pid_t pid_C, int tirado_B, int tirado_C)
{
  /* TODO: Comprobar que haya puesto los números que corresponden a las letras
   * bien
   */
  /* Hacemos comprobaciones para saber si los bolos están tirados o no
   * (B->tirado[0], C->tirado[1], D->tirado[2]...)
   */
  int tirado[9] = {0};
  int num_en_pie, stat;

  if (tirado_B) {
    tirado[0] = 1; /* B tirado*/
    /* Comprobar ristra BDG. Para ello vemos el resultado que ha devuelto
     * B. 
     * Si es 0, entonces no hay ningún bolo tirado (nunca va a pasar).
     * Si es 1, entonces sólo está tirado B.
     * Si es 2, entonces están tirados B y D.
     * Si es 3, entonces están tirados B, D y G.
     */

    waitpid(pid_B, &stat, 0);
    num_en_pie = WEXITSTATUS(stat);
    switch (num_en_pie) {
      case 0: /* Nunca va a pasar */
        return -1;
      case 1: /* B tirado */
        break;
      case 2: /* B y D tirados */
        tirado[2] = 1;
        break;
      case 3: /* B, D y G tirados */
        tirado[2] = 1;
        tirado[5] = 1;
        break;
    }

  }

  if (tirado_C) {
    tirado[1] = 1; /* C tirado*/
    /* Comprobar ristra CEF. Análogo a BDG */
    
    waitpid(pid_C, &stat, 0);
    num_en_pie = WEXITSTATUS(stat);
    switch (num_en_pie) {
      case 0: /* Nunca va a pasar */
        return -1;
      case 1: /* C tirado */
        break;
      case 2: /* E y F tirados */
        tirado[3] = 1;
        break;
      case 3: /* C, E y F tirados */
        tirado[3] = 1;
        tirado[4] = 1;
        break;
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
  printf("   *\n"); /* A siempre está tirado */
  printf("  %c %c\n", tirado[0] ? '*' : 'B', tirado[1] ? '*' : 'C');
  printf(" %c %c %c\n", tirado[2] ? '*' : 'D', tirado[3] ? '*' : 'E',
         tirado[4] ? '*' : 'F');
  printf("%c %c %c %c\n", tirado[5] ? '*' : 'G', tirado[6] ? '*' : 'H',
         tirado[7] ? '*' : 'I', tirado[8] ? '*' : 'J');

  return 0;
}

pid_t ejecutar_ps(void)
{
    pid_t pid;
    switch (pid = fork()) {
        case -1:
            return -1;
        case 0:
            execlp("ps", "ps", "-fu", getenv("USER"), NULL);
            return -1;
        default:
            return pid;
    }
}

int elegir_accion(void)
{
    struct timeval tv;
    return 3;

    /* TODO: Quitar el return de arriba. Por ahora devuelve siempre 3. */
    gettimeofday(&tv, NULL);
    return tv.tv_usec % 4;
}

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
int engendrar(int n, int *args, char *argv0_inicial)
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
                    suBoloI = engendrar(n, args, argv0_inicial);
                else
                    suBoloD = engendrar(n, args, argv0_inicial);
            }

            /* Cuando un hijo es creado y termina de engendrar, ejecuta execl
             * para la función principal con el nombre cambiado y los siguientes
             * argumentos:
             *      [nombre, argv0_inicial, suBoloI, suBoloD]
             */
            // TODO: Esto en un ctostr()?
            //  ~ ctostr no le gusta a mi compilador :(
            name = malloc(2 * sizeof(char));
            if (name == NULL)
            {
                fprintf(stderr, "Error al alocar memoria en %c\n", name[0]);
                exit(2);
            }

            name[0] = args[ai];
            name[1] = 0;

            execl(argv0_inicial, name, argv0_inicial, toString(suBoloI),
                  toString(suBoloD), NULL);

            /* Si llegamos aquí, algo ha fallado */
            perror("Se ha llegado al tope de engendrar. (4)\n");
            exit(4);

        default:
            /* Somos el bolo padre (la primera tupla que se ha de engendrar),
             * devolvemos el pid del hijo que hemos creado.*/
            return f;
    }
}

char *toString(int v)
{
    int len = (int)(ceil(log10(abs(v))) + 2);
    char *str = malloc(len * sizeof(char));
    if (str == NULL)
    {
        perror("Error al alocar memoria en toString\n");
        exit(2);
    }

    sprintf(str, "%d", v);
    return str;
}
