#include<string>
#include<iostream>
#include<fstream>
#include<sys/inotify.h>
#include<unordered_map>
#include<sstream>
#include<vector>
struct entry{
    std::vector<std::string> srcs;
    std::vector<std::string> dests;
};
int fd = inotify_init1(IN_CLOEXEC);
std::unordered_map<int,entry> autoFiles;
std::unordered_map<std::string,entry> manFiles;
std::ifstream inW("/home/nullora/novusdeploy/.ndeploy/watched_files.nd");
std::string lineW;
void saveFiles();
int main(int argc, char* argv[]){
    while(std::getline(inW,lineW)){
        if(!lineW.empty()){
            entry e;
            std::istringstream ss(lineW);
            std::string tag, srcsStr, destsStr;
            ss >> tag >> srcsStr >> destsStr;
            std::istringstream sss(srcsStr);
            std::string s;
            while(std::getline(sss,s,',')) e.srcs.push_back(s);
            std::istringstream dss(destsStr);
            std::string d;
            while(std::getline(dss,d,',')) e.dests.push_back(d);
            manFiles[tag] = e;
        }
    }
    if(argc < 3){ std::cout << "not enough args\n"; return 1; }
    std::string cmd = argv[1];
    std::string filepath = argv[2];
    std::string tagC = (argc >= 4) ? argv[3] : "";
    if(cmd=="add"){
        manFiles[tagC].srcs.push_back(filepath);
        saveFiles();
    }
    if(cmd=="add-d"){
        manFiles[tagC].dests.push_back(filepath);
        saveFiles();
    }
    if(cmd=="deploy"){
        if(manFiles.find(filepath)==manFiles.end()){
            std::cout<<"tag not found\n";
            return 1;
        }
        entry& e = manFiles[filepath];
        for(int i = 0; i < e.srcs.size(); i++){
            std::string cpCmd = "cp " + e.srcs[i] + " " + e.dests[i];
            system(cpCmd.c_str());
        }
        std::cout<<"deployed " << filepath << " to " << e.dests.size() << " destinations\n";
    }
}
void saveFiles(){
    std::ofstream out("/home/nullora/novusdeploy/.ndeploy/watched_files.nd", std::ios::trunc);
    for(auto& [tag, e] : manFiles){
        out << tag << " ";
        for(int i = 0; i < e.srcs.size(); i++){
            out << e.srcs[i];
            if(i < e.srcs.size()-1) out << ",";
        }
        out << " ";
        for(int i = 0; i < e.dests.size(); i++){
            out << e.dests[i];
            if(i < e.dests.size()-1) out << ",";
        }
        out << "\n";
    }
}