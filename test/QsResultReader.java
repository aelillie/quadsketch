import java.util.ArrayList;
import java.io.*; 
public class QsResultReader{
    private String[] files;
    public ArrayList<QsResult> results;

    public ArrayList<String> getFileNames(){
        File folder = new File("./");
        File[] listOfFiles = folder.listFiles();
        ArrayList<String> filenames = new ArrayList<>();
        for (int i = 0; i < listOfFiles.length; i++) {
            File file = listOfFiles[i];
            if (file.isFile()) {
                if(file.getName().endsWith(".txt")){
                    filenames.add(listOfFiles[i].getName());
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
                System.out.println("Processing file: " + file);
                String everything = sb.toString();
                String[] everythingSplit = everything.split(" ");

                //Usefull loop for diagnosing current files. 
                // for(int i = 0; i < everythingSplit.length; i++){
                //     System.out.println("Index: " + i + " - value: " + everythingSplit[i]);
                // }
                
                QsResult res = new QsResult(
                    everythingSplit[3],
                    Integer.parseInt(everythingSplit[5]),
                    Integer.parseInt(everythingSplit[7]),
                    Integer.parseInt(everythingSplit[9]),
                    Double.parseDouble(everythingSplit[11]),
                    Integer.parseInt(everythingSplit[16]),
                    Double.parseDouble(everythingSplit[14]),
                    Double.parseDouble(everythingSplit[18]),
                    Double.parseDouble(everythingSplit[20])
                );
                
                results.add(res);

            }catch(Exception ex){
                System.out.println("Exception thrown: " + ex.getMessage());
                return null;
            }
        }
        return results; 

    }
}