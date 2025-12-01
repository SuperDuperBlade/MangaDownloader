package downloader;

import downloader.Downloader.MangaDex;

import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.concurrent.ExecutionException;

public class Main {
    public static void main(String[] args) {
        try {
            MangaDex mangaDex = new MangaDex(args);
            mangaDex.downloadManga();
        }catch (Exception e){
            try {
                writeToLogFile(getDateTime()+e.getStackTrace().toString());
            } catch (IOException ex) {
                System.out.println("Failed to write error to log file");
            }
            throw e;
        }

        }
    public static void debug(String message){
        String timeDateAppendix = getDateTime();
        String logMessage = timeDateAppendix+message;
        System.out.println(logMessage);
        try {
            writeToLogFile(logMessage);
        } catch (IOException e) {
            e.printStackTrace();
            System.out.println(timeDateAppendix+"Failed To Log "+message+ "To file");

        }
    }
    private static void writeToLogFile(String logMessage) throws IOException {
        FileWriter fileWriter = new FileWriter("log_"+getDate()+".txt",true);
        fileWriter.write(logMessage +"\n");
        fileWriter.close();
    }
    private static String getDate(){
        LocalDateTime localDateTime = LocalDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("[dd:MM:yyyy]");
        return formatter.format(localDateTime);
    }
    private static String getDateTime(){
        LocalDateTime localDateTime = LocalDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("[dd:MM:yyyy HH:mm:ss]");
        String timeDateAppendix = "("+formatter.format(localDateTime)+"): ";
        return timeDateAppendix;
    }
}