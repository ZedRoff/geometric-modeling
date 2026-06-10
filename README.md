# Projet de Geometric Modeling

Ce projet consiste à implémenter une bibliothèque permettant de manipuler la structure half-edges. En passant par la simplification, la triangulation, la conversion d'uin .obj en une structure half-edges exploitable, l'implémentation de catmull-clark et plus.


## Tâches

Voici ce qui a été réalisé : 

- [x] readFile : conversion de .obj en structure halfedge
- [x] computeNormals : calculer les normales des faces et des vertex
- [x] silhouette : trouver les arêtes frontières pour afficher la silhouette du mesh
- [x] triangulation : la triangulation par ear-clipping de surfaces concaves et convexes
- [x] halk-edge data structure tests : implémentation de tests dans la méthode checkMesh() afin de vérifier la cohérence globale
- [x] surfaceRevolution : génération d'une figure à partir d'un profil, on place des points autour de l'axe Z et on fait tourner
- [x] simplify : simplifier un mesh en détectant l'edge la plus courte et en collapse cette dernière. 30% des edges seront supprimées
- [x] catmull-clark : implémentation de la méthode catmull-clark qui consiste à diviser les faces en quadrangles en cherchant le centre d'arêtes et de faces, puis lisser les points existants

## Précisions

La méthode checkMesh() est appelée à la fin de chaque opération afin de s'assurer de la cohérence.

Le code a été commenté afin d'expliquer ce qui a été réalisé, au dessus de chaque fonction développée une section "Ce qui a été fait" est disponible à la lecture.

Un bouton "Surface of Revolution" est disponible dans le menu

Les TPs ont été réalisé avec un mac intel, des adaptations par rapport au squelette de base ont dû être réalisées notammene tdans le fichier CMakeLists.txt

## Usage de l'IA

Gemini a été utilisé lors de la réalisation des TPs afin de : 
- Comprendre mieux les principes du cours et leur implémentation en C++
- Trouver les méthodes nécessaires afin de réaliser le code à partir d'un pseudo code dressé par mes soins (std::map, make_pair, etc. leur fonctionnement m'a été expliqué par l'ia et j'ai intégré)
- La formule de catmull-clark
- Pour les libérations de mémoires et le fait de supprimer les faces/points/demi-arêtes nécessaires afin de gérer la cohérence


Les codes générés explicitement par l'ia et intégré brut ont été marqué d'une balise (IA)

## Compilation du projet

1. Changer le fichier CMakeLists.txt en fonction de votre configuration
2. Se placer dans la racine, lancer cmake .
3. Compiler le projet avec make ./MeshViewer
4. Le lancer avec ./MeshViewer

## Résultats

Voici les résultats visuels : 

- Catmull clark sur un cube, deux itérations

![alt text](screenshots/catmull_clark_on_cube_2_iter.png)

- computeNormals sur dolphin

![alt text](screenshots/compute_normals_on_dolphin.png)

- Ear clipping sur gear

![alt text](screenshots/ear_clipping_on_gear.png)

- Ear clipping sur c_gear

![alt text](screenshots/ear_clipping_on_c_gear.png)

- Main avec le calcul des normales

![alt text](screenshots/hand_with_normals.png)

- Main sans le calcul des normales

![alt text](screenshots/hand_without_normals.png)

- Silhouette sur dolphin

![alt text](screenshots/silhouette.png)

- Simplify sur cube, 1 itération

![alt text](screenshots/simplify_on_cube_1_iter.png)

- Surface de revolution avec le profil suivant : 
(profil ia)
```
 profile.push_back(myPoint3D(0.00, -0.50, 0.0)); 
    profile.push_back(myPoint3D(0.20, -0.50, 0.0));  
    profile.push_back(myPoint3D(0.25, -0.40, 0.0));  
    profile.push_back(myPoint3D(0.45, -0.10, 0.0));  
    profile.push_back(myPoint3D(0.35,  0.20, 0.0)); 
    profile.push_back(myPoint3D(0.15,  0.40, 0.0)); 
    profile.push_back(myPoint3D(0.22,  0.50, 0.0));  
```
![alt text](screenshots/surface_of_revolution.png)