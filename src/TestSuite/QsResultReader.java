import java.util.ArrayList;
import java.io.*; 
public class QsResultReader{
    private String[] files;
    public ArrayList<QsResult> results;

    public ArrayList<String> getFileNames(String foldername){
        File folder = new File(foldername);
        File[] listOfFiles = folder.listFiles();
        ArrayList<String> filenames = new ArrayList<>();
        for (int i = 0; i < listOfFiles.length; i++) {
            File file = listOfFiles[i];
            if (file.isFile()) {
                if(file.getName().endsWith(".txt")){
                    System.out.println(folder.getAbsolutePath());                    
                    System.out.println(foldername + listOfFiles[i].getName());
                    filenames.add(foldername + listOfFiles[i].getName());
                }    
            }
        }

        return filenames;
    }

    public ArrayList<QsResult> readAllResults(ArrayList<String> files){
        ArrayList<QsResult> results = new ArrayList<>();
        for (String file : files) {
            try(BufferedReader br = new BufferedReader(new FileReader(file))) {
                StringBuilder sb = new StringBuilder();
                String line = br.readLine();
            
                while (line != null) {
                    sb.append(line + " ");
                    sb.append(System.lineSeparator());
                    line = br.readLine();
                }
                String everything = sb.toString();
                String[] everythingSplit = everything.split(" ");
                QsResult res = new QsResult(
                    everythingSplit[3],
                    Integer.parseInt(everythingSplit[5]),
                    Integer.parseInt(everythingSplit[7]),
                    Integer.parseInt(everythingSplit[9]),
                    Integer.parseInt(everythingSplit[11]),
                    Integer.parseInt(everythingSplit[16]),
                    Integer.parseInt(everythingSplit[14]),
                    Double.parseDouble(everythingSplit[18]),
                    Double.parseDouble(everythingSplit[20])
                );
                System.out.println(res);
                results.add(res);

            }catch(Exception ex){
                System.out.println("Exception thrown: " + ex.toString());
                return null;
            }
        }
        return results; 

    }
}