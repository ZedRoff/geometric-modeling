#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>

// #include <GL/glew.h>
#include "myVector3D.h"

using namespace std;

myMesh::myMesh(void) { 
  vertices.clear();
  halfedges.clear();
  faces.clear();
  name = "";
 }

myMesh::~myMesh(void) {
  clear();
}

void myMesh::clear() {
  for (unsigned int i = 0; i < vertices.size(); i++)
    if (vertices[i])
      delete vertices[i];
  for (unsigned int i = 0; i < halfedges.size(); i++)
    if (halfedges[i])
      delete halfedges[i];
  for (unsigned int i = 0; i < faces.size(); i++)
    if (faces[i])
      delete faces[i];

  vector<myVertex *> empty_vertices;
  vertices.swap(empty_vertices);
  vector<myHalfedge *> empty_halfedges;
  halfedges.swap(empty_halfedges);
  vector<myFace *> empty_faces;
  faces.swap(empty_faces);
}

void myMesh::checkMesh() {
  vector<myHalfedge *>::iterator it;
  for (it = halfedges.begin(); it != halfedges.end(); it++) {
    if ((*it)->twin == NULL)
      break;
  }
  if (it != halfedges.end())
    cout << "Error! Not all edges have their twins!\n";
  else
    cout << "Each edge has a twin!\n";
}

/*
  Explication de cette partie : 
  Cette fonction lit un fichier .obj et construit le maillage structure halfedges.
*/
bool myMesh::readFile(std::string filename) {
  string s, t, u;
  vector<int> faceids;
  myHalfedge **hedges;

  ifstream fin(filename);
  if (!fin.is_open()) {
    cout << "Unable to open file!\n";
    return false;
  }
  name = filename;

  map<pair<int, int>, myHalfedge *> twin_map;
  map<pair<int, int>, myHalfedge *>::iterator it;

  while (getline(fin, s)) {
    stringstream myline(s);
    if (!(myline >> t)) 
        continue;
    if (t == "g") {
    } else if (t == "v") {
      float x, y, z;
      myline >> x >> y >> z;
      cout << "v " << x << " " << y << " " << z << endl;
      myPoint3D *p = new myPoint3D(x, y, z);
      myVertex *v = new myVertex();
      v->point = p;
      vertices.push_back(v);
    } else if (t == "mtllib") {
    } else if (t == "usemtl") {
    } else if (t == "s") {
    } else if (t == "f") {
    faceids.clear();

    while (myline >> u)
        faceids.push_back(atoi(u.substr(0, u.find("/")).c_str()) - 1);

    if (faceids.size() < 3)
        continue;

    // créer face
    myFace *f = new myFace();

    // créer halfedges
    hedges = new myHalfedge*[faceids.size()];
    for (unsigned int i = 0; i < faceids.size(); i++)
        hedges[i] = new myHalfedge();

    // connecter face
    f->adjacent_halfedge = hedges[0];

    // boucle principale
    for (unsigned int i = 0; i < faceids.size(); i++) {
        int iplusone = (i + 1) % faceids.size();
        int iminusone = (i - 1 + faceids.size()) % faceids.size();

        // vertex source
        hedges[i]->source = vertices[faceids[i]];

        // face
        hedges[i]->adjacent_face = f;

        // next / prev
        hedges[i]->next = hedges[iplusone];
        hedges[i]->prev = hedges[iminusone];

        // twin
        pair<int,int> edge = make_pair(faceids[i], faceids[iplusone]);
        pair<int,int> twin_edge = make_pair(faceids[iplusone], faceids[i]);

        auto it = twin_map.find(twin_edge);
        // twin existe, connexion halfedges
        if (it != twin_map.end()) {
            hedges[i]->twin = it->second;
            it->second->twin = hedges[i];
        } else {
            twin_map[edge] = hedges[i];
        }

        // origine du vertex
        if (!vertices[faceids[i]]->originof)
            vertices[faceids[i]]->originof = hedges[i];

        // stocker
        halfedges.push_back(hedges[i]);
    }

    // ajouter face
    faces.push_back(f);

    delete[] hedges;
}
  }

  checkMesh();
  normalize();

  return true;
}

/*
  Explication de cette partie : 

  On utilise les deux méthodes computeNormals() développées dans myVertex.cpp et myFace.cpp afin de calculer les normales des faces et des halfEdges
*/
void myMesh::computeNormals() { 
  for (int i = 0; i < faces.size(); i++)
    faces[i]->computeNormal();
  for (int i = 0; i < vertices.size(); i++)
    vertices[i]->computeNormal();
}


void myMesh::normalize() {
  if (vertices.size() < 1)
    return;

  int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0,
      tmpzmax = 0;

  for (unsigned int i = 0; i < vertices.size(); i++) {
    if (vertices[i]->point->X < vertices[tmpxmin]->point->X)
      tmpxmin = i;
    if (vertices[i]->point->X > vertices[tmpxmax]->point->X)
      tmpxmax = i;

    if (vertices[i]->point->Y < vertices[tmpymin]->point->Y)
      tmpymin = i;
    if (vertices[i]->point->Y > vertices[tmpymax]->point->Y)
      tmpymax = i;

    if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z)
      tmpzmin = i;
    if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z)
      tmpzmax = i;
  }

  double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
         ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
         zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

  double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
  scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

  for (unsigned int i = 0; i < vertices.size(); i++) {
    vertices[i]->point->X -= (xmax + xmin) / 2;
    vertices[i]->point->Y -= (ymax + ymin) / 2;
    vertices[i]->point->Z -= (zmax + zmin) / 2;

    vertices[i]->point->X /= scale;
    vertices[i]->point->Y /= scale;
    vertices[i]->point->Z /= scale;
  }
}

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p) { /**** TODO ****/ }

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p) { /**** TODO ****/ }

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p) { /**** TODO ****/ }

void myMesh::subdivisionCatmullClark() { /**** TODO ****/ }

void myMesh::simplify() { /**** TODO ****/ }

void myMesh::simplify(myVertex *) { /**** TODO ****/ }

/*
  Explication de cette partie :
  Cette fonction utilitaire permet de détermminer si le vertex est dans l'oreille formée par les sommets a, b et c. 

  Pour ce faire, j'ai utilisé la méthode du cours sur les coordonnées barycentrique, qui permet de déterminer si un point est dans un triangle.
*/
bool isVertexInsideEar(myVertex *p, myVertex *a, myVertex *b, myVertex *c, myVector3D *N)
{
    // Calcul de AB, AC et AP, vecteurs formés par les sommets de l'oreille et le point à tester
    myVector3D vAB = *(b->point) - *(a->point);
    myVector3D vAC = *(c->point) - *(a->point);
    myVector3D vAP = *(p->point) - *(a->point);

    // Calcul de l'aire du triangle ABC (formule du produit vectoriel cf. cours de l'année dernière)
    myVector3D crossTotal = vAB.crossproduct(vAC);
    double doubleAreaTotal = crossTotal * (*N);

    // Calcul de u (poids associé au sommet b) 
    myVector3D crossU = vAP.crossproduct(vAC);
    double doubleAreaU = crossU * (*N);

    // Calcul de v (poids associé au sommet c) 
    myVector3D crossV = vAB.crossproduct(vAP);
    double doubleAreaV = crossV * (*N);

    // Normalisation des aires pour le test 
    double u = doubleAreaU / doubleAreaTotal;
    double v = doubleAreaV / doubleAreaTotal;

    return (u >= 0 && v >= 0 && (u + v) <= 1.0);
}

/*
  Explication de cette partie : 

  Cette fonction implémente l'algorithme d'Ear Clipping pour la triangulation d'une face. 

  L'idée est de trouver une "oreille" du polygone (un triangle formé par trois sommets consécutifs du polygone, tel que le triangle est entièrement contenu dans le polygone et ne contient aucun autre sommet du polygone), de la couper et de répéter l'opération jusqu'à ce qu'il ne reste plus que des triangles.

  On utilise une liste dynamique des demi-arêtes encore actives dans le polygone (IA), et on vérifie à chaque étape si une oreille est trouvée et peut être coupée. Si aucune oreille n'est trouvée, alors la triangulation échoue.
*/
bool myMesh::triangulate(myFace *face)
{
    //  On compte le nombre de sommets de la face
    int vertexCount = 0;
    myHalfedge *loopPointer = face->adjacent_halfedge;
    do
    {
        vertexCount++;
        loopPointer = loopPointer->next;
    } while (loopPointer != face->adjacent_halfedge);

    if (vertexCount <= 3)
        return false; 

    // On calcule la normale de la face
    face->computeNormal();
    myVector3D areaNormal = *(face->normal); 

    if (areaNormal.length() < 1e-10)
        return false;

    // Liste dynamique des demi-arêtes encore actives dans le polygone
    std::vector<myHalfedge *> activeHalfedges(vertexCount);
    loopPointer = face->adjacent_halfedge;
    for (int i = 0; i < vertexCount; i++)
    {
        activeHalfedges[i] = loopPointer;
        loopPointer = loopPointer->next;
    }

    int currentIdx = 0;

    // Boucle de Ear Clipping
    while (activeHalfedges.size() > 3)
    {
        bool earClipped = false;
        int startIdx = currentIdx;
        do
        {
            int size = activeHalfedges.size();
            // Gestion circulaire propre des voisins via la taille du vecteur (IA)
            int prevIdx = (currentIdx - 1 + size) % size;
            int nextIdx = (currentIdx + 1) % size;

            myHalfedge *hePrev = activeHalfedges[prevIdx];
            myHalfedge *heCurr = activeHalfedges[currentIdx];
            myHalfedge *heNext = activeHalfedges[nextIdx];

            myVertex *vPrev = hePrev->source;
            myVertex *vCurr = heCurr->source;
            myVertex *vNext = heNext->source;

            myVector3D vecU = *(vCurr->point) - *(vPrev->point);
            myVector3D vecV = *(vNext->point) - *(vCurr->point);

            // On vérifie si on a la convexité
            if ((vecU.crossproduct(vecV)) * areaNormal > 0)
            {
                bool isEarValid = true;
                
                // On vérifie si un des autres sommets est dans l'oreille
                for (int i = 0; i < size; i++)
                {
                    if (i == currentIdx || i == prevIdx || i == nextIdx)
                        continue;

                    if (isVertexInsideEar(activeHalfedges[i]->source, vCurr, vPrev, vNext, &areaNormal))
                    {
                        isEarValid = false;
                        break;
                    }
                }
                
                if (isEarValid)
                {
                    // Création des deux nouvelles demi-arêtes formant la diagonale de l'oreille
                    myHalfedge *diagonalA = new myHalfedge();
                    myHalfedge *diagonalB = new myHalfedge();

                    // Attribution des sources et des twins
                    diagonalA->source = vNext;
                    diagonalB->source = vPrev;

                    // Création du lien de twins entre les deux demi-arêtes
                    diagonalA->twin = diagonalB;
                    diagonalB->twin = diagonalA;
                    halfedges.push_back(diagonalA);
                    halfedges.push_back(diagonalB);

                    // Création de la nouvelle face formée par l'oreille
                    myFace *subFace = new myFace();
                    faces.push_back(subFace);
                    subFace->adjacent_halfedge = hePrev;

                    // Connexion des demi-arêtes de l'oreille entre elles et avec la diagonale
                    hePrev->next = heCurr;
                    heCurr->next = diagonalA;
                    diagonalA->next = hePrev;
                    hePrev->prev = diagonalA;
                    heCurr->prev = hePrev;
                    diagonalA->prev = heCurr;
                  
                    // Attribution de la face aux demi-arêtes de l'oreille
                    hePrev->adjacent_face = subFace;
                    heCurr->adjacent_face = subFace;
                    diagonalA->adjacent_face = subFace;

                    // Mise à jour de la face d'origine pour le triangle restant
                    activeHalfedges[prevIdx] = diagonalB;

                    // On supprime l'arête courante (heCurr) du polygone restant (IA)
                    activeHalfedges.erase(activeHalfedges.begin() + currentIdx);

                    // On réinitialise l'index pour continuer à parcourir le polygone restant (IA)
                    currentIdx = prevIdx % activeHalfedges.size();
                    earClipped = true;
                    break;
                }
            }

            currentIdx = (currentIdx + 1) % activeHalfedges.size();

        } while (currentIdx != startIdx);
        // Si pas d'oreille coupée, alors false
        if (!earClipped) {
            return false; 
        }
    }

    // Connexion des trois dernières demi-arêtes restantes pour former le dernier triangle
    myHalfedge *heA = activeHalfedges[0];
    myHalfedge *heB = activeHalfedges[1];
    myHalfedge *heC = activeHalfedges[2];

    face->adjacent_halfedge = heA;

    heA->next = heB;
    heB->next = heC;
    heC->next = heA;

    heA->prev = heC;
    heB->prev = heA;
    heC->prev = heB;

    heA->adjacent_face = face;
    heB->adjacent_face = face;
    heC->adjacent_face = face;

    return true;
}


/*
  Explication de cette partie : 
  On itère sur toutes les faces du maillage et on applique la fonction de triangulation à chacune d'entre elles.
*/
void myMesh::triangulate() {
    for (unsigned int i = 0; i < faces.size(); i++)
        triangulate(faces[i]);
        
}