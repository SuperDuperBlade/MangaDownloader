import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class Main {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
    public static void log(String message){
        LocalDateTime localDateTime = LocalDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("[dd:MM:yyyy HH:mm:ss]: ");
        System.out.println(formatter.format(localDateTime)+message);
    }

}