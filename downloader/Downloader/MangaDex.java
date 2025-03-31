package downloader.Downloader;

import com.google.gson.*;
import downloader.Main;
import downloader.Util.Arg;
import downloader.Util.CmdParser;
import downloader.Util.FileHandler;
import downloader.Util.JSONparser;


import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
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
        public String coverUrl;
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
                    .connectTimeout(Duration.ofSeconds(40))
                    .version(HttpClient.Version.HTTP_1_1)
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
        float previousChapter =0;

        mngInfo.title = getTitle();
        mngInfo.coverFileName = getCoverFileName();

        String url = jparser.getValue("baseSite_MANGA")
                +cparser.getValueFromArg(mangaIdentifier)+
                "/feed?translatedLanguage[]="+cparser.getValueFromArg(langIdentifier);
        JsonObject jobj = getJsonFromUrlWithMaxLimit(url);

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

            if (!attributes.get("chapter").isJsonNull()) cinfo.chapter = attributes.get("chapter").getAsString();
            else cinfo.chapter = String.valueOf(previousChapter+0.00001);


            previousChapter = Float.parseFloat(cinfo.chapter);
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

        //Get covers for volumes
        String cover_url = jparser.getValue("baseSite_COVER")
                +cparser.getValueFromArg(mangaIdentifier);
        JsonObject coverObj= getJsonFromUrlWithMaxLimit(cover_url);

        for(JsonElement cover: coverObj.getAsJsonArray("data")) {
            JsonObject attributes = cover.getAsJsonObject().getAsJsonObject("attributes");
            if (attributes.get("volume").isJsonNull()){
                Main.debug("Skipping due to null volume");
                continue;
            }

            String volume = attributes.get("volume").getAsString();
            for (volumeInfo vinfoI:mngInfo.volumes){
                if (vinfoI.title.equals(volume)){
                    if (attributes.get("fileName").isJsonNull()){
                        Main.debug("Skipping due to null url");
                        continue;
                    }
                    vinfoI.coverUrl = attributes.get("fileName").getAsString();
                    Main.debug("Found cover for volume: "+volume);
                    break;
                }
            }

        }

        return mngInfo;
    }

    public String getTitle() {
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
        String mode = cparser.getValueFromArg(modeIdentifier);
        if (mangInfo.coverFileName !=null&& mangInfo.coverFileName != "") {
            String coverFileNamePrefix = "\\00_cover" + mangInfo.coverFileName;
            String coverUrl = jparser.getValue("downloadSite_COVER") + cparser.getValueFromArg(mangaIdentifier) + "/" + mangInfo.coverFileName;
            if (mode.equalsIgnoreCase("manga")) {
                downloadImage(coverUrl, fileDir + coverFileNamePrefix);
            }
        }
        long filecounter = 0;
        int volumeCounter = 0;
        long chapterCounter = 0;
        for (volumeInfo vinfo: mangInfo.volumes){
            fileDir = mangaOutDir;
            if (mode.equalsIgnoreCase("Volume")||mode.equalsIgnoreCase("Volumes")){
                fileDir += "\\"+vinfo.title+"v_"+sanitisedTitle;


                filecounter=0;

                if (FileHandler.doesExist(fileDir+".cbz")){
                    if (!(volumeCounter+1 >= mangInfo.volumes.size())){
                        volumeInfo vinfo2 = mangInfo.volumes.get( (volumeCounter+1));
                       String secondFile = mangaOutDir+"\\"+vinfo.title+"v_"+sanitisedTitle+".cbz";
                       if (FileHandler.doesExist(secondFile)){
                           continue;
                       }
                    }
                }
                FileHandler.mkdir(fileDir+"\\");

                //Downloads the cover for the volume if not null
                if (vinfo.coverUrl!=null&&vinfo.coverUrl!="") {
                    String coverFilepath = fileDir + "\\" + "00_cover" + vinfo.title + "v_" + sanitisedTitle;
                    downloadImage(jparser.getValue("downloadSite_COVER") + cparser.getValueFromArg(mangaIdentifier) + "/" + vinfo.coverUrl, coverFilepath);
                }
            } else if (mode.equalsIgnoreCase("Manga")) {
                fileDir +=  "\\"+sanitisedTitle;
                FileHandler.mkdir(fileDir);


            }
            for (chapterInfo cinfo: vinfo.chapters){
                if (!isInRange(cinfo)){
                    continue;
                }
                if (mode.equalsIgnoreCase("Chapter")||mode.equalsIgnoreCase("Chapters")){
                    fileDir += "\\"+vinfo.title+"v_"+cinfo.chapter+"c_"+sanitisedTitle;


                    if (FileHandler.doesExist(fileDir+".cbz")){
                        long nextInfoIndex = vinfo.chapters.indexOf(cinfo)+1;
                        if (!(nextInfoIndex >= vinfo.chapters.size())){
                            chapterInfo cinfo2 = new chapterInfo();
                            String nextCBZ = mangaOutDir+"\\"+vinfo.title+"v_"+cinfo2.chapter+"c_"+sanitisedTitle+".cbz";
                            if (FileHandler.doesExist(nextCBZ)){
                                continue;
                            }
                        }
                    }
                    FileHandler.mkdir(fileDir);
                    filecounter =0;
                }

                //checks if the cbz eqivelent already exits but continues to download if the next volume/chapter is missing
                for (String filename :cinfo.filenames_data){
                   // String responce  = sendRequestViaBaseUrl();
                    String finalFilepath = fileDir+"\\"+String.valueOf(filecounter)+filename;
                    if(!FileHandler.doesExist(finalFilepath)) downloadImage(jparser.getValue("downloadSite_Data")+"/"+cinfo.hash+"/"+filename,finalFilepath);
                    filecounter++;
                }

            }
            volumeCounter++;
        }

        compile(mangaOutDir);

    }
    public void compile(String headDir){
        File[] directories = new File(headDir).listFiles(File::isDirectory);

//        Main.debug(String.valueOf(directories.length));
        for (File dir: directories){
            Main.debug("Now compiling "+dir.getAbsolutePath());
            FileHandler.compressFolder(dir.getAbsolutePath());
        }

    }
    public float[] getHighestChapterAndVolume(){
        float highestVolume=0, highestChapter =0;


        String url = jparser.getValue("baseSite_MANGA")
                +cparser.getValueFromArg(mangaIdentifier)+
                "/feed?translatedLanguage[]="+cparser.getValueFromArg(langIdentifier);
        JsonObject jobj = getJsonFromUrlWithMaxLimit(url);
        ArrayList<chapterInfo> cinfos = new ArrayList<>();
        float previousChapter = 0;
        for(JsonElement chapter: jobj.getAsJsonArray("data")){
            JsonObject attributes = chapter.getAsJsonObject().getAsJsonObject("attributes");
            float currentVolume =  attributes.get("volume").getAsFloat();
            float currentChapter =0.0f;
            if (!attributes.get("chapter").isJsonNull()) currentChapter = attributes.get("chapter").getAsFloat();
            else currentChapter =  (previousChapter+0.00001f);


            previousChapter = currentChapter;
            if(currentVolume > highestVolume){
                highestVolume = currentVolume;
            }

            if (currentChapter > highestChapter){
                highestChapter = currentChapter;
            }
        }
        float toReturn[] = new float[]{highestVolume,highestChapter};
        return toReturn;
    }

    public JsonObject getJsonFromUrlWithMaxLimit(String url){

        JsonObject jobj = JsonParser.parseString(
                        sendRequestViaBaseUrl(url))
                .getAsJsonObject();

        String limit = jobj.get("total").getAsString();
        url+= "&limit="+limit;

        jobj  = JsonParser.parseString(sendRequestViaBaseUrl(url)).getAsJsonObject();
        return jobj;
    }
    public ArrayList<String> getMangaIDsFromAuthor(String author){
        ArrayList<String> mangaIDS= new ArrayList<>();

        JsonObject obj = JsonParser.parseString(sendRequestViaBaseUrl(jparser.getValue("baseSite_AUTHOR")+author)).getAsJsonObject();


        JsonObject attributeObj = obj.getAsJsonObject("data");
        for (JsonElement el : attributeObj.getAsJsonArray("relationships")) {
            JsonObject arrayObj = el.getAsJsonObject();

            JsonPrimitive typeObj = arrayObj.getAsJsonPrimitive("type");
            String type = typeObj.getAsString();
            if (type.equals("manga")){
                mangaIDS.add(arrayObj.getAsJsonPrimitive("id").getAsString());
            }

        }
        return mangaIDS;
    }

}
