package downloader.Util;

import com.google.gson.*;

import java.io.*;
import java.net.URL;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

public class JSONparser {
    private File file;
    private HashMap<String ,String> vals;

    public  JSONparser(String path){
        Gson gson = new Gson();
        try (FileReader reader = new FileReader(path)){



            Map<String, String> data = gson.fromJson(reader,Map.class);

            vals = new HashMap<>(data);


        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public JSONparser(URL url){
        Gson gson = new Gson();
        try (FileReader reader = new FileReader(url.getPath())){



            Map<String, String> data = gson.fromJson(reader,Map.class);

            vals = new HashMap<>(data);


        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public JSONparser(InputStream inputStream){
        Gson gson = new Gson();
        try(BufferedReader br = new BufferedReader(new InputStreamReader(inputStream))) {
            Map<String, String> data = gson.fromJson(br,Map.class);

            vals = new HashMap<>(data);

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public String getValue(String key){
        return this.vals.get(key);
    }

}
