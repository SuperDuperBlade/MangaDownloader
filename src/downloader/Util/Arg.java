package downloader.Util;

public class Arg {
    public String identifier;
    public String helpMessage;
    public String defualt = null;
    public boolean isRequired;
    public boolean hasChild;
    public boolean requiresSpecific = false;
    public String[] accepted ;


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
    public Arg(String identifier, String helpMessage, boolean isRequired, boolean hasChild,String defualt,String accepted[]) {
        this.identifier = identifier;
        this.helpMessage = helpMessage;
        this.isRequired = isRequired;
        this.hasChild = hasChild;
        this.defualt = defualt;
        requiresSpecific = true;
        this.accepted = accepted;
    }


    public String getArgMessage(){
        if(!requiresSpecific) return "identifier: "+identifier+" ,Info:"+helpMessage +" ,isRequired: "+isRequired+" ,hasChild:"+hasChild;
        else return "identifier: "+identifier+" ,Info:"+helpMessage +"Options"+accepted.toString() +"(defualt: "+defualt+")" +" ,isRequired: "+isRequired+" ,hasChild:"+hasChild;
    }

    public boolean IsValid(String identifier){
        return  this.identifier.contentEquals(identifier);
    }
}
