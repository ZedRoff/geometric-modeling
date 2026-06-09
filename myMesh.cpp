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
    myline >> t;
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

    //  créer halfedges
    hedges = new myHalfedge*[faceids.size()];
    for (unsigned int i = 0; i < faceids.size(); i++)
        hedges[i] = new myHalfedge();

    //  connecter face
    f->adjacent_halfedge = hedges[0];

    //  boucle principale
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

        if (it != twin_map.end()) {
            hedges[i]->twin = it->second;
            it->second->twin = hedges[i];
        } else {
            twin_map[edge] = hedges[i];
        }

        // originof
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

void myMesh::computeNormals() { 
  for (unsigned int i = 0; i < faces.size(); i++)
    faces[i]->computeNormal();
  for (unsigned int i = 0; i < vertices.size(); i++)
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



bool myMesh::triangulate(myFace *f) {

    vector<myVertex*> verts;
    myHalfedge* start = f->adjacent_halfedge;
    myHalfedge* he = start;

    do {
        verts.push_back(he->source);
        he = he->next;
    } while (he != start);

    if (verts.size() == 3)
        return false;

    // Liste d'indices actifs
    vector<int> V(verts.size());
    for (int i = 0; i < verts.size(); i++)
        V[i] = i;

    // --- fonctions utilitaires ---
    auto cross = [](myVertex* a, myVertex* b, myVertex* c) {
        return (b->point->X - a->point->X) * (c->point->Y - a->point->Y)
               - (b->point->Y - a->point->Y) * (c->point->X - a->point->X);
    };

    auto isConvex = [&](myVertex* a, myVertex* b, myVertex* c) {
        return cross(a, b, c) > 0; // CCW
    };

    auto pointInTri = [&](myVertex* p, myVertex* a, myVertex* b, myVertex* c) {
        double c1 = cross(p, a, b);
        double c2 = cross(p, b, c);
        double c3 = cross(p, c, a);
        return (c1 >= 0 && c2 >= 0 && c3 >= 0) ||
               (c1 <= 0 && c2 <= 0 && c3 <= 0);
    };

    // --- ear clipping ---
    while (V.size() > 3) {
        bool earFound = false;

        for (size_t i = 0; i < V.size(); i++) {

            int i0 = V[(i + V.size() - 1) % V.size()];
            int i1 = V[i];
            int i2 = V[(i + 1) % V.size()];

            myVertex* v0 = verts[i0];
            myVertex* v1 = verts[i1];
            myVertex* v2 = verts[i2];

            if (!isConvex(v0, v1, v2))
                continue;

            bool contains = false;
            for (int j : V) {
                if (j == i0 || j == i1 || j == i2)
                    continue;

                if (pointInTri(verts[j], v0, v1, v2)) {
                    contains = true;
                    break;
                }
            }

            if (contains)
                continue;

            // 🎯 EAR TROUVÉ → créer triangle
            myFace* newFace = new myFace();

            myHalfedge* he0 = new myHalfedge();
            myHalfedge* he1 = new myHalfedge();
            myHalfedge* he2 = new myHalfedge();

            he0->source = v0;
            he1->source = v1;
            he2->source = v2;

            he0->next = he1;
            he1->next = he2;
            he2->next = he0;

            he0->prev = he2;
            he1->prev = he0;
            he2->prev = he1;

            he0->adjacent_face = newFace;
            he1->adjacent_face = newFace;
            he2->adjacent_face = newFace;

            newFace->adjacent_halfedge = he0;

            halfedges.push_back(he0);
            halfedges.push_back(he1);
            halfedges.push_back(he2);
            faces.push_back(newFace);

            if (!v0->originof) v0->originof = he0;
            if (!v1->originof) v1->originof = he1;
            if (!v2->originof) v2->originof = he2;

            // ❌ supprimer l’oreille
            V.erase(V.begin() + i);

            earFound = true;
            break;
        }

        if (!earFound) {
            // polygone invalide
            return false;
        }
    }

    // 🔺 dernier triangle
    myFace* newFace = new myFace();

    myHalfedge* he0 = new myHalfedge();
    myHalfedge* he1 = new myHalfedge();
    myHalfedge* he2 = new myHalfedge();

    myVertex* v0 = verts[V[0]];
    myVertex* v1 = verts[V[1]];
    myVertex* v2 = verts[V[2]];

    he0->source = v0;
    he1->source = v1;
    he2->source = v2;

    he0->next = he1;
    he1->next = he2;
    he2->next = he0;

    he0->prev = he2;
    he1->prev = he0;
    he2->prev = he1;

    he0->adjacent_face = newFace;
    he1->adjacent_face = newFace;
    he2->adjacent_face = newFace;

    newFace->adjacent_halfedge = he0;

    halfedges.push_back(he0);
    halfedges.push_back(he1);
    halfedges.push_back(he2);
    faces.push_back(newFace);

    return true;
}

void myMesh::triangulate() {
    std::vector<myFace*> facesCopy = faces; 
    for (myFace* f : facesCopy) {
        triangulate(f);
    }
}