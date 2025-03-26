package util;

import java.util.ArrayList;
import java.util.HashMap;

public class CmdParser {


    private String args[];
    private ArrayList<Arg> proccessor;
    private HashMap<String ,String> proccessed;
    private boolean exitOnFailure = false;

    public CmdParser(String args[]){
        this.args = args;
    }
    public CmdParser(){}

    public void addArgument(Arg arg){
        proccessor.add(arg);
    }
    public void proccessArgument(String args[]){
        this.args = args;
        proccessArgument();
    }
    public void proccessArgument(){

    }
    public void printHelpMessage(){
        for (Arg arg:proccessor){
            System.out.println(arg.getArgMessage());
        }
    }
    public void logError(String message){
        printHelpMessage();
        Main.log(message);
    }

}
