package downloader.Util;

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

    //proccess the argument according to the args added to the class
    public void proccessArgument(){
        for (int i = 0; i < args.length; i++) {
            String user_Arg = args[i];
            Arg setting = getByIdentifier(user_Arg);
            if(setting != null){
                if(setting.hasChild){
                    if(!setting.requiresSpecific) {
                        proccessed.put(setting.identifier, args[i + 1]);
                    }else {
                        boolean isValid;
                        String value;
                        for(String msg: setting.accepted){
                            if(msg.equalsIgnoreCase(args[i])){
                                proccessed.put(setting.identifier,msg);
                                return;
                            }
                        }

                        logError("value: "+args[i+1]+ " did not match the required values attempting to set to default");
                        proccessed.put(setting.identifier,setting.defualt);
                    }
                    i++;
                }else{
                    proccessed.put(setting.identifier,"true");
                }
            }else{
                logError("invalid arg"+user_Arg);
            }
        }
    }
    //prints all the args defined to this class
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
           logError("Invalid identifier:"+identifier);
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
