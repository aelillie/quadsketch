import java.util.ArrayList;

public class ResultProcessor{
    
    public static void main(String[] args){
        QsResultReader reader = new QsResultReader();
        CsvWriter writer = new CsvWriter();
        ArrayList<String> filenames = reader.getFileNames();
        ArrayList<QsResult> results = reader.readAllResults(filenames);
        writer.write(results);
    }
}