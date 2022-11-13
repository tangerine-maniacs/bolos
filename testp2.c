#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Prototipos de funciones */
void logica();
int numdigitos(int);
int engendrar(const char*, const char*, pid_t, pid_t);
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
 * Se le pueden pasar dos argumentos opcionales: dos pid_t, que se pasarán
 * al hijo como argumentos. El valor por defecto (si no se pasan), debería
 * ser -1.
 *
 * Devuelve el PID del hijo.
 */
int engendrar(const char* nom_hijo, const char* nom_programa, pid_t pid1, pid_t pid2)
{
  char** argv = NULL;
  pid_t pid;
  int argv_size = 0;

  switch (pid = fork())
  {
    case -1:
      perror("no pude hacer fork en engendrar :(\n");
      exit(3);

    case 0:
      /*
       * argv va a tener un tamaño mínimo de 3 (nom_programa, nom_hijo, NULL),
       * y ese tamaño aumentará en 1 por cada pid que se le pase.
       * Como los pidx que no se pasan son -1, `pidx != -1` devuelve 1 si pidx 
       * se ha pasado, y 0 si no, por tanto suma 1 por cada pid que se le pase.
       */
      argv = malloc(sizeof(char*) * (3 + (pid1 != -1) + (pid2 != -1)));
      argv[argv_size++] = (char*) nom_hijo; /*cast a char pq son const char y
                                              si no el compilador se queja*/
      argv[argv_size++] = (char*) nom_programa;

      /*
       * Convertimos los pids que se han pasado como parámetros de esta
       * función en cadenas de caracteres.
       */
      if (pid1 != -1)
      {
        argv[argv_size] = malloc(numdigitos(pid1) + 1);
        sprintf(argv[argv_size], "%d", pid1);
        argv_size++;
      }
      if (pid2 != -1)
      {
        argv[argv_size] = malloc(numdigitos(pid2) + 1);
        sprintf(argv[argv_size], "%d", pid2);
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
      return pid;
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
        pid_H = engendrar("H", argv[1], -1, -1);
        pid_I = engendrar("I", argv[1], -1, -1);

        pid_E = engendrar("E", argv[1], pid_H, pid_I);
        engendrar("B", argv[1], pid_E, -1);
        engendrar("C", argv[1], pid_E, -1);
        break;
      case 'G':
      case 'H':
      case 'I':
      case 'J':
        printf("Soy %s, no debería tener hijos, tengo %d subordinados\n", 
            argv[0], argc-2);
        break;
      default:
        printf("Soy %s, debería tener %d hijos. Me han pasado %d bolos "
            "subordinados\n", argv[0], 4-argc, argc-2);
        if (4-argc > 0) {
          /*
           * NOTA: Nunca deberíamos engendrar más de un hijo, así que ni siquiera
           * merece la pena hacer un bucle.
           */
          engendrar(nomhijo(argv[0][0]), argv[1], -1, -1);
        }
    }

  }

  return 0;
}

