#include<string>
#include<iostream>
#include<fstream>
#include<sys/inotify.h>
#include<unordered_map>
#include<sstream>
#include<vector>
#include <unistd.h>
struct entry{
    std::string src;
    std::vector<std::string> dests;
};
//init inotify for auto pushes (future update)
int fd = inotify_init1(IN_CLOEXEC);
std::unordered_map<int,entry> autoFiles;
//manual files (updated with ndep update "tag")
std::unordered_map<std::string,entry> manFiles;
//watch file
std::ifstream inW("/home/nullora/ndep/.ndeploy/watched_files.nd");
std::string lineW;
void saveFiles();
int main(int argc, char* argv[]){
    //load from watch file into manFiles.
    setuid(0);
    setgid(0);
    while(std::getline(inW,lineW)){
        if(!lineW.empty()){
            entry e;
            std::string tag, src, destsStr;
            std::istringstream ss(lineW);
            std::string dest;
            ss >> tag >> src >> destsStr;
            std::istringstream dss(destsStr);
            e.src = src;
            while(std::getline(dss, dest, ',')){
                e.dests.push_back(dest);
            }
            manFiles[tag] = e;
        }
    }
    std::string cmd = argv[1];
    std::string filepath = argv[2];
    std::string tagC;
    if(argc==4) tagC = argv[3];
    if(cmd=="add"){
        entry e;
        e.src = filepath;
        manFiles[tagC] = e;
        saveFiles();
    }
    if(cmd=="add-d"){
        manFiles[tagC].dests.push_back(filepath);
        saveFiles();
    }
    if(cmd=="dep"){
        if(manFiles.find(filepath)==manFiles.end()){
            std::cout<<"[--] tag not found\n";
            return 1;
        }
        entry& e = manFiles[filepath];
        for(auto& dest : e.dests){
            std::string cpCmd = "cp " + e.src + " " + dest;
            system(cpCmd.c_str());
        }
        std::cout<<"[++] deployed " << filepath << " to " << e.dests.size() << " destination(s)\n";
    }
}
void saveFiles(){
    std::ofstream out("/home/nullora/ndep/.ndeploy/watched_files.nd", std::ios::trunc);
    for(auto& [tag, e] : manFiles){
        out << tag << " " << e.src << " ";
        for(int i = 0; i < e.dests.size(); i++){
            out << e.dests[i];
            if(i < e.dests.size()-1) out << ",";
        }
        out << "\n";
    }
}