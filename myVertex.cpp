#include "myVertex.h"
#include "myFace.h"
#include "myHalfedge.h"
#include "myVector3D.h"


myVertex::myVertex(void) {
  point = NULL;
  originof = NULL;
  normal = new myVector3D(1.0, 1.0, 1.0);
}

myVertex::~myVertex(void) {
  if (normal) delete normal;
}

/*
  Explication de cette partie : 
  Pour déterminer la normale d'un point, on prend les faces autour du point d'origine et on somme les normales de ces faces. 

  Pour ce faire, on utilise l'algorithme du cours qui permet de naviguer autour d'un vertex.
*/
void myVertex::computeNormal() { 
  // Si le sommet n'a pas d'arête de départ, on ne fait rien
  if (!originof) return;

  myHalfedge *h = originof;
  normal->dX = 0.0;
  normal->dY = 0.0;
  normal->dZ = 0.0;

  // Algorithme du cours pour naviguer autour du vertex
  do {
    if (h->adjacent_face && h->adjacent_face->normal) {
      normal->dX += h->adjacent_face->normal->dX;
      normal->dY += h->adjacent_face->normal->dY;
      normal->dZ += h->adjacent_face->normal->dZ;
    }

    // détection bord ouvert
    if (h->twin != nullptr) {
      h = h->twin->next;
    } else {
    
      break; 
    }

  } while (h != originof && h != nullptr);

  normal->normalize();
}