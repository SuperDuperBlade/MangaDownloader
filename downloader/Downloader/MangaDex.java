package downloader.Downloader;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import downloader.Main;
import downloader.Util.Arg;
import downloader.Util.CmdParser;
import downloader.Util.JSONparser;


import java.io.IOException;
import java.lang.reflect.Array;
import java.net.Authenticator;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.util.ArrayList;
import java.util.PrimitiveIterator;

public class MangaDex {
    private class mangaInfo{
       public String MangaId;
       public String title;
       public ArrayList<volumeInfo> vinfos = new ArrayList<>();
       public boolean hasUnorderedVolume;
    }
    private class volumeInfo{
        public String title;
        public ArrayList<chapterInfo> cinfos = new ArrayList<>();
    }
    private class chapterInfo{
        public String id,title,hash,volume,chapter;
        public ArrayList<String> filenames_data = new ArrayList<>();
        public ArrayList<String> filenames_datasaver =new ArrayList<>();

    }

    private CmdParser cparser ;
    JSONparser jparser;
    private int rateLimit = 20; //seconds
    private final String mangaIdentifier = "-i",
    outDirIdentifier = "-o",
    modeIdentifier = "-m",
    rangeIdentifier = "-r",
    langIdentifier = "-l";


    public MangaDex(String[] args){
        this.cparser = new CmdParser(args);
        this.cparser.addArgument(new Arg(mangaIdentifier,"The id of the manga",true,true));
        this.cparser.addArgument(new Arg(outDirIdentifier,"The id of the manga",false,true,System.getProperty("user.dir")));
        this.cparser.addArgument(new Arg(modeIdentifier,"The way to download files in",false,true,"Volume",new String[]{"Volumes","Volume","Manga","Chapters","Chapter"}));
        this.cparser.addArgument(new Arg(rangeIdentifier,"The range of chapters/volumes to download",false,true));
        this.cparser.addArgument(new Arg(langIdentifier,"The language to download the manga in (shorthand)",false,true,"en"));
        this.cparser.proccessArgument();

        this.jparser = new JSONparser(Main.class.getClassLoader().getResource("mangaDex.json").getPath());
    }

    public String sendRequestViaBaseUrl(String url){

        Main.debug("Sending Request to "+url);

        try {
            HttpClient hclient = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(24))

                    .build();
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(new URI(url))
                    .GET()
                    .timeout(Duration.ofSeconds(40))
                    .header("Content-Type","application/json")
                    .build();


            HttpResponse<String> responce = hclient.send(request, HttpResponse.BodyHandlers.ofString());
            if(responce.statusCode() ==202 || responce.statusCode() ==200){
                Main.debug("Request was successful: "+url);
                return responce.body();
            }else if(responce.statusCode() == 429){
                Main.debug("Encountered rate limit waiting "+rateLimit+" Seconds...");
                Thread.sleep(rateLimit*1000);
                return sendRequestViaBaseUrl(url);
            }else{
                Main.debug("Encountered error code: "+responce.statusCode());
                Main.debug(responce.body());
                return responce.body();
            }


        } catch (URISyntaxException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }

    }

    public mangaInfo getMetaData(){
        mangaInfo mngInfo = new mangaInfo();
        float highestChapter = 0 , highestVolume = 1 ;
        String url = jparser.getValue("baseSite_MANGA")
                +cparser.getValueFromArg(mangaIdentifier)+
                "/feed?translatedLanguage[]="+cparser.getValueFromArg(langIdentifier);
        JsonObject jobj = JsonParser.parseString(
                sendRequestViaBaseUrl(url))
                .getAsJsonObject();

        String limit = jobj.get("total").getAsString();
        url+= "&limit="+limit;

        jobj  = JsonParser.parseString(sendRequestViaBaseUrl(url)).getAsJsonObject();

        ArrayList<chapterInfo> cinfos = new ArrayList<>();
        for(JsonElement chapter: jobj.getAsJsonArray("data")){
            chapterInfo cinfo = new chapterInfo();
            JsonObject attributes = chapter.getAsJsonObject().getAsJsonObject("attributes");
            cinfo.id = chapter.getAsJsonObject().get("id").getAsString();

            if(attributes.get("title").isJsonNull()){
                cinfo.title = "";
            }else{
                cinfo.title = attributes.get("title").getAsString();
            }

            if(attributes.get("volume").isJsonNull()){
                cinfo.volume = "0";
                mngInfo.hasUnorderedVolume  =true;
            }else{
                cinfo.volume =  attributes.get("volume").getAsString();
            }

            cinfo.chapter = attributes.get("chapter").getAsString();

            //gets all the files in the chapter
            JsonObject chapterSiteObj = JsonParser.parseString(sendRequestViaBaseUrl(jparser.getValue("baseSite_CHAPTER_IMAGES")+cinfo.id)).getAsJsonObject();

            JsonObject chapterObj = chapterSiteObj.getAsJsonObject("chapter");


            cinfo.hash = chapterObj.get("hash").getAsString();


            JsonArray dataFiles  = chapterObj.get("data").getAsJsonArray();
            JsonArray dataSaverFiles = chapterObj.get("dataSaver").getAsJsonArray();
            for(JsonElement file:dataFiles){
                cinfo.filenames_data.add(file.getAsString());
            }
            for(JsonElement file:dataSaverFiles){
                cinfo.filenames_datasaver.add(file.getAsString());
            }
            //adds the chapter to the list for sorting
            cinfos.add(cinfo);
        }
        //sorts the chapters from smallest to largest
        cinfos.sort((c1,c2) -> Float.compare(Float.parseFloat(c1.chapter),Float.parseFloat(c2.chapter)));

        //checkForDuplicates
        for (int i = 0; i < cinfos.size()-1; i++) {
                if(Float.parseFloat(cinfos.get(i).chapter) == Float.parseFloat(cinfos.get(i+1).chapter)) {
                    //latter translation is usally worse
                    cinfos.remove(i + 1);
                }
        }

        //sorts them into volumes

        float volumeCounter = 1;
        if(mngInfo.hasUnorderedVolume) volumeCounter =0;

        volumeInfo vinfo = new volumeInfo();
        vinfo.title = String.valueOf(volumeCounter);
        for (chapterInfo cinfo: cinfos) {
            Main.debug(cinfo.volume + "v " +cinfo.chapter+" c"+volumeCounter);
           

        }
        mngInfo.vinfos.add(vinfo);

        for (volumeInfo vinfos : mngInfo.vinfos){
            for (chapterInfo cinfo: vinfo.cinfos) {
                Main.debug(vinfos.title + "v " +cinfo.chapter+" c");
            }
        }
        return mngInfo;
    }




}
