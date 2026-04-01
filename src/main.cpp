#include<string>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include<sstream>
#include<vector>
#include <unistd.h>
#include <filesystem>

std::string home;
struct entry{
    std::filesystem::path src;
    std::vector<std::string> dests;
};
struct tag_group{
    std::vector<std::string> tags;
};

//manual files (updated with ndep update "tag")
std::unordered_map<std::string,entry> manFiles;
std::unordered_map<std::string,tag_group> TagGroups;
//watch file
std::string watchfile;
std::ifstream inW;
std::string lineW;
void saveFiles();
//& deploy
bool deploy(std::string tag){
    if(manFiles.find(tag)==manFiles.end()){
        std::cout<<"[--] tag not found..\n";
        return false;
    }
    entry& e = manFiles[tag];
    for(auto& dest : e.dests){
        std::string cpCmd = "cp " + e.src.string() + " " + dest;
        system(cpCmd.c_str());
    }
    std::cout<<"[++] deployed " << tag << " to " << e.dests.size() << " destination(s)\n";
    return true;
}
//!
//& set
bool set(std::string tag){
    if(manFiles.find(tag)==manFiles.end()){
        std::cout<<"[--] tag not found..\n";
        return false;
    }
    entry e = manFiles[tag];
    std::string cpCmd = "cp " + e.src.string() + " " + home+"/work/ndep/.ndeploy/backups";
    system(cpCmd.c_str());
    std::cout<<"[+] set checkpoint for "<< tag << '\n';
    return true;
}
//!
//& revert
bool revert(std::string tag){
    if(manFiles.find(tag)==manFiles.end()){
        std::cout<<"[--] tag not found..\n";
        return false;
    }
    entry e = manFiles[tag];
    std::filesystem::path backup = home+"/work/ndep/.ndeploy/backups/" + e.src.filename().string();
    std::string cpCmd = "cp " + backup.string() + " " + e.src.string();
    system(cpCmd.c_str());
    std::cout<<"[+] reverted to checkpoint for "<< tag << '\n';
    return true;
}
//!
//& deployToPath
bool deployToPath(std::string tag){
    if(manFiles.find(tag) == manFiles.end()){
        std::cout << "[--] tag not found..\n";
        return false;
    }
    entry& e = manFiles[tag];
    std::error_code ec;
    std::filesystem::path dest = "/usr/local/bin/" + e.src.filename().string();
    //faster
    std::filesystem::copy_file(e.src, dest, std::filesystem::copy_options::overwrite_existing, ec);
    
    if (ec) {
        std::cout << "[--] Failed: " << ec.message() << " (Did you forget sudo/SUID?)\n";
        return false;
    }
    
    std::cout << "[++] deployed " << tag << " to PATH\n";
    return true;
}
//!
//& main
int main(int argc, char* argv[]){
    //load from watch file into manFiles.
    setgid(0);
    if (setuid(0) != 0) {
        std::cout<<"ndep not running as root, some commands might not work.\n";
    }
    home = getenv("HOME");
    watchfile = home + "/work/ndep/.ndeploy/watched_files.nd";
    inW.open(watchfile);
    
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
    else if(cmd=="add-d"){
        if(manFiles.find(tagC)==manFiles.end()){
            std::cout<<"[--] tag not found..\n";
            return false;
        }
        manFiles[tagC].dests.push_back(filepath);
        saveFiles();
    }
    else if(cmd=="addg"){
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
    else if(cmd=="dep"){
        deploy(filepath);
    }
    else if(cmd=="depg"){
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
    else if(cmd=="set"){
        if(manFiles.find(filepath)==manFiles.end()){
            std::cout<<"[--] tag not found..\n";
            return 1;
        }
        set(filepath);
    }
    else if(cmd=="rev"){
        if(manFiles.find(filepath)==manFiles.end()){
            std::cout<<"[--] tag not found..\n";
            return 1;
        }
        revert(filepath);
    }
    else if(cmd=="setg"){
        if(TagGroups.find(filepath)==TagGroups.end()){
            std::cout<<"[--] group not found..\n";
            return 1;
        }
        tag_group& e = TagGroups[filepath];
        for(auto& t : e.tags){
            if(!set(t)) return 1;
        }
    }
    else if(cmd=="revg"){
        if(TagGroups.find(filepath)==TagGroups.end()){
            std::cout<<"[--] group not found..\n";
            return 1;
        }
        tag_group& e = TagGroups[filepath];
        for(auto& t : e.tags){
            if(!revert(t)) return 1;
        }
    }
    else if(cmd=="depp"){
        deployToPath(filepath);
    }
    return 0;
}
//!
//& saveFiles
void saveFiles(){
    std::ofstream out(watchfile, std::ios::trunc);
    for(auto& [tag, e] : manFiles){
        out<<"T:" << tag << " " << e.src.string() << " ";
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
//!
