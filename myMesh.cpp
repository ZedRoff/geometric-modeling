#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>
#include <algorithm>
#include <unordered_set>
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
   
   cout << "Début tests du cours" << endl;
    int errors = 0;

    // Tests sur les demi-arêtes
    for (int i = 0; i < halfedges.size(); i++) {
        myHalfedge* h = halfedges[i];
        // On vérifie qu'aucune demi-arête n'est NULL
        if (h == NULL) {
            cout << "Erreur : La demi-arete " << i << " est NULL." << endl;
            errors++;
            continue;
        }
        // On vérifie si chaque demi-arête a une source
        if (h->source == NULL) {
            cout << "Erreur : La demi-arete " << i << " n'a pas de sommet source." << endl;
            errors++;
        }

        // On vérifie si depuis une arête on peut aller à la suivante
        if (h->next == NULL) {
            cout << "Erreur : La demi-arete " << i << " n'a pas de pointeur 'next'." << endl;
            errors++;
        // On vérifie que aller à l'arête suivante et revenir en arrière est cohérent
        } else if (h->next->prev != h) {
            cout << "Erreur : Le 'next->prev' de la demi-arete " << i << " ne revient pas sur elle-meme." << endl;
            errors++;
        }

        // On vérifie si chaque demi-arête a une arête précédente
        if (h->prev == NULL) {
            cout << "Erreur : La demi-arete " << i << " n'a pas de pointeur 'prev'." << endl;
            errors++;
        // On vérifie si prev est cohérent avec next
        } else if (h->prev->next != h) {
            cout << "Erreur : Le 'prev->next' de la demi-arete " << i << " ne revient pas sur elle-meme." << endl;
            errors++;
        }

        // On vérifie que chaque demi-arête a un twin
        if (h->twin == NULL) {
            cout << "Alerte (Bordure) : La demi-arete " << i << " n'a pas de twin." << endl;
        } else {
             // On vérifie si aller au twin, que le twin de ce twin est bien h
            if (h->twin->twin != h) {
                cout << "Erreur : Le twin du twin de la demi-arete " << i << " n'est pas elle-meme." << endl;
                errors++;
            }
            // On vérifie que les demi arêtes ne la même source
            if (h->source == h->twin->source) {
                cout << "Erreur : La demi-arete " << i << " et son twin ont le meme sommet source." << endl;
                errors++;
            }
        }

        // On vérifie que chaque demi-arête est associée à une face
        if (h->adjacent_face == NULL) {
            cout << "Erreur : La demi-arete " << i << " n'est liee a aucune face." << endl;
            errors++;
        }
    }



   
    if (errors == 0) {
        cout << "Fin des tests, tout est valide" << endl;
    } else {
        cout << "Fin des tests, " << errors << " erreurs detectees." << endl;
    }
    cout << "------------------------------------------------" << endl;
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


void myMesh::subdivisionCatmullClark()
{
    computeNormals();
}


/*
  Explication de cette partie :
  Cette fonction implémente la simplification du maillage par collapse d'arêtes.
  On cherche l'arête la plus courte du maillage, et on fusionne deux sommets de cette arête situé au milieu

*/
void myMesh::simplify()
{
    if (vertices.size() == 0 || halfedges.size() == 0) return;

    int gap_limit = vertices.size() * 0.30;
    int items_removed = 0;

    // Boucle de suppression
    while (items_removed < gap_limit && vertices.size() > 4) {
        
        myHalfedge* target_edge = NULL;
        double current_min_dist = std::numeric_limits<double>::max();

        // Boucle pour trouver l'arête la plus courte
        for (int idx = 0; idx < halfedges.size(); idx++) {
            myHalfedge* current_edge = halfedges[idx];
            
            // Si on est sur une arête de frontière
            if (current_edge == NULL || current_edge->source == NULL || 
                current_edge->twin == NULL || current_edge->twin->source == NULL) {
                continue;
            }

            // On calcule la longueur de l'arête
            myPoint3D* start_node = current_edge->source->point;
            myPoint3D* end_node = current_edge->twin->source->point;

            double x_offset = end_node->X - start_node->X;
            double y_offset = end_node->Y - start_node->Y;
            double z_offset = end_node->Z - start_node->Z;
            double edge_len = sqrt(x_offset * x_offset + y_offset * y_offset + z_offset * z_offset);

            // Si la longueur est plus courte que le minimum local
            if (edge_len < current_min_dist) {
                current_min_dist = edge_len;
                target_edge = current_edge;
            }
        }

        // Si on a trouvé une arête, on l'écrase
        if (target_edge != NULL) {
            collapseEdge(target_edge);
            items_removed = items_removed + 1;
        } else {
            break; 
        }
    }

    computeNormals();
}

/*
  Explication de cette partie : 

  Cette fonction implémente la notion du cours de collapse d'arête, fusionnant deux sommets et supprimant les faces dégénérées résultantes.
  On commence par trouver les éléments liés à l'arête à supprimer : les deux faces adjacentes, les deux sommets de l'arête, et les deux sommets opposés à l'arête dans les faces adjacentes.
  On calcule ensuite le milieu de l'arête et on déplace le sommet pivot vers ce milieu.
  Ensuite, on recoud les demi-arêtes pour fusionner les deux sommets, et on nettoie les données obsolètes (sommet supprimé, faces dégénérées, demi-arêtes supprimées).
*/
void myMesh::collapseEdge(myHalfedge* e)
{
    if (e == NULL) return;

    myHalfedge* e_opposite = e->twin;
    if (e_opposite == NULL) return; 

    myFace* target_face_1 = e->adjacent_face;
    myFace* target_face_2 = e_opposite->adjacent_face;

    myHalfedge* side_a1 = e->next;
    myHalfedge* side_b1 = e->next->next;
    myHalfedge* side_a2 = e_opposite->next;
    myHalfedge* side_b2 = e_opposite->next->next;

    myVertex* pivot_vertex = e->source;
    myVertex* delete_vertex = e_opposite->source;
    myVertex* apex_vertex_1 = side_b1->source;
    myVertex* apex_vertex_2 = side_b2->source;

    // Calcul du milieu de l'arête
    double calculated_x = (pivot_vertex->point->X + delete_vertex->point->X) / 2.0;
    double calculated_y = (pivot_vertex->point->Y + delete_vertex->point->Y) / 2.0;
    double calculated_z = (pivot_vertex->point->Z + delete_vertex->point->Z) / 2.0;

    // On déplace le sommet vers ce milieu
    pivot_vertex->point = new myPoint3D(calculated_x, calculated_y, calculated_z);

    // Recouture pour fusionner v2 dans v1
    for (int idx = 0; idx < halfedges.size(); idx++) {
        if (halfedges[idx]->source == delete_vertex) {
            halfedges[idx]->source = pivot_vertex;
        }
    }

    // Recouture des twins
    side_b1->twin->twin = side_a1->twin;
    side_a1->twin->twin = side_b1->twin;
    side_b2->twin->twin = side_a2->twin;
    side_a2->twin->twin = side_b2->twin;

    // Reset des originof
    pivot_vertex->originof = side_a2->twin;
    apex_vertex_1->originof  = side_a1->twin;
    apex_vertex_2->originof  = side_b1->twin;

    // Netoyage des données obsolètes 
    for (int idx = vertices.size() - 1; idx >= 0; idx--) {
        if (vertices[idx] == delete_vertex) {
            delete vertices[idx]->point;
            delete vertices[idx];
            vertices.erase(vertices.begin() + idx);
        }
    }

    for (int idx = faces.size() - 1; idx >= 0; idx--) {
        if (faces[idx] == target_face_1 || faces[idx] == target_face_2) {
            delete faces[idx];
            faces.erase(faces.begin() + idx);
        }
    }

    for (int idx = halfedges.size() - 1; idx >= 0; idx--) {
        myHalfedge* current_h = halfedges[idx];
        if (current_h == e || current_h == e_opposite || 
            current_h == side_a1 || current_h == side_a2 || 
            current_h == side_b1 || current_h == side_b2) {
            delete current_h;
            halfedges.erase(halfedges.begin() + idx);
        }
    }
}

void myMesh::simplify(myVertex *v)
{
 
}


void myMesh::linkHalfedgeTwins()
{
    // On utilise une map pour stocker les demi-arêtes en fonction de leur orientation (source -> destination)
    map<pair<myVertex*, myVertex*>, myHalfedge*> map;
    vector<myHalfedge*> allHalfedges;

    // On parcourt toutes les faces et on stocke leurs demi-arêtes dans la map
    for (myFace *f : faces) {
        myHalfedge *he0 = f->adjacent_halfedge;
        myHalfedge *he1 = he0->next;
        myHalfedge *he2 = he1->next;
        myHalfedge *he3 = he2->next;

       // On stocke les demi-arêtes dans une liste pour pouvoir les parcourir ensuite
        allHalfedges.push_back(he0);
        allHalfedges.push_back(he1);
        allHalfedges.push_back(he2);
        allHalfedges.push_back(he3);

        // On stocke les demi-arêtes dans la map avec leur orientation
        map[make_pair(he0->source, he1->source)] = he0;
        map[make_pair(he1->source, he2->source)] = he1;
        map[make_pair(he2->source, he3->source)] = he2;
        map[make_pair(he3->source, he0->source)] = he3;
    }

    // On parcourt toutes les demi-arêtes et on cherche leur twin dans la map
    for (myHalfedge *h : allHalfedges) {
        myVertex *vStart = h->source;
        myVertex *vEnd = h->next->source;
        
        // On cherche la demi-arête opposée (twin) dans la map
        auto it = map.find(make_pair(vEnd, vStart));
        
        if (it != map.end()) {
            h->twin = it->second;
        } else {
            h->twin = nullptr; // Bordure du maillage
        }
    }
}
/*
  Explication de cette partie : 

  Cette fonction construit une surface de révolution à partir d'un profil défini par une série de points.
  Les étapes sont les suivantes :
  - On définit un profil de points dans le plan XY (ici, un profil de vase).
  - On itère sur un nombre de tranches de révolution (slices) pour créer les sommets en appliquant une rotation autour de l'axe Y.
  - On itère à nouveau pour créer les faces entre les tranches en connectant les sommets correspondants.
  - On utilise la fonction linkHalfedgeTwins() pour connecter correctement les demi-arêtes entre elles.

*/
void myMesh::surfaceRevolution()
{
    vector<myPoint3D> profile;
   
    // profil d'un vase
    profile.clear();
    profile.push_back(myPoint3D(0.00, -0.50, 0.0)); 
    profile.push_back(myPoint3D(0.20, -0.50, 0.0));  
    profile.push_back(myPoint3D(0.25, -0.40, 0.0));  
    profile.push_back(myPoint3D(0.45, -0.10, 0.0));  
    profile.push_back(myPoint3D(0.35,  0.20, 0.0)); 
    profile.push_back(myPoint3D(0.15,  0.40, 0.0)); 
    profile.push_back(myPoint3D(0.22,  0.50, 0.0));  
    

    // On efface le rendu actuel
    clear();

    const int slices = 20;

    // Grille pour stocker les sommets 
    vector<vector<myVertex *>> grid(slices, vector<myVertex *>(profile.size(), nullptr));
    
    // On itère sur les tranches de révolution
    for (int s = 0; s < slices; s++)
    {
        double cosRes = cos((2*M_PI * static_cast<double>(s)) / static_cast<double>(slices));
        double sinRes = sin((2*M_PI * static_cast<double>(s)) / static_cast<double>(slices));
        // On itère sur les points du profil pour créer les sommets de la tranche
        for (int i = 0; i < profile.size(); i++)
        {
            const myPoint3D &p = profile[i];
            myVertex *v = new myVertex();
            
            // Placement du vertex en appliquant la rotation autour de l'axe Y
            v->point = new myPoint3D(cosRes * p.X + sinRes * p.Z, p.Y, -sinRes * p.X + cosRes * p.Z);
            v->originof = nullptr;
            
            vertices.push_back(v);
            grid[s][i] = v;
        }
    }

    // On itère à nouveau pour créer les faces entre les tranches
    for (int s = 0; s < slices; s++)
    {
        // Indice de la tranche suivante, avec un modulo pour boucler
        int k = (s + 1) % slices;
        // On itère sur les segments du profil pour créer les faces
        for (int i = 0; i < profile.size() - 1; i++)
        {
          // On récupère les 4 sommets formant la face entre les tranches s et k, et les segments i et i+1 du profil
            myVertex *v0 = grid[s][i];
            myVertex *v1 = grid[k][i];
            myVertex *v2 = grid[k][i + 1];
            myVertex *v3 = grid[s][i + 1];

            // Création de la face et des demi-arêtes
            myFace *f = new myFace();
            
            myHalfedge *e0 = new myHalfedge();
            myHalfedge *e1 = new myHalfedge();
            myHalfedge *e2 = new myHalfedge();
            myHalfedge *e3 = new myHalfedge();

            // On connecte la face à une de ses demi-arêtes
            f->adjacent_halfedge = e0;

             // Connexion des demi-arêtes à la face
            e0->adjacent_face = f;
            e1->adjacent_face = f;
            e2->adjacent_face = f;
            e3->adjacent_face = f;

            // Connexion des demi-arêtes à leurs sommets et à la face
            e0->source = v0;
            e1->source = v1;
            e2->source = v2;
            e3->source = v3;

            // Chainage next
            e0->next = e1;
            e1->next = e2;
            e2->next = e3;
            e3->next = e0;

            // Chainage prev
            e0->prev = e3;
            e1->prev = e0;
            e2->prev = e1;
            e3->prev = e2;

            // On assigne une demi-arête d'origine à chaque vertex
            v0->originof = e0;
            v1->originof = e1;
            v2->originof = e2;
            v3->originof = e3;

            
            // On stocke la face et les demi-arêtes dans le maillage
            faces.push_back(f);
            halfedges.push_back(e0);
            halfedges.push_back(e1);
            halfedges.push_back(e2);
            halfedges.push_back(e3);
        }
    }

    // Après avoir créé toutes les faces et demi-arêtes, on connecte les demi-arêtes entre elles pour former les twins
    linkHalfedgeTwins();

    // On calcule les normales pour un rendu correct
    computeNormals();
}


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
    // On compte le nombre de sommets de la face
    int vertexCount = 0;
    myHalfedge *loopPointer = face->adjacent_halfedge;
    do
    {
        vertexCount++;
        loopPointer = loopPointer->next;
    } while (loopPointer != face->adjacent_halfedge);

    if (vertexCount <= 3)
        return false; 

   

    // Liste dynamique des demi-arêtes encore actives dans le polygone (IA)
    std::vector<myHalfedge *> activeHalfedges(vertexCount);
    loopPointer = face->adjacent_halfedge;
    for (int i = 0; i < vertexCount; i++)
    {
        activeHalfedges[i] = loopPointer;
        loopPointer = loopPointer->next;
    }

    // méthode de newell explicitée par kyrian dans le chat (les recherches que j'ai réalisé : https://stackoverflow.com/questions/27326636/calculate-normal-vector-of-a-polygon-newells-method)
    // sans ça, (en utilisant computeNormal de myFace) j'avais des problèmes d'artifacts sur les triangles 
    myVector3D areaNormal(0, 0, 0);
    for (int i = 0; i < vertexCount; i++)
    {
        myPoint3D *a = activeHalfedges[i]->source->point;
        myPoint3D *b = activeHalfedges[(i + 1) % vertexCount]->source->point;
        
        areaNormal.dX += (a->Y - b->Y) * (a->Z + b->Z);
        areaNormal.dY += (a->Z - b->Z) * (a->X + b->X);
        areaNormal.dZ += (a->X - b->X) * (a->Y + b->Y);
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
            // Indices des demi-arêtes précédentes et suivantes dans la liste active (commme dans le readFile) (IA)
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
                    // Création des deux nouvelles demi-arêtes formant la diagonale
                    myHalfedge *diagonalA = new myHalfedge();
                    myHalfedge *diagonalB = new myHalfedge();

                    // Connexion des demi-arêtes de la diagonale avec les sommets de l'oreille
                    diagonalA->source = vNext;
                    diagonalB->source = vPrev;
                    diagonalA->twin = diagonalB;
                    diagonalB->twin = diagonalA;
                    halfedges.push_back(diagonalA);
                    halfedges.push_back(diagonalB);

                    // Création de la nouvelle face formée par l'oreille (le triangle isolé)
                    myFace *subFace = new myFace();
                    faces.push_back(subFace);
                    subFace->adjacent_halfedge = hePrev;

                    // On insère diagonalA entre l'arête qui précédait l'oreille et l'arête de l'oreille
                    myHalfedge *meshOuterPrev = hePrev->prev;
                    myHalfedge *meshOuterNext = heNext; 

                    // Mise à jour des connexions pour la nouvelle face
                    hePrev->next = heCurr;
                    heCurr->next = diagonalA;
                    diagonalA->next = hePrev;
                    
                    hePrev->prev = diagonalA;
                    heCurr->prev = hePrev;
                    diagonalA->prev = heCurr;
                  
                    hePrev->adjacent_face = subFace;
                    heCurr->adjacent_face = subFace;
                    diagonalA->adjacent_face = subFace;

                    // On insère diagonalB entre l'arête qui précédait l'oreille et l'arête qui la suivait
                    meshOuterPrev->next = diagonalB;
                    diagonalB->prev = meshOuterPrev;

                    diagonalB->next = meshOuterNext;
                    meshOuterNext->prev = diagonalB;

                    // Mise à jour des connexions pour la face originale
                    diagonalB->adjacent_face = face;
                    face->adjacent_halfedge = diagonalB;

                    // Mise à jour de notre tableau de suivi
                    activeHalfedges[prevIdx] = diagonalB;
                    activeHalfedges.erase(activeHalfedges.begin() + currentIdx);

                    // On réinitialise l'index
                    currentIdx = prevIdx % activeHalfedges.size();
                    earClipped = true;
                    break;
                }
            }

            currentIdx = (currentIdx + 1) % activeHalfedges.size();

        } while (currentIdx != startIdx);

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