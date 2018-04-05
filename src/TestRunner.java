import java.io.*;

public class TestRunner {

    public static void main(String[] args) {
        try {
            String command = combineArgs(args);
            System.out.println("Starting command: " + command);
            run(command);
            System.out.println("Done running command.");
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println(e.getMessage());
        }
    }

    public static String combineArgs(String[] args){
        String s = "";
        for(int i = 0; i < args.length; i++){
            s += args[i] + " ";
        }
        System.out.println(s);
        return s;
    }

    public static void run(String command) throws Exception {
        System.out.println("Starting command: " + command);
        Process pro = Runtime.getRuntime().exec(command);
        printLines(command + " stdout:", pro.getInputStream());
        pro.waitFor();
        System.out.println(command + " exitValue() " + pro.exitValue());
    }

    public static void printLines(String name, InputStream ins) throws Exception {
        String line = null;
        BufferedReader in = new BufferedReader(
            new InputStreamReader(ins));
        while ((line = in.readLine()) != null) {
            System.out.println(name + " " + line);
        }
    }

}