
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//  Read .obj file                                                      //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include "models.h"

class ObjReader {
private:
    Vertex* read_vertex(std::string &line);
    std::tuple<int, int, int> read_face(std::string &line);

public:
    Model*      model;
    std::string filename;
    ObjReader(Model* _model, std::string _filename) {
        model = _model;
        filename = _filename;
    }

    int read_obj(void);
};

