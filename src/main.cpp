#include <GL/freeglut.h>
#include <string>

#include "models.h"
#include "obj_reader.h"
#include "display.h"

Model* model;

// parameters
int HEIGHT = 640;         // 画面サイズ-縦
int WIDTH = 640;           // 画面サイズ-横
int MIN_FACES = 50;       // 最小の面数
int STEP_SIZE = 100;       // ステップ分割数
std::string filename = "./obj/bunny.obj";

int main(int argc, char** argv) {
    // new Model
    model = new Model(MIN_FACES, STEP_SIZE);

    // read .obj
    ObjReader obj_reader(model, filename);
    if (obj_reader.read_obj()) {
        return 1;
    }

    // Execute QEM
    model->QEM();

    // display
    display_start(argc, argv);
    
    return 0;
}