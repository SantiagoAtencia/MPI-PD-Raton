# MPI-PD-Raton

### Trabajo para la asignatura __Programación distribuida__

Diciembre 2024


Autores: _Santiago Atencia Jiménez, Sara Yue Comino Márquez, Claudio María Martínez Velasco, Javier Sánchez Alarcón_


Estudiantes de Grado en Ingeniería de Computadores, Universidad de Málaga

santiagoatencia@uma.es
sarayuecomar@uma.es
clamaver@uma.es
js.alarcon09@uma.es

___
## Maze-race

### _Inicio rápido:_ 
Abrir este [entorno](https://automatic-space-meme-6xw9p799q6g24r4.github.dev/) y en el terminal ejecutar `make run`
___



Programa desarrollado en _Codespaces_, no es necesario configurar nada localmente. Está asociado a un repositorio GitHUB. Si se quiere, se puede copiar a un linux local (git pull).

Repositorio gitHUB: https://github.com/SantiagoAtencia/MPI-PD-Raton

Entorno virtual Codespaces: https://automatic-space-meme-6xw9p799q6g24r4.github.dev/

El pruyecto tiene un _Makefile_. Con `make run` se compila y ejecuta automáticame.
En el _Makefile_ se pueden cambiar los parámetros de tamaño y tiempo.

Con `make install` se instala el entorno si fuera necesario. En el codespaces ya está todo instalado, lógicamente.

Lo podemos compilar y ejecutar manualmente pudiendo poner los parámetros de ejecución que deseemos.

Compilación manual (Ejecutar en este directorio):

```
mpicc -I include -o maze_race src/*.c
```

Ejecución manual:

```
mpirun -np <num_processes> ./maze_race <width> <height> <play time>
```

La versión actual:

  
  - Genera unos sub-laberintos de forma aleatoria.
  - Al juntarse se comprueban y corrigen



En la parte de la carrera hay 3 procesos:
 - rank 0. Es el controlador del juego
 - rank 1. Ratón
 - rank 2. Gato
### Algoritmos
*El controlador del juego:*
```  
  - Le pasa el laberinto a los animales.
  - Mantiene una copia local de las posiciones de los animales
  - Bucle:
      - Espera mensaje de algún animal (receive bloqueante)
      - Si es del ratón, actualiza su posición y lo pinta
      - Si es del gato, idem.
      - Comprueba alguna de las 3 condiciones de fin de juego: gato come, ratón llega a meta , o ha pasado un tiempo
      - Si "fin de juego":
          - Notifica a los animales para que paren
          - Se imprime razón de finalización.
          - Salir  
      - En otro caso: imprimir tiempo restante
      - Fin de bucle 
```
*Rutina de ratón/gato*
``` 
- Recibe el laberinto
- Bucle:
    - Espera un tiempo fijo (sleep)
    - Si hay un mensaje del controlador de juego: salir del bucle
    - Actualiza su posición
    - Envia posición a controlador del juego
    - Fin de bucle
```

