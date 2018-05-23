import java.util.ArrayList;

public class ResultProcessor{
    
    public static void main(String[] args){
        String foldername = "./Results/";
        QsResultReader reader = new QsResultReader();
        CsvWriter writer = new CsvWriter();
        ArrayList<String> filenames = reader.getFileNames(foldername);
        ArrayList<QsResult> results = reader.readAllResults(filenames);
        writer.write(foldername, results);
    }
}