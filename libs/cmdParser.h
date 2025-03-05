#pragma once
#include <map>
#include <string>
#include <any>
#include <vector>
#include <iostream>
#include <format>
using namespace std;

class CmdParser
{
    struct argument{
        string identifier = "";
        string helpMessage = "";
        bool required = false;
        bool subtype = true;
        argument(string identifierArg,string helpMessageArg,bool requiredArg,bool subtypeArg):identifier(identifierArg),helpMessage(helpMessageArg),required(requiredArg),subtype(subtypeArg){}
        argument(string identifierArg,string helpMessageArg,bool requiredArg):identifier(identifierArg),helpMessage(helpMessageArg),required(requiredArg){}
        argument(string identifierArg,string helpMessageArg):identifier(identifierArg),helpMessage(helpMessageArg){}
        argument() = default;
    };
private:
    /* data */
    std::vector<argument> arguments;
    std::map<string,string> userArgument_values;
    bool exit_On_Fail;
    int numberOfArgs;
    char* argv[];
public:

    CmdParser(bool exit_If_Failed){
        this->exit_On_Fail = exit_If_Failed;
    }
    CmdParser() {
        this->exit_On_Fail = true; //defualt
    }


    //You MUST have defined all your options before doing this!
    bool passArguments(int numberOfArgs,char* argv[]){

        

        if(getNumberOfRequiredArguments()> numberOfArgs){
            error("All of the required arguments where not put!");
            return false;
        }
        if (getNumberOfRequiredArguments() > 0 && numberOfArgs == 1) {
            error("None of the the required args where put!");
        }

        
        //starts at 1 instead of position 0 as the first argument is the executable
        for(int i = 1; i < numberOfArgs;i++){
            string u_arg = argv[i];
            bool was_found = false;

            for (argument arg: arguments){
                if(arg.identifier == u_arg ){
                    if (arg.subtype){
                        if( i +1 != numberOfArgs ){
                            this->userArgument_values[arg.identifier] =  argv[i+1];
                           
                        //skips the next provided argument as thats the value the user put
                            i++;
                            was_found = true;
                        }else {
                            error("No value provided next to argument: "+arg.subtype);
                            return false;
                        }
                    }else{
                        //just puts the identifier itself
                        userArgument_values[arg.identifier] = argv[i];
                        was_found = true;
                    }
                }

            }

            if (!was_found) {
                error("No argument was found for: " + u_arg);
            }
        }
        return false;
    }

    void addOption(string identifier,string helpMessage, bool required,bool subtype){
        arguments.push_back(argument(identifier,helpMessage,required,subtype));
    }
    void addOption(string identifier,string helpMessage, bool required){
        arguments.push_back(argument(identifier,helpMessage,required));
    }
    void addOption(string identifier,string helpMessage){
        arguments.push_back(argument(identifier,helpMessage));
    }

    string getArgument(std::string identifier){
        return userArgument_values[identifier];
    }
    map<string ,string> getArguments(){
        return  this->userArgument_values;
    }

    void error(){
        cerr << this->getHelpMessage();
        if(exit_On_Fail){
            cout << "Exiting... \n";
            exit(-1);
        }
    }

    void error(string message){
        cerr <<"Encountered error: "<< message << '\n';
        cerr << this->getHelpMessage();
        if(exit_On_Fail){
            cout << "Exiting... \n";
            exit(-1);
        }
    }

    

//displays the contents of the arguments passed into this libary 
    string getHelpMessage(){
        string helpMessage = "This program uses the following command line arguments: \n";
        for (argument arg : arguments){
            helpMessage += std::format("Identifier: {} , Required: {}, Help: {} \n",arg.identifier,arg.required,arg.helpMessage);      
        }
        return helpMessage;
    }

    void displayHelpMessage() {
        cout << getHelpMessage();
    }

    int getNumberOfRequiredArguments(){
        int counter = 0;
        for (argument arg: arguments ){
            if(arg.required) counter++;
        }
        return counter;
    }

    bool doesArgExist(string identifier) {
        return userArgument_values[identifier] != "";
    }

};
