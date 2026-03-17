#include<string>
#include<iostream>
#include<fstream>
#include<sys/inotify.h>
#include<unordered_map>
#include<sstream>
#include<vector>
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
std::ifstream inW("/home/nullora/novusdeploy/.ndeploy/watched_files.nd");
std::string lineW;
std::string tag, src, destsStr;
void saveFiles();
void saveAutoFiles();
int main(int argc, char* argv[]){
    //load from watch file into manFiles.
    while(std::getline(inW,lineW)){
        if(!lineW.empty()){
            entry e;
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
    std::string tagC = argv[3];
    std::string flag = "EMP";
    if(argc==5) flag = argv[4];
    if(cmd=="add"){
        entry e;
        e.src = filepath;
        manFiles[tagC] = e;
        if(flag=="--auto")saveAutoFiles();
        else{ saveFiles();}
    }
    if(cmd=="add-d"){
    manFiles[tagC].dests.push_back(filepath);
        if(flag=="--auto"){
            // append just this one entry to auto_watch.nd
            std::ofstream out(".ndeploy/auto_watch.nd", std::ios::app);
            entry& e = manFiles[tagC];
            out << tagC << " " << e.src << " ";
            for(int i = 0; i < e.dests.size(); i++){
                    out << e.dests[i];
                    if(i < e.dests.size()-1) out << ",";
                }
            out << "\n";
        }
        saveFiles();
    }
    if(cmd=="deploy"){
        if(manFiles.find(filepath)==manFiles.end()){
            std::cout<<"tag not found\n";
            return 1;
        }
        entry& e = manFiles[filepath];
        for(auto& dest : e.dests){
            std::string cpCmd = "cp " + e.src + " " + dest;
            system(cpCmd.c_str());
        }
        std::cout<<"deployed " << filepath << " to " << e.dests.size() << " destinations\n";
    }
}
void saveFiles(){
    std::ofstream out("/home/nullora/novusdeploy/.ndeploy/watched_files.nd", std::ios::trunc);
    for(auto& [tag, e] : manFiles){
        out << tag << " " << e.src << " ";
        for(int i = 0; i < e.dests.size(); i++){
            out << e.dests[i];
            if(i < e.dests.size()-1) out << ",";
        }
        out << "\n";
    }
}
void saveAutoFiles(){
    std::ofstream out("/home/nullora/novusdeploy/.ndeploy/auto_watch.nd", std::ios::app);
    for(auto& [tag, e] : autoFiles){
        out << tag << " " << e.src << " ";
        for(int i = 0; i < e.dests.size(); i++){
            out << e.dests[i];
            if(i < e.dests.size()-1) out << ",";
        }
        out << "\n";
    }
}