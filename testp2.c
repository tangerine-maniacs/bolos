#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

/* Prototipos de funciones */
void logica();
int numdigitos(int);
int engendrar(const char*, const char*, int, ...);
const char* nomhijo(char);

/*
 * Árbol de procesos:
 *
 *              P
 *              |
 *        ------A------
 *        |  |  |  |  |
 *        B  H  E  I  C
 *        |           |
 *        D           F
 *        |           |
 *        G           J
 *
 * Creamos primero H, E e I, para que B, D, C, y F sepan sus PIDs
 * y puedan mandarles señales.
 *
 */

void logica() {
  while (1) 
    pause();
  exit(0);
}

/*
 * Devuelve el nombre que debería tener el hijo de un bolo.
 * B->D, D->G, C->F, F->J.
 */
const char* nomhijo(char nompadre) {
  switch (nompadre) {
    case 'B':
      return "D";
    case 'D':
      return "G";
    case 'C':
      return "F";
    case 'F':
      return "J";
    /*
     * Default no se debería ejecutar nunca (nunca lo llamamos si el padre no 
     * es B, D, C o F), así que si se ejecuta avisamos de un error, y salimos
     * del programa.
     */
    default:
      fprintf(stderr, "Error: nomhijo(%c) llamado con un padre que no es "
          "B, D, C o F\n", nompadre);
      exit(4);
  }
}

/*
 * Devuelve el número de dígitos de un número, en base 10, utilizando 
 * logaritmos.
 */
int numdigitos(int n) {
  return (int) (log10(n) + 1);
}

/*
 * Engendra un hijo con el nombre 'nom_hijo'.
 * Se le pueden pasar argumentos opcionales del tipo pid_t, que se pasarán
 * al hijo como argumentos. El número de argumentos opcionales que se vayan
 * a pasar se debe indicar en 'num_pids'.
 *
 * Devuelve el PID del hijo.
 */
int engendrar(const char* nom_hijo, const char* nom_programa, int num_pids, ...)
{
  char** argv = NULL;
  pid_t pid_hijo, pid;
  int argv_size = 0;
  int i;

  va_list ap;
  va_start(ap, num_pids);

  switch (pid_hijo = fork())
  {
    case -1:
      perror("no pude hacer fork en engendrar :(\n");
      exit(3);

    case 0:
      argv = malloc(sizeof(char*) * (3 + num_pids));
      argv[argv_size++] = (char*) nom_hijo; /*cast a char pq son const char y
                                              si no el compilador se queja*/
      argv[argv_size++] = (char*) nom_programa;

      /*
       * Convertimos los pids que se han pasado como parámetros de esta
       * función en cadenas de caracteres.
       */
      for (i = 0; i < num_pids; i++)
      {
        pid = va_arg(ap, pid_t);
        argv[argv_size] = malloc(sizeof(char) * (numdigitos(pid) + 1));
        sprintf(argv[argv_size], "%d", pid);
        argv_size++;
      }
      argv[argv_size++] = NULL;

      /* 
       * Al hijo le pasamos en argv[0] el nombre del bolo, y en argv[1]
       * el nombre del programa, para que pueda hacer execl para engendrar
       * sus propios hijos.
       * También le pasamos los pids de los bolos que quedan debajo suya,
       * si es que los hay, para que pueda mandarles señales.
       */
      execv(nom_programa, argv);

      /*
       * Toda la memoria del proceso "bifurcado" se "libera" al llamar a execl()
       * (si no ha habido problemas con la llamada).
       * Por tanto, no tenemos que liberar argv.
       */

      /*
       * TODO: Comprobar que execv no dé problemas.
       */
      perror("No he podido engendrar a mi hijo por un problema con alguna de "
          "mis partes!\n");
      exit(13); // 13 porque mala suerte!

    default:
      return pid_hijo;
  }
}

/*
 * Esta función va a tener los siguientes argumentos:
 *   - cuando se ejecuta el programa: no tendrá ningún argumento.
 *        argc = 1, argv = {"bolos"}
 *   - cuando A, J, G, H, I:
 *        argc = 2, argv = {nombre del bolo, nombre del programa}
 *   - para el resto de los bolos:
 *        argc = 3-4 (depende del número de hijos que tengan que engendrar).
 *        argv = {nombre del bolo, nombre del programa, pid de bolo1, pid de bolo2}
 *     
 *     Estos bolos engedrarán 2-(argc-2)=4-argc hijos (si no se le han pasado
 *     bolos, engendrarán 2, si se le ha pasado 1 bolo, engendrá 1, y si se le
 *     han pasado 2, no engendrará ninguno).
 */
/*
 * TODO: Juntar los casos del switch de D, F, y B, C.
 */
int main(int argc, char *argv[])
{
  pid_t pid_H, pid_E, pid_I;

  if (argc == 0) {
    fprintf(stderr, 
        "No se me pasó el nombre del programa, ¡no sé quién soy!\n"
        "¡Me vuelvo loco y muero!\n");
    exit(1);
  }

  /*
   * "Al entrar por el main, miramos el valor de argv[0] y sabemos que si vale
   * bolos es la entrada inicial y si vale A es la segunda entrada del proceso 
   * A y lo podemos dirigir a una función propia."
   */
  if (strstr(argv[0], "bolos") != NULL) 
  {
    // Lógica de P
    printf("Soy P, engendro a A\n");
    engendrar("A", argv[0], -1, -1);
  }
  else 
  {
    if (argc < 2) {
      fprintf(stderr,
        "¡Ha pasado un error terrible! No sé cuál es el nombre de mi programa"
        "y no voy a poder tener hijos.\n ¡Me deprimo y muero de síndrome del"
        "corazón roto!\n");
      exit(2);
    }

    printf("Soy %s, el programa es %s\n", argv[0], argv[1]);
    /*
     * Lógica del resto de bolos.
     */
    switch (argv[0][0])
    {
      case 'A':
        printf("Soy A, engendro a B, H, E, I y C\n");
        pid_H = engendrar("H", argv[1], 0);
        pid_I = engendrar("I", argv[1], 0);

        pid_E = engendrar("E", argv[1], 2, pid_H, pid_I);

        /* 
         * Pasamos a B y C su bolo subordinado (E)
         * y el bolo subordinado de cada uno de sus hijos (H, I)
         */
        engendrar("B", argv[1], 2, pid_E, pid_H);
        engendrar("C", argv[1], 2, pid_E, pid_I);
        break;

      case 'B':
      case 'C':
        /* 
         * B y C tienen un bolo subordinado (argv[2]), y se les pasa un bolo
         * que va a ser subordinado de su hijo (argv[3]).
         */
        printf("Soy %s, debería tener un hijo, engendro a %s\n", argv[0], nomhijo(argv[0][0]));
        engendrar(nomhijo(argv[0][0]), argv[1], 1, atoi(argv[3]));
        break;

      case 'E':
        printf("Soy %s, no debería de tener hijos. Me han pasado %d bolos "
            "subordinados\n", argv[0], argc-2);
        break;

      case 'D':
      case 'F':
        /* D y F tienen un bolo subordinado (argv[2]), y tienen 1 hijo. */
        printf("Soy %s, debería tener un hijo, engendro a %s\n", argv[0], nomhijo(argv[0][0]));
        engendrar(nomhijo(argv[0][0]), argv[1], 0);
        break;

      case 'G':
      case 'H':
      case 'I':
      case 'J':
        printf("Soy %s, no debería tener hijos, tengo %d subordinados\n", 
            argv[0], argc-2);
        break;
    }

    // Sea cual sea el bolo, entramos en la lógica.
    logica();
  }

  return 0;
}

