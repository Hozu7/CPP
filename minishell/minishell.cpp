#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <bits/stdc++.h> 
#include <string.h>
#include <cctype>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h> //Shell works on linux OS


using namespace std;

//Global VARS
bool keepPrompting = true; //keep the shell going 
int exitCode = 0; //exit code
string prompt = "cwushell";

//Function PROTOTYPES
string getLow(string str); //prototype
bool isValidNum(string str); //prototype
void interpreter(string str); //prototype
void fileInfo(int status_id, string str); //prototype

//MAIN
int main() {

    string entered; //string to take input 
    
    while(keepPrompting){

        cout << prompt << ">"; //prompt

        getline(cin, entered); //take input in

        if(entered.empty()){ //null input

            cout << "Hmm no input this time \n";

            continue;
        }
        
        interpreter(entered); //call function

    }

    return (exitCode);
}

//string converter to lowercase
string getLow(string foo){

    string res = "";

    for(char ch : foo){

        res += tolower(ch);

    }

    return res;
}

//check if a string is an integer
bool isValidNum(string foo){

    bool num = true;

    for(char ch : foo){
        if(!(isdigit(ch))){

            num = false;

        }
    }

    if(num && foo.size() > 9){

        num = false;
        cout << "Exit code must be under 9 digits \n";

    }

    return num;
}

void fileInfo(int status_id, string foo){

    struct stat info; //variable of type stat to access info later

    const char* str = foo.c_str(); //convert string to const char* because stat needs const char*

    int ret = stat(str, &info); //will use the return values to check whether to print error message or actual fileInfo

    switch(status_id){

        case 0://iNode

            if(ret < 0){//if stat() return negative int then error happened

                cout << "Error retrieving information from that file\n";

            }else {

                cout << "inode: " << info.st_ino << endl;

            }
            break;

        case 1://file type

            if(ret < 0){

                cout << "Error retrieving information from that file\n";

            }else {

                if ((info.st_mode & S_IFMT) == S_IFDIR){

                    cout << "Directory\n";

                } else {

                    cout << "File\n";

                }

            }
            break;

        case 2://last modified

            if(ret < 0){

                cout << "Error retrieving information from that file\n";

            }else {

                cout << "Last modified time: " << ctime(&info.st_mtime);

            }
            break;
        
        case 3://all info

            if(ret < 0){

                cout << "Error retrieving information from that file\n";

            }else {

                //iNode
                cout << "inode: " << info.st_ino << endl;

                //File type
                if ((info.st_mode & S_IFMT) == S_IFDIR){

                    cout << "Type: Directory\n";

                } else {

                    cout << "Type: File\n";

                }

                //last modified
                cout << "Last modified time: " << ctime(&info.st_mtime) << endl;

            }
            break;

    }

}

//interpreter of commands
void interpreter(string entered){

    //TOKENIZATION
    stringstream tempStream(entered); //convert string to stringstream type for tokenization
    string temp; //temp to hold tokens and push them into vector
    vector<string> commands; //vector that'll store commands

    while(getline(tempStream, temp, ' ')){

        commands.push_back(temp); //populate vector with command and arguments; command is at index 0

    }

    //CHECK EVERYTHING
    if(getLow(commands[0]) == "exit"){//EXIT

        if(commands.size() == 1){//only exit command

            exit(exitCode);

        }else if(commands.size() == 2 && isValidNum(commands[1])){//only exit and one argument that must be a number 

            exitCode = stoi(commands[1]);//change exit code
            exit(exitCode);

        }else if((commands.size() == 2 && getLow(commands[1]) == "-help") || 
                (commands.size() == 2 && getLow(commands[1]) == "-h")){

            cout << "exit [n] -- Command to exit cwushell; n is an optional integer which will replace the exit status code.\n";

        }else{

            cout << "Invalid input; type exit -h or exit -help for command info\n";

        }

    }else if(getLow(commands[0]) == "prompt"){//PROMPT

        if(commands.size() == 1){

            prompt = "cwushell";//prompt gets back to cwushell or never changes

        }else if((commands.size() == 2 && getLow(commands[1]) == "-help") || 
                (commands.size() == 2 && getLow(commands[1]) == "-h")){

            cout << "prompt [new_prompt] -- Command to change cwushell prompt from cwushell to [new_prompt], or back to cwushell by just choosing prompt.\n";

        }else if(commands.size() == 2){

            prompt = commands[1];//prompt changes

        }else{

            cout << "Invalid input; type prompt -h or prompt -help for command info\n";

        }

    }else if(getLow(commands[0]) == "fileinfo"){//FILEINFO

        int stat_id;

        if(commands.size() == 3 && !(isValidNum(commands[1]))){

            if(getLow(commands[1]) == "-i"){

                stat_id = 0;//idNode

                fileInfo(stat_id, getLow(commands[2]));

            }else if(getLow(commands[1]) == "-t"){

                stat_id = 1;//file type

                fileInfo(stat_id, getLow(commands[2]));

            }else if(getLow(commands[1]) == "-m"){

                stat_id = 2;//last modified

                fileInfo(stat_id, getLow(commands[2]));

            }else{

            cout << "Invalid input; type fileinfo -h or fileinfo -help for command info\n";

            }
            
        }else if(commands.size() == 2 && !(isValidNum(commands[1]))){

            if(getLow(commands[1]) == "-help" || getLow(commands[1]) == "-h"){

                cout << "fileinfo [-switch] filename -- Command which will print to the screen file related information depending on the switch: \n";
                cout << " -i for inode\n -t for type of filename (directory or file)\n -m for last modified date\n -no switch for all of the information together \n";

            }else{

                stat_id = 3;//all info

                fileInfo(stat_id, commands[1]);
            }

        }else{

            cout << "Invalid input; type fileinfo -h or fileinfo -help for command info\n";

        }

    }else if(getLow(commands[0]) == "osinfo"){//OSINFO

        if(commands.size() == 2 && !(isValidNum(commands[1]))){

            struct utsname uts;

            if(getLow(commands[1]) == "-s"){

                if (uname(&uts) < 0){

                    cout << "Error retrieving information\n";

                }else{

                    cout << "OS: " << uts.sysname << endl;

                }
            }else if(getLow(commands[1]) == "-v"){

                if (uname(&uts) < 0){

                    cout << "Error retrieving information\n";

                }else{

                    cout << "Version: " << uts.version << endl;

                }

            }else if(getLow(commands[1]) == "-a"){

                if (uname(&uts) < 0){

                    cout << "Error retrieving information\n";

                }else{

                    cout << "Machine: " << uts.machine << endl;

                }

            }else if(getLow(commands[1]) == "-help" || getLow(commands[1]) == "-h"){

                cout << "osinfo [-switch] -- Command which will print to the screen OS related information depending on the switch: \n";
                cout << " -s OS name\n -v for OS version\n -a for computer architecture \n";

            }else{

            cout << "Invalid input; type osinfo -h or osinfo -help for command info\n";

            }

        }else{

            cout << "Invalid input; type osinfo -h or osinfo -help for command info\n";

        }

    }else if(commands.size() == 1 && getLow(commands[0]) == "manual"){

        cout << "exit [n] -- Command to exit cwushell; n is an optional integer which will replace the exit status code.\n";
        cout << "prompt [new_prompt] -- Command to change cwushell prompt from cwushell to [new_prompt], or back to cwushell by just choosing prompt.\n";
        cout << "fileinfo [-switch] filename -- Command which will print to the screen file related information depending on the switch: \n";
        cout << " -i for inode\n -t for type of filename (directory or file)\n -m for last modified date\n -no switch for all of the information together \n";
        cout << "osinfo [-switch] -- Command which will print to the screen OS related information depending on the switch: \n";
        cout << " -s OS name\n -v for OS version\n -a for computer architecture \n";

    }else{

        const char* str = entered.c_str(); //convert entered to type accepted by system()
        system(str);

    }
}