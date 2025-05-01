package downloader.Util;

import downloader.Main;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

public class FileHandler {
    public static boolean doesExist(String path){
        File file = new File(path);
            return file.exists();
    }
    public static boolean doesExist(String path, boolean createIfNotFound){
        boolean doesExist = doesExist(path);
        if (!doesExist){
            File file = new File(path);
            if (file.isDirectory()){
                file.mkdirs();
            }else{
                try {
                    file.createNewFile();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        }
            return doesExist;
    }
    public static void mkdir(String path){
        if(doesExist(path)) return;

        File dir = new File(path);
        dir.mkdirs();

        if (System.getProperty("os.name").toLowerCase() == "linux"){
            try {
                Files.setPosixFilePermissions(dir.toPath(), PosixFilePermissions.fromString("rwxrwxrwx"));
            } catch (IOException e) {

            }
        }
    }
    public static void writeToFile(String path ,String content){
        File file = new File(path);
        //doesExist(path,true);

        try {
            FileWriter fileWriter = new FileWriter(file);
            fileWriter.write(content);
            fileWriter.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public static void compressFolder(String dirPath){
        String outPath = dirPath + ".cbz";
        File zip = new File(outPath);
        try {
            if(zip.exists()){
                File dir = new File(outPath);
                dir.delete();
            }
            Path zipPath = Files.createFile(Paths.get(outPath));

            Path inputPath =  Paths.get(dirPath);
            ZipOutputStream zout =  new ZipOutputStream(new BufferedOutputStream(new FileOutputStream(zipPath.toString())));

            Files.walk(inputPath).filter(path ->!Files.isDirectory(path))
                    .forEach(path->{
                        ZipEntry zipEntry = new ZipEntry(inputPath.relativize(path).toString());
                        try {
                            zout.putNextEntry(zipEntry);
                            Files.copy(path,zout);
                            zout.closeEntry();
                        } catch (Exception e) {
                            throw new RuntimeException(e);
                        }
                    });

            zout.close();
            File dir = new File(dirPath);
            if (dir.exists()){
                File[] contents = dir.listFiles();
                for (File f : contents){
                    f.delete();
                }
                dir.delete();
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        Main.debug("Finished compiling: "+outPath);
        if (System.getProperty("os.name").toLowerCase() == "linux"){
            try {
                Files.setPosixFilePermissions(zip.toPath(), PosixFilePermissions.fromString("rwxrwxrwx"));
            } catch (IOException e) {

            }
        }
    }
    public static String sanitise(String input){
        // Replace forbidden characters for Windows
        String sanitized = input.replaceAll("[\\\\/:*?\"<>|]", "");

        // Remove control characters except common whitespace
        sanitized = sanitized.replaceAll("[\\p{Cntrl}&&[^\r\n\t]]", "");

        // Trim and remove trailing dots/spaces
        sanitized = sanitized.trim().replaceAll("[.\\s]+$", "");

        // Truncate to the max allowed length
        if (sanitized.length() > 259) {
            sanitized = sanitized.substring(0, 259);
            // Ensure again that no trailing space/dot after truncation
            sanitized = sanitized.replaceAll("[.\\s]+$", "");
        }

        return sanitized;
    }
}
