
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//  Read .obj file                                                      //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "models.h"
#include "obj_reader.h"

Vertex* ObjReader::read_vertex(std::string &line) {
    std::stringstream ss(line);

    std::string val;
    std::vector<std::string> vals;
    while(std::getline(ss, val, ' ')) {
        vals.push_back(val);
    }

    if (vals.size() != 4) {
        std::cerr << "Invalid .obj file " << line << std::endl;
        return nullptr;
    }

    // new Vertex
    Vertex *vertex = new Vertex(
        std::stof(vals[1]),
        std::stof(vals[2]) - 0.1,
        std::stof(vals[3])
    );

    return vertex;
}

std::tuple<int, int, int> ObjReader::read_face(std::string &line) {
    std::stringstream ss(line);

    std::string val;
    std::vector<std::string> vals;
    while(std::getline(ss, val, ' ')) {
        vals.push_back(val);
    }

    if (vals.size() != 4) {
        std::cerr << "Invalid .obj file " << line << std::endl;
        return {-1, -1, -1};
    }

    return {
        std::stoi(vals[1]),
        std::stoi(vals[2]),
        std::stoi(vals[3])
    };
}

int ObjReader::read_obj() {
    std::ifstream input_file(filename);
    if (input_file.is_open() == false) {
        std::cerr << filename << " doesn't exist!!" << std::endl;
        return 1;
    }
    
    std::vector<Vertex*> vertices;

    std::string line;
    while (getline(input_file, line)) {
        if (line.size() == 0) continue;

        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == 'v') {
            Vertex *vertex = read_vertex(line);
            vertices.push_back(vertex);
        }
        else if (line[0] == 'f') {
            auto [a, b, c] = read_face(line);

            model->add_face(
                vertices[a - 1],
                vertices[b - 1],
                vertices[c - 1],
                model->AddMode::Init
            );
        }
        else {
            std::cerr << "Invalid .obj file" << std::endl;
            return 2;
        }
    }

    model->snapshots.push_back(model->faces);

    std::cout << "Reading .obj finished." << std::endl;

    return 0;
}


