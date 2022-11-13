#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

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

// Engendra un hijo con el nombre 
int engendrar(char* nom_hijo, char* nom_programa)
{
  pid_t pid;
  switch (pid = fork())
  {
    case -1:
      perror("no pude hacer fork en engendrar :(\n");
      exit(3);

    case 0:
      /* 
       * Al hijo le pasamos en argv[0] el nombre del bolo, y en argv[1]
       * el nombre del programa, para que pueda hacer execl para engendrar
       * sus propios hijos.
       */
      execl(nom_programa, nom_hijo, nom_programa, NULL);

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
    engendrar("A", argv[0]);
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
        engendrar("H", argv[1]);
        engendrar("I", argv[1]);

        engendrar("E", argv[1]);
        engendrar("B", argv[1]);
        engendrar("C", argv[1]);
        break;

    }

  }

  return 0;
}

