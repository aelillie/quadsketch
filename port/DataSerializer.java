import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class DataSerializer<T> {

    private static <T> T deserialize(File file, T entity) {
        Serializer<T> serializer = new Serializer<>();
        return null;
    }

    public static <T> T deserialize(String fileName, T entity) {
        File input = new File(fileName);
        if (input == null || !input.exists()) {
            throw new RuntimeException("can't open file for reading");
        }
        return deserialize(input, entity);
    }

    static class Serializer<S> {
        List<S> items;
        Serializer() {
            items = new ArrayList<>();
        }
    }
}