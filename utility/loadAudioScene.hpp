#ifndef LOAD_AUDIO_SCENE_HPP
#define LOAD_AUDIO_SCENE_HPP
#include <sys/types.h>
#include <dirent.h>
#include<stdlib.h>
#include "al/app/al_App.hpp"

class AudioLoader {
public:
void loadSceneAudio(std::vector<std::vector<std::string>>& storageVector, int sceneNumber) {
   std::string path = "/Users/lucian/Desktop/201B/allolib_playground/softlight-sphere/audioFolder-" + std::to_string(sceneNumber);
    //method using posix, this c++ version is too old for anything else
    DIR *dir;
    struct dirent *entry;
    dir = opendir(path.c_str());
    if (dir == nullptr) { //if the directory doesn't exist//
        std::cerr << "Error: Directory not found.\n";
        //return 1;
    }

    while ((entry = readdir(dir)) != nullptr) { //if directory exists//
        std::string fileName = entry->d_name;
        if (fileName != "." && fileName != "..") { // Skip . and ..
            //std::cout << fileName << std::endl;
            storageVector[sceneNumber-1].push_back(path + "/" + fileName);
            std::cout << fileName << std::endl;


        }
    }
    closedir(dir);
    //std::cout << song1files[0] << song1files[1] << song1files[2] << std::endl;
    std::cout << storageVector[0][0].c_str() << std::endl;

}
};





#endif