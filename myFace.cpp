#include "myFace.h"
#include "myVector3D.h"
#include "myHalfedge.h"
#include "myVertex.h"
//#include <GL/glew.h>

myFace::myFace(void)
{
	adjacent_halfedge = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
}

myFace::~myFace(void)
{
	if (normal) delete normal;
}


/*
	Explication de cette partie :

	On récupère les 3 premiers sommets de la face, et détermine trois points A, B et C constituant la face.
	Pour récupérer A, B et C : 
	- A : directement la source
	- B : La source du next
	- C : La source du next->next

	Et on calcul le produit vectoriel des vecteurs AB et AC ainsi formé, comme expliqué en cours.

	On normalise le vecteur à la fin
*/
void myFace::computeNormal()
{

    // On récupère les 3 premiers sommets de la face
    myPoint3D &A = *(adjacent_halfedge->source->point);
    myPoint3D &B = *(adjacent_halfedge->next->source->point);
    myPoint3D &C = *(adjacent_halfedge->next->next->source->point);

    // Calcul des vecteurs AB et AC, produit vectoriel et normalisation
    normal->crossproduct(B - A, C - A);
    normal->normalize();

}