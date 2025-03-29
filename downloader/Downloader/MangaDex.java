package downloader.Downloader;

import com.google.gson.*;
import downloader.Main;
import downloader.Util.Arg;
import downloader.Util.CmdParser;
import downloader.Util.FileHandler;
import downloader.Util.JSONparser;


import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Map;

public class MangaDex {
    private class mangaInfo{
       public String MangaId;
       public String title;
       public String coverFileName;
       public ArrayList<volumeInfo> volumes = new ArrayList<>();
       public boolean hasUnorderedVolume;

       public void reset(){
           MangaId = null;
           title = null;
           volumes.clear();
           hasUnorderedVolume = false;
       }
    }
    private class volumeInfo{
        public String title;
       public ArrayList<chapterInfo> chapters ;

        public volumeInfo(String title,ArrayList<chapterInfo> chapterInfos){
            this.title = title;
            chapters = chapterInfos;
        }
        public volumeInfo(){
            chapters =  new ArrayList<>();
        }

        public void reset(){
            title = null;
            chapters.clear();
        }

        public ArrayList<chapterInfo> getChapters() {
            return chapters;
        }
    }
    private class chapterInfo{
        public String id,title,hash,volume,chapter;
        public ArrayList<String> filenames_data = new ArrayList<>();
        public ArrayList<String> filenames_datasaver =new ArrayList<>();

        public void reset(){
            id = null;
            title = null ;
            hash = null;
            volume = null;
            chapter = null;
            filenames_data.clear();
            filenames_datasaver.clear();
        }
    }

    private CmdParser cparser ;
    JSONparser jparser;
    private int rateLimit = 20; //seconds
    private final String mangaIdentifier = "-i",
    outDirIdentifier = "-o",
    modeIdentifier = "-m",
    rangeIdentifier = "-r",
    langIdentifier = "-l";
    boolean isUsingRange = false, isRangeMaxEnabled = false;
    float rangeMin= 0, rangeMax =0;

    public MangaDex(String[] args) {
        this.cparser = new CmdParser(args);
        this.cparser.addArgument(new Arg(mangaIdentifier, "The id of the manga", true, true));
        this.cparser.addArgument(new Arg(outDirIdentifier, "The id of the manga", false, true, System.getProperty("user.dir")));
        this.cparser.addArgument(new Arg(modeIdentifier, "The way to download files in", false, true, "Volume", new String[]{"Volumes", "Volume", "Manga", "Chapters", "Chapter"}));
        this.cparser.addArgument(new Arg(rangeIdentifier, "The range of chapters/volumes to download", false, true));
        this.cparser.addArgument(new Arg(langIdentifier, "The language to download the manga in (shorthand)", false, true, "en"));
        this.cparser.proccessArgument();

        isUsingRange = cparser.getValueFromArg(rangeIdentifier) != null;
        if (isUsingRange) {
            String ranges[] = rangeIdentifier.split(":");
            isRangeMaxEnabled = (ranges.length == 2);
            if (ranges.length > 2) {
                new Exception("More ranges where supplied than added (2)");
            } else {
                rangeMin = Float.parseFloat(ranges[0]);
                if (isRangeMaxEnabled) rangeMax = Float.parseFloat(ranges[1]);
            }


        }

        this.jparser = new JSONparser(Main.class.getClassLoader().getResourceAsStream("mangaDex.json"));
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
                 //   .header("Content-Type","application/json")
                    .build();


            HttpResponse<String> responce = hclient.send(request, HttpResponse.BodyHandlers.ofString());
            if(responce.statusCode() ==202 || responce.statusCode() ==200){
                Main.debug("Request was successful: "+url);
                return responce.body();
            }else if(responce.statusCode() == 429){
                Main.debug("Encountered rate limit waiting "+rateLimit+" seconds...");
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


        mngInfo.title = getTitle();
        mngInfo.coverFileName = getCoverFileName();

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

        float volumeCounter = Float.parseFloat(cinfos.get(0).volume);
        int volumeRange = getVolumeRange(cinfos);
        if(mngInfo.hasUnorderedVolume) volumeCounter =0;



        volumeInfo vinfo = new volumeInfo();
        vinfo.title = cinfos.get(0).volume;
        for (chapterInfo cinfo: cinfos) {
            if(Float.parseFloat(cinfo.volume) == volumeCounter){
                vinfo.chapters.add(cinfo);
            }else{
                String title = vinfo.title;

                mngInfo.volumes.add(vinfo);
                vinfo = new volumeInfo();

                volumeCounter++;
                vinfo.title = String.valueOf(volumeCounter);
                //this chapter belongs in the new volume
                vinfo.chapters.add(cinfo);
            }

        }
         mngInfo.volumes.add(vinfo);

        for (volumeInfo vinfos : mngInfo.volumes){
            for (chapterInfo cinfo: vinfos.chapters) {
                Main.debug(String.valueOf(vinfos.chapters.size()));
                Main.debug(vinfos.title + "v " +cinfo.chapter+" c");
            }
        }
        return mngInfo;
    }

    private String getTitle() {
      String responce =  sendRequestViaBaseUrl(jparser.getValue("baseSite_MANGA")+cparser.getValueFromArg(mangaIdentifier));
      JsonObject jobj = JsonParser.parseString(responce).getAsJsonObject();
      JsonObject dataObj = jobj.getAsJsonObject("data");
      JsonObject attrributeObj = dataObj.getAsJsonObject("attributes");
     JsonObject title = attrributeObj.getAsJsonObject("title");
        Map.Entry<String, JsonElement> firstEntry = title.entrySet().iterator().next();
     return firstEntry.getValue().toString();
    }


    public int getVolumeRange(ArrayList<chapterInfo> cinfos){
        return Integer.parseInt(cinfos.get(cinfos.size()-1).volume) - Integer.parseInt(cinfos.get(0).volume);
    }

    public String getCoverFileName(){
        String responce =  sendRequestViaBaseUrl(jparser.getValue("baseSite_MANGA")+cparser.getValueFromArg(mangaIdentifier));
        JsonObject jobj = JsonParser.parseString(responce).getAsJsonObject();

        JsonObject attributeObj = jobj.getAsJsonObject("data");
        for (JsonElement el : attributeObj.getAsJsonArray("relationships")){
            JsonObject arrayObj = el.getAsJsonObject();

            JsonPrimitive typeObj = arrayObj.getAsJsonPrimitive("type");
            String type = typeObj.getAsString();
            if (type.equals("cover_art")){
                return arrayObj.getAsJsonPrimitive("id").getAsString();
            }
        }
        return null;
    }

    public boolean isInRange(chapterInfo chapterInfo){

        String mode = cparser.getValueFromArg(modeIdentifier);


        if (mode.equalsIgnoreCase("volumes")|| mode.equalsIgnoreCase("volume")){
            float volume = Float.parseFloat(chapterInfo.volume);
            if (isRangeMaxEnabled){
                return volume > rangeMin && volume < rangeMax;
            }else{
                return volume > rangeMin;
            }
        } else if (mode.equalsIgnoreCase("chapters")||mode.equalsIgnoreCase("chapter")) {
            float chapter = Float.parseFloat(chapterInfo.chapter);
            if (isRangeMaxEnabled){
                return chapter >rangeMin && chapter < rangeMax;
            }
        }
        //range is unsupported for this mode so will always return true
        return true;
    }

    public void downloadImage(String url, String path){
        Main.debug("Downloading image from: "+url);
        if (FileHandler.doesExist(path)){
            File fileToDelete = new File(path);
            Main.debug("File already exits deleteing file and creating new one: "+path);
            fileToDelete.delete();
        }
        try(InputStream in = new URL(url).openStream()){
            Files.copy(in, Paths.get(path));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void downloadManga(){
        //Gets the mangaMetadata
        mangaInfo mangInfo = getMetaData();

        String sanitisedTitle = FileHandler.sanitise(mangInfo.title);
        String outdir = cparser.getValueFromArg(outDirIdentifier);
        String mangaOutDir = cparser.getValueFromArg(outDirIdentifier)+"\\"+sanitisedTitle;
        String fileDir = mangaOutDir;
        //creates the Folder and downloads the cover
        FileHandler.mkdir(mangaOutDir+"\\");

        String coverFileNamePrefix = "\\00_cover"+sanitisedTitle;
     //   String coverBuffer  =sendRequestViaBaseUrl(jparser.getValue("downloadSite_COVER")+cparser.getValueFromArg(mangaIdentifier)+"/"+mangInfo.coverFileName);

        String mode = cparser.getValueFromArg(modeIdentifier);

        long filecounter = 0;
        for (volumeInfo vinfo: mangInfo.volumes){
            fileDir = mangaOutDir;
            if (mode.equalsIgnoreCase("Volume")||mode.equalsIgnoreCase("Volumes")){
                fileDir += "\\"+vinfo.title+"v_"+sanitisedTitle;
                FileHandler.mkdir(fileDir+"\\");
                filecounter=0;
            } else if (mode.equalsIgnoreCase("Manga")) {
                fileDir +=  "\\"+sanitisedTitle;
            }
            for (chapterInfo cinfo: vinfo.chapters){
                if (!isInRange(cinfo)){
                    continue;
                }
                if (mode.equalsIgnoreCase("Chapter")||mode.equalsIgnoreCase("Chapters")){
                    fileDir += "\\"+vinfo.title+"v_"+cinfo.chapter+"c_"+sanitisedTitle;
                    FileHandler.mkdir(fileDir);
                    filecounter =0;
                }

                for (String filename :cinfo.filenames_data){
                   // String responce  = sendRequestViaBaseUrl();
                    String finalFilepath = fileDir+"\\"+String.valueOf(filecounter)+filename;
                    downloadImage(jparser.getValue("downloadSite_Data")+"/"+cinfo.hash+"/"+filename,finalFilepath);
                    filecounter++;
                }

            }
        }



    }
    public void compile(String headDir){
        File[] directories = new File(headDir).listFiles(File::isDirectory);

//        Main.debug(String.valueOf(directories.length));
        for (File dir: directories){
            Main.debug("Now compiling "+dir.getAbsolutePath());
            FileHandler.compressFolder(dir.getAbsolutePath());
        }

    }

}
