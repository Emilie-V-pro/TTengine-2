# TTengine (Tu tournes engine)

## compilation

### prérequis

- la lib de dev de vulkan (libvulkan-dev sur ubuntu)

- la lib de glfw3 (libglfw3-dev sur ubuntu
)

- la lib de glm (libglm-dev)

- la lib openMP (libomp-dev)

- gcc, g++, cmake, make

- un driver vulkan 

- une taille de push constant suppérieur à 160 (vulkaninfo | grep maxPushConstantsSize
)

(voir le tuto de vulkan-tutorial pour mettre en place l'environement de développement https://vulkan-tutorial.com/Development_environment#page_Linux)

### compilation du projet


```bash
mkdir build && cd build

cmake ..

make TTengineApp -j 12
```
## lancement et contrôles

il suffit de lancer l'éxecutable `TTengineApp` depuis le dossier `build`.
Du texte devrais s'afficher dans le terminal et une fenêtre devrait se lancer.
En cas d'erreur broken pipe ou un truc comme, relancez, je sais pas d'où ça vient

### contrôles


Pour switcher avec le mode de controle de la camera, il faut appuyer sur `echape`.

La rotation de la caméra se fait à la souris et les déplacement avec `z q s d`

Le sprint se fait avec `shift`

la rotation se fait avec les flèche gauche et droite

avancé reculer se fait avec les flèche haut et bas

le coup de pied avec `espace`



Pour fermer la fenêtre il suffit juste de fermet la fenêtre (incroyable hein ? (faut pas faire gaffe au segfault))

