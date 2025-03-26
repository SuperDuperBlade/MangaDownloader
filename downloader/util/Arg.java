package downloader.util;

public class Arg {
    public String identifier;
    public String helpMessage;
    public String defualt = "";
    public boolean isRequired;
    public boolean hasChild;

    public Arg(String identifier, String helpMessage, boolean isRequired, boolean hasChild) {
        this.identifier = identifier;
        this.helpMessage = helpMessage;
        this.isRequired = isRequired;
        this.hasChild = hasChild;
    }
    public Arg(String identifier, String helpMessage, boolean isRequired, boolean hasChild,String defualt) {
        this.identifier = identifier;
        this.helpMessage = helpMessage;
        this.isRequired = isRequired;
        this.hasChild = hasChild;
        this.defualt = defualt;
    }

    public String getArgMessage(){
        return "identifier: "+identifier+" ,Info:"+helpMessage +" ,isRequired: "+isRequired+" ,hasChild:"+hasChild;
    }

    public boolean IsValid(String identifier){
        return  this.identifier.contentEquals(identifier);
    }
}
