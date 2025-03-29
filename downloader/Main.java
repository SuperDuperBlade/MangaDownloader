package downloader;

import downloader.Downloader.MangaDex;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class Main {
    public static void main(String[] args) {
        MangaDex mangaDex = new MangaDex(args);
        mangaDex.downloadManga();

    }
    public static void debug(String message){
        LocalDateTime localDateTime = LocalDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("[dd:MM:yyyy HH:mm:ss]");
        System.out.println("("+formatter.format(localDateTime)+"): "+message);
    }

}