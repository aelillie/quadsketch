import java.util.*;
import java.io.*;
import java.lang.reflect.Field;

public class CsvWriter{
    public void write(String foldername, ArrayList<QsResult> results){
        try{
            PrintWriter writer = new PrintWriter(new File(foldername + "test.csv"));
            StringBuilder sb = InitStringBuilderWithHeaders(results.get(1));
            for(QsResult res: results){
                for (Field property : res.getClass().getDeclaredFields()) {
                    String propVal = property.get(res).toString();
                    System.out.println("Propval: " + propVal);
                    sb.append(propVal);
                    sb.append(",");
                  }
                  sb.append("\n");
            }
    
            writer.write(sb.toString());
            writer.close();
            System.out.println("Done writing csv");
        }catch(Exception ex){
            System.err.println(ex.toString());
        }
       
    }

    public StringBuilder InitStringBuilderWithHeaders(QsResult result){
        StringBuilder sb = new StringBuilder();
        for (Field property : result.getClass().getDeclaredFields()) {
            sb.append(property.getName() + ",");
        }

        sb.setLength(sb.length()-1); //remove last comma. 
        sb.append("\n");
        return sb; 

    }
}