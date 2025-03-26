package downloader.Downloader;

import downloader.Util.Arg;
import downloader.Util.CmdParser;

import java.lang.reflect.Array;
import java.util.ArrayList;

public class MangaDex {
    private class mangaInfo{
       public String MangaId;
       public String title;
       public ArrayList<volumeInfo> vinfos = new ArrayList<>();
       public boolean hasUnorderedVolume;
    }
    private class volumeInfo{
        public String title;
        public ArrayList<chapterInfo> cinfo = new ArrayList<>();
    }
    private class chapterInfo{
        public String id,title,hash,volume,chapter;
        public ArrayList<String> filenames_data = new ArrayList<>();
        public ArrayList<String> filenames_datasaver =new ArrayList<>();

    }

    private CmdParser cparser ;


    public MangaDex(String[] args){
        this.cparser = new CmdParser(args);
        this.cparser.addArgument(new Arg("-i","The id of the manga",true,true));
        this.cparser.addArgument(new Arg("-o","The id of the manga",false,true,System.getProperty("user.dir")));
        this.cparser.addArgument(new Arg("-m","The way to download files in",false,true,"Volume",new String[]{"Volumes","Volume","Manga","Chapters","Chapter"}));
        this.cparser.addArgument(new Arg("-r","The range of chapters/volumes to download",false,true));
    }

    






}
