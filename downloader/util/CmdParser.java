package downloader.util;

import downloader.Main;

import java.util.ArrayList;
import java.util.HashMap;

public class CmdParser {


    private String args[];
    private ArrayList<Arg> proccessor = new ArrayList<>();
    private HashMap<String ,String> proccessed = new HashMap<>();
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
        for (int i = 0; i < args.length; i++) {
            String user_Arg = args[i];
            Arg setting = getByIdentifier(user_Arg);
            if(setting != null){
                if(setting.hasChild){
                    proccessed.put(setting.identifier,args[i+1]);
                }else{
                    proccessed.put(setting.identifier,"true");
                }
            }else{
                logError("invalid arg");
            }
        }
    }
    public void printHelpMessage(){
        for (Arg arg:proccessor){
            System.out.println(arg.getArgMessage());
        }
    }
    public void logError(String message){
        printHelpMessage();
        Main.debug("Error:"+message);
    }
    public Arg getByIdentifier(String identifier){
        for (Arg arg: proccessor){
            if(identifier.contentEquals(arg.identifier)){
                return arg;
            }
        }
        return null;
    }
    public String getValueFromArg(String identifier){
       boolean doesExist = false;
       Arg arg = getByIdentifier(identifier);
       if (arg == null) {
           logError("Invalid identifier");
           return null;
       }

        return proccessed.getOrDefault(identifier,arg.defualt);
    }

    public boolean contains(String identifier){
        for (Arg arg:  proccessor){
            if (arg.identifier.equals(identifier)){
                return true;
            }
        }
        return false;
    }
}
