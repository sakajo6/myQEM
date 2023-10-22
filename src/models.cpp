
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//  Half Edge structure                                                 //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <queue>
#include <unordered_set>
#include <Eigen/Dense>

#include "models.h"

// Vertex
Vertex::Vertex(double x, double y, double z) {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = 1.0;
        half_edge = nullptr;
}



// HalfEdge
HalfEdge::HalfEdge(Vertex *v) {
    vertex = v;
    pair = nullptr;

    if (v->half_edge == nullptr) {
        v->half_edge = this;
    }
}



// Edge
Edge::Edge(HalfEdge *_half_edge, double _delta, Vertex *_v_bar) {
    half_edge = _half_edge;
    delta = _delta;
    v_bar = _v_bar;
}
bool operator>(const Edge &e1, const Edge &e2) {
    return e1.delta > e2.delta;
}



// Face
Face::Face(HalfEdge *_half_edge) {
    half_edge = _half_edge;
    calc_normal();
}

void Face::calc_normal() {
    // Calculate normal
    HalfEdge *curr_half_edge = half_edge;
    Vertex *v0 = curr_half_edge->vertex;

    curr_half_edge = curr_half_edge->next;
    Vertex *v1 = curr_half_edge->vertex;

    curr_half_edge = curr_half_edge->next;
    Vertex *v2 = curr_half_edge->vertex;


    Eigen::Vector4d v0_v1 = v1->v - v0->v;
    Eigen::Vector4d v0_v2 = v2->v - v0->v;

    normal = Eigen::Vector3d(
        v0_v1[1]*v0_v2[2] - v0_v1[2]*v0_v2[1],
        v0_v1[2]*v0_v2[0] - v0_v1[0]*v0_v2[2],
        v0_v1[0]*v0_v2[1] - v0_v1[1]*v0_v2[0]
    );

    // Normalize
    double length = 0.0;
    for (int i = 0; i < 3; i++) {
        length += normal[i]*normal[i];
    }
    length = std::sqrt(length);

    normal /= length;
}



// Model
void Model::set_halfedge_pair(HalfEdge *half_edge) {
    for (auto face: faces) {
        HalfEdge *curr_half_edge = face->half_edge;

        // Check half_edges of the face
        do {
            // Half_edge of the same edge
            if (half_edge->vertex == curr_half_edge->next->vertex
                    && half_edge->next->vertex == curr_half_edge->vertex) {
                
                // Verify that the found half_edge doesn't have pair
                half_edge->pair = curr_half_edge;
                curr_half_edge->pair = half_edge;


                return;
            }

            // Check the next half_edge
            curr_half_edge = curr_half_edge->next;
        } while (curr_half_edge != face->half_edge);
    }
        
}

void Model::add_face(Vertex *v0, Vertex *v1, Vertex *v2, AddMode add_mode) {
    HalfEdge *he0 = new HalfEdge(v0);
    HalfEdge *he1 = new HalfEdge(v1);        
    HalfEdge *he2 = new HalfEdge(v2);

    half_edges.insert(he0);
    half_edges.insert(he1);
    half_edges.insert(he2);

    he0->next = he1;    he0->prev = he2;
    he1->next = he2;    he1->prev = he0;
    he2->next = he0;    he2->prev = he1;

    Face *face = new Face(he0);
    faces.insert(face);

    he0->face = face;
    he1->face = face;
    he2->face = face;

    set_halfedge_pair(he0);
    set_halfedge_pair(he1);
    set_halfedge_pair(he2);

    if (add_mode == AddMode::Add) {
        set_contraction_info(he0);
        set_contraction_info(he1);
        set_contraction_info(he2);
    }
}

void Model::remove_face(Face *face) {
    faces.erase(face);

    HalfEdge *half_edge = face->half_edge;  // half_edge on the face
    for (int i = 0; i < 3; i++) {
        half_edges.erase(half_edge);

        if (half_edge->pair != nullptr) {
            half_edge->pair->pair = nullptr;
            half_edge->pair = nullptr;
        }

        // Next half_edge
        half_edge = half_edge->next;
    }        
}

bool Model::edge_exists(Edge *edge) {
    HalfEdge *half_edge = edge->half_edge;
    if (half_edges.count(half_edge)) {
        return true;
    }
    else {
        return false;
    }
}

void Model::edge_collapse(Edge *edge) {
    // Collect unnecessary faces.
    HalfEdge *half_edge = edge->half_edge;      // deleted edges

    // Removed objects
    std::unordered_set<Vertex*> removed_vertices = {
        half_edge->vertex,
        half_edge->next->vertex
    };
    std::unordered_set<Face*> removed_faces;     // adjacent faces

    // For half_edge and half_edge->pair
    std::unordered_set<Vertex*> left_adjacent_vertices;     // vertices adjacent to left removed areas
    std::unordered_set<Vertex*> right_adjacent_vertices;    // vertices adjacent to right removed areas
    for (int i = 0; i < 2; i++) {
        HalfEdge *curr_half_edge = half_edge;   // adjacent edge
        do {
            removed_faces.insert(curr_half_edge->face);

            // For check of "Pipe collapsion"
            if (i == 0) left_adjacent_vertices.insert(curr_half_edge->next->vertex);
            else right_adjacent_vertices.insert(curr_half_edge->next->vertex);

            // Next edge
            curr_half_edge = curr_half_edge->pair;
            if (curr_half_edge == nullptr) {
                break;
            }
            curr_half_edge = curr_half_edge->next;
        } while(curr_half_edge != half_edge);

        curr_half_edge = half_edge->prev;             // adjacent edge
        do {
            removed_faces.insert(curr_half_edge->face);

            // For check of "Pipe collapsion"
            if (i == 0) left_adjacent_vertices.insert(curr_half_edge->vertex);
            else right_adjacent_vertices.insert(curr_half_edge->vertex);
            
            // Prev edge
            curr_half_edge = curr_half_edge->pair;
            if (curr_half_edge == nullptr) {
                break;
            }
            curr_half_edge = curr_half_edge->prev;
        } while(curr_half_edge != half_edge->prev);

        // Another face
        if (half_edge->pair == nullptr) break;
        half_edge = half_edge->pair;
    }

    // Check of "Pipe Collapsion"
    // delete common vertices
    for (auto v: removed_vertices) {
        left_adjacent_vertices.erase(v);
        right_adjacent_vertices.erase(v);
    }
    left_adjacent_vertices.erase(half_edge->prev->vertex);
    right_adjacent_vertices.erase(half_edge->prev->vertex);
    if (half_edge->pair != nullptr) {
        left_adjacent_vertices.erase(half_edge->pair->prev->vertex);
        right_adjacent_vertices.erase(half_edge->pair->prev->vertex);
    }
    // detect "Pipe Collapsion"
    for (auto v: left_adjacent_vertices) {
        if (right_adjacent_vertices.count(v)) {
            // Detected "Pipe Collapsion"
            return;
        }
    }

    // Erase unnecessary faces
    for (Face *face: removed_faces) {
        remove_face(face);
    }

    // Add new faces
    for (Face *face: removed_faces) {
        int removed_vertices_cnt = 0;

        // Check all half_edge on the face.
        HalfEdge *curr_half_edge = face->half_edge;     // half_edge on the face
        for(int i = 0; i < 3; i++) {
            // If the vertex will be removed, count up.
            removed_vertices_cnt += removed_vertices.count(curr_half_edge->vertex);

            // Next edge
            curr_half_edge = curr_half_edge->next;
        }
        if (removed_vertices_cnt == 2) continue;

        // The face is not adjacent to removed half_edge.
        std::vector<Vertex*> new_vertices;
        curr_half_edge = face->half_edge;  // half_edge on the face

        for (int i = 0; i < 3; i++) {
            // Change the removed vertex to v_bar
            if (removed_vertices.count(curr_half_edge->vertex)) {
                new_vertices.push_back(edge->v_bar);
            }
            // Use the used vertex of the removed face.
            else {
                new_vertices.push_back(curr_half_edge->vertex);
            }

            // Next edge
            curr_half_edge = curr_half_edge->next;
        }

        add_face(new_vertices[0], new_vertices[1], new_vertices[2], AddMode::Add);
    }
}

// Set Q_bar, v_bar, delta to half_edge.
// Push half_edge to pq.
void Model::set_contraction_info(HalfEdge *half_edge) {
    // Calculate Q_bar, v_bar, delta.
    Eigen::Matrix4d Q_bar = half_edge->vertex->Q + half_edge->next->vertex->Q;

    // Edit Q_bar for finding minimum
    Eigen::Matrix4d Q_bar_ = Q_bar;
    for (int i = 0; i < 3; i++) {
        Q_bar_(3, i) = 0.0;
    }
    Q_bar_(3, 3) = 1.0;
    Eigen::Vector4d v_bar_coordinates = Q_bar_.colPivHouseholderQr().solve(Eigen::Vector4d(0.0, 0.0, 0.0, 1.0));

    double delta = v_bar_coordinates.transpose() * Q_bar * v_bar_coordinates;

    // New Vertex.
    Vertex *v_bar = new Vertex(
        v_bar_coordinates[0],
        v_bar_coordinates[1],
        v_bar_coordinates[2]
    );
    v_bar->Q = Q_bar;

    // Push edge to pq.
    Edge edge = Edge(
        half_edge,
        delta,
        v_bar
    );
    pq.push(edge);
}

void Model::initialize_Q(void) {
    // For each face.
    for (auto face: faces) {
        HalfEdge *half_edge = face->half_edge;

        // Calculate the normal of the face
        Eigen::Vector3d normal = face->normal;

        // Calculate p, K_p
        double d = 0.0;     // d = - a*x - b*y - c*z
        for (int i = 0; i < 3; i++) {
            d -= normal[i]*half_edge->vertex->v[i];
        }
        Eigen::Vector4d p {
            normal[0], normal[1], normal[2], d
        };
        Eigen::Matrix4d K_p = p * p.transpose();

        // Update Q for each vertex
        for (int i = 0; i < 3; i++) {
            half_edge->vertex->Q += K_p;

            // Next edge
            half_edge = half_edge->next;
        }
    }
}

void Model::initialize_pq(void) {
    // For each edge
    for (auto face: faces) {
        HalfEdge *half_edge = face->half_edge;

        /*
            TODO: 本来は距離t以内の2点間についてもcontraction. まずは辺のみで実装.
        */

        // For each edge
        for (int i = 0; i < 3; i++) {
            // Set Q_bar, v_bar, delta to half_edge.
            // Push half_edge to pq.
            set_contraction_info(half_edge);

            // Next edge
            half_edge = half_edge->next;
        }
    }
}

void Model::QEM(void) {
    // Initialization
    initialize_Q();
    initialize_pq();

    std::cout << "QEM initialization finished." << std::endl;

    // Contract half_edges until the face_num reaches MIN_FACES
    int face_num = faces.size();

    int prev_face_num = face_num;
    while (faces.size() > MIN_FACES && pq.size()) {
        // Fetch the edge with minimum cost.
        // Verify that the edge is actually contained the polygon at the time.
        while (pq.size()) {
            Edge edge = pq.top();
            pq.pop();

            if (!edge_exists(&edge)) {
                continue;
            }

            // Contract the edge.
            edge_collapse(&edge);
            break;
        }
        
        if ((int)faces.size() <= prev_face_num - STEP_SIZE) {
            std::cout << "Snapshot: " << faces.size() << std::endl;

            prev_face_num = faces.size();
            snapshots.push_back(faces);
        }
    }
    std::cout << "Snapshot: " << faces.size() << std::endl;
    snapshots.push_back(faces);

    std::cout << "QEM finished" << std::endl;
}
