#include<string>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include<sstream>
#include<vector>
#include <unistd.h>
#include <filesystem>
//  initial stuff
enum CMDS {ADD, ADDD, ADDG, DEP, DEPP, SET, REV, SETG, REVG, DEL, DEPG};
std::string home;
struct entry{
    std::filesystem::path src;
    std::vector<std::string> dests;
};
struct tag_group{
    std::vector<std::string> tags;
};
std::unordered_map<std::string,entry> manFiles;
std::unordered_map<std::string,tag_group> TagGroups;
//these are global so i dont pass them to each function that uses them (2 whole functions)
std::string watchfile;
std::ifstream inW;
std::string lineW;


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
        std::cout << "[--] Failed: " << ec.message()<<'\n';
        return false;
    }
    
    std::cout << "[++] deployed " << tag << " to PATH\n";
    return true;
}

void deleteTag(std::string tag){
    if(manFiles.find(tag)==manFiles.end()){
        std::cout<<"[--] tag not found..\n";
        return;
    }
    manFiles.erase(tag);
    std::cout<<"Successfully removed " << tag << '\n';
    saveFiles(); 

}

//the 3 functions below are ugly and made to keep the main() clean.
void assignCmdToEnum(std::string cmd, CMDS* e_){
    //we put the value of e_ according to the value of cmd
    if(cmd=="add") *e_ = ADD;
    else if(cmd=="add-d") *e_ = ADDD; 
    else if(cmd=="addg") *e_ = ADDG;
    else if(cmd=="dep") *e_ = DEP;
    else if(cmd=="depg") *e_ = DEPG;
    else if(cmd=="depp") *e_ = DEPP;
    else if(cmd=="set") *e_ = SET;
    else if(cmd=="rev") *e_ = REV;
    else if(cmd=="revg") *e_ = REVG;
    else if(cmd=="setg") *e_ = SETG;
    else if(cmd=="del") *e_ = DEL;

}

void handleCommands(CMDS* cmds, std::string var1, std::string var2) {
    if (!cmds) return;

    
    entry e;
    std::string tag;

    switch (*cmds) {
        case ADD:
            e.src = var1;
            manFiles[var2] = e;
            saveFiles();
            break;

        case ADDD:
            if (manFiles.find(var2) == manFiles.end()) {
                std::cout << "[--] tag not found..\n";
                break;
            }
            manFiles[var2].dests.push_back(var1);
            saveFiles();
            break;

        case ADDG: {
            tag_group t; 
            std::string grp = var1;
            while (true) {
                std::cout << "Tag to add: ";
                std::getline(std::cin, tag);
                if (tag == "hlt") break;
                t.tags.push_back(tag);
            }
            TagGroups[grp] = t;
            saveFiles();
            break;
        }

        case DEP:
            deploy(var1);
            break;

        case DEPG: {
            if (TagGroups.find(var1) == TagGroups.end()) {
                std::cout << "[--] group not found..\n";
                break;
            }
            tag_group& ei = TagGroups[var1];
            for (auto& t_tag : ei.tags) {
                if (!deploy(t_tag)) {
                    std::cout << "not deployed\n"; 
                    break;
                }
            }
            std::cout << "deploy group " << var1 << " successfully \n";
            break;
        }

        case SET:
            set(var1);
            break;

        case REV:
            revert(var1);
            break;

        case SETG: {
            if (TagGroups.count(var1)) {
                tag_group& ei = TagGroups[var1];
                for (auto& t_tag : ei.tags) {
                    if (!set(t_tag)) break;
                }
            }
            break;
        }

        case REVG: {
            if (TagGroups.count(var1)) {
                tag_group& ei = TagGroups[var1];
                for (auto& t_tag : ei.tags) {
                    if (!revert(t_tag)) break;
                }
            }
            break;
        }

        case DEPP:
            deployToPath(var1);
            break;
            
        case DEL:
            deleteTag(var1);
            break;
        default:
            break;
    }
}


void parseFile(){
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
}

int main(int argc, char* argv[]){
    //  run as root
    setgid(0);
    if (setuid(0) != 0) {
        std::cerr<<"ndep not running as root, some commands might not work.\n";
    }
    if (setreuid(0, 0) != 0) {
        std::cerr << "ndep not running as root, some commands might not work.\n";
    }
    //  variables for functions
    home = getenv("HOME");
    watchfile = home + "/work/ndep/.ndeploy/watched_files.nd";
    inW.open(watchfile);

    //  variables for current
    CMDS e_;
    std::string cmd;
    std::string var1;
    std::string var2;
    if(argc>=2) cmd = argv[1];
    if(argc>=3) var1 = argv[2];
    if(argc>=4) var2 = argv[3];


    //  actual command handling
    parseFile();
    assignCmdToEnum(cmd, &e_);
    handleCommands(&e_, var1, var2);


    return 0;
}
