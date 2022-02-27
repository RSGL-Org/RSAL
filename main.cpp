#include "RSAL.hpp"
#include <RSGL/RSGL.hpp>

//RSGL::window win("music",{500,500,500,500},{255,255,255});

/*

namespace RSGL{
using namespace RSAL;
};


*/

int main(){
    RSAL::audio a("6969.mp3");
    a.play(); float i=0;
    while (a.isPlaying()){
        //win.checkEvents();
        i+=0.01;
        //if (win.event.type == RSGL::KeyPressed) a.stop();
        //win.clear();
    } //win.close();