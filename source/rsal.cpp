#include "../include/rsal.hpp"
#include <pthread.h>
#include <string.h>

void* RAWPTHREADRSALplayAudio(void* ap){
    RSAL::audio* a = (RSAL::audio*)ap;
    if (a->getFile().size() > 0){ 
        a->done=false; std::string cmd= "source/deps/ffplay -nodisp -autoexit  -hide_banner "+a->getFile();
        system(cmd.data()); 
    } a->done=true;

    return NULL;
}

std::string RSAL::audio::getFile(){return File;}

bool RSAL::audio::isPlaying(){return !done;}

void RSAL::audio::play(std::string file){
    File=file; done=false;
    pthread_create(&t,NULL,RAWPTHREADRSALplayAudio,this);
}   

void RSAL::audio::play(){
    if (File.size() > 0){
        done=false;
        pthread_create(&t,NULL,RAWPTHREADRSALplayAudio,this); 
    }
}   

void RSAL::audio::stop(){
    pthread_cancel(t); done=true;
}   