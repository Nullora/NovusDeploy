#include<string>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include<sstream>
#include<vector>
#include <unistd.h>
struct entry{
    std::string src;
    std::vector<std::string> dests;
};
struct tag_group{
    std::vector<std::string> tags;
};

//manual files (updated with ndep update "tag")
std::unordered_map<std::string,entry> manFiles;
std::unordered_map<std::string,tag_group> TagGroups;
//watch file
std::ifstream inW("/home/nullora/ndep/.ndeploy/watched_files.nd");
std::string lineW;
void saveFiles();

bool deploy(std::string tag){
    if(manFiles.find(tag)==manFiles.end()){
        std::cout<<"[--] tag not found..\n";
        return false;
    }
    entry& e = manFiles[tag];
    for(auto& dest : e.dests){
        std::string cpCmd = "cp " + e.src + " " + dest;
        system(cpCmd.c_str());
    }
    std::cout<<"[++] deployed " << tag << " to " << e.dests.size() << " destination(s)\n";
    return true;
}


int main(int argc, char* argv[]){
    //load from watch file into manFiles.
    setuid(0);
    setgid(0);
    while(std::getline(inW,lineW)){
        if(!lineW.empty()){
            // handle tags
            if(lineW.starts_with("T:")){
                lineW = lineW.substr(2);
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
            }else{
                // handle tag groups
                if(lineW.starts_with("G:")){
                    lineW = lineW.substr(2);
                    tag_group t;
                    std::string groupId, tagsStr, tag;
                    std::istringstream ss(lineW);
                    ss >> groupId >> tagsStr;
                    std::istringstream tss(tagsStr);
                    while(std::getline(tss,tag,',')){
                        t.tags.push_back(tag);
                    }
                    TagGroups[groupId] = t;
                    
                }
            }
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
    if(cmd=="addg"){
        bool in = true;
        tag_group t;
        std::string grp,tag;
        grp = filepath;
        while(true){
            std::cout<<"Tag to add: ";
            std::getline(std::cin,tag);
            if(tag=="hlt"){
                saveFiles();
                return 0;
            }
            t.tags.push_back(tag);
            TagGroups[grp] = t;
        }
    }
    if(cmd=="dep"){
        deploy(filepath);
    }
    if(cmd=="depg"){
        if(TagGroups.find(filepath)==TagGroups.end()){
            std::cout<<"[--] group not found..\n";
            return 1;
        }
        tag_group& e = TagGroups[filepath];
        for(auto& t : e.tags){
            if(!deploy(t)){std::cout<<"not deployed\n"; return 1;}
        }
        std::cout << "deploy group " << filepath << " successfully \n";
    }
}
void saveFiles(){
    std::ofstream out("/home/nullora/ndep/.ndeploy/watched_files.nd", std::ios::trunc);
    for(auto& [tag, e] : manFiles){
        out<<"T:" << tag << " " << e.src << " ";
        for(int i = 0; i < e.dests.size(); i++){
            out << e.dests[i];
            if(i < e.dests.size()-1) out << ",";
        }
        out << "\n";
    }
    for(auto& [gid, t] : TagGroups){
        out << "G:"<< gid << " ";
        for(int i = 0; i < t.tags.size(); i++){
            out << t.tags[i];
            if(i < t.tags.size()-1) out << ",";
        }
        out << "\n";
    }
}