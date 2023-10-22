
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//  Half Edge structure                                                 //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <queue>
#include <unordered_set>
#include <Eigen/Dense>

class Vertex;
class HalfEdge;
class Edge;
class Face;
class Model;

class Vertex {
public:
    Eigen::Vector4d     v;    // 座標
    HalfEdge            *half_edge;     // この頂点を視点にもつハーフエッジの一つ(要NULL処理)

    // for QEM
    Eigen::Matrix4d     Q;              // 行列Q

    Vertex(double x, double y, double z);
};

// Face作成時, pqにpush時にメモリ確保
class HalfEdge {
public:
    Vertex      *vertex;    // 始点となる頂点(not nullptr)
    Face        *face;      // このハーフエッジを含む面(not nullptr)
    HalfEdge    *pair;      // 稜線を挟んで反対側のハーフエッジ(要NULL処理)
    HalfEdge    *next;      // 次のハーフエッジ(not nullptr)
    HalfEdge    *prev;      // 前のハーフエッジ(not nullptr)

    HalfEdge(Vertex *v);
};

class Edge  {
public:
    HalfEdge    *half_edge;     // edgeに含まれるhalf_edgeの一つへのポインタ(not nullptr)

    // for QEM
    double      delta;          // cost value
    Vertex      *v_bar;         // edgeをcontracした後の頂点(not nullptr)

    Edge(HalfEdge *_half_edge, double _delta, Vertex *_v_bar);
};

class Face {
public:
    HalfEdge            *half_edge; // この面を含むハーフエッジの一つ
    Eigen::Vector3d     normal;     // 法線   

    Face(HalfEdge *_half_edge);

    void calc_normal();
};


class Model {
public:
    std::vector<std::unordered_set<Face*>>   snapshots;
    std::unordered_set<Face*>                faces;
    std::unordered_set<HalfEdge*>   half_edges;

    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;

    int MIN_FACES;
    int STEP_SIZE;
    Model(int _MIN_FACES, int _STEP_SIZE) {
        MIN_FACES = _MIN_FACES;
        STEP_SIZE = _STEP_SIZE;
    }

    void set_halfedge_pair(HalfEdge* half_edge);

    enum class AddMode {
        Init, Add
    };
    void add_face(Vertex*, Vertex*, Vertex*, AddMode);
    void remove_face(Face*);
    
    bool edge_exists(Edge *edge);
    void edge_collapse(Edge *edge);

    // Set Q_bar, v_bar, delta to half_edge.
    // Push half_edge to pq.
    void set_contraction_info(HalfEdge *half_edge);
    
    void initialize_Q(void);
    void initialize_pq(void);
    void QEM(void);
};
