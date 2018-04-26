import java.io.*;

public class TestRunner {

    private static String initArg = "-i ../datasets/sift -o sift_resultsB*D#L!.txt -n * -d # -l !";

    public static void main(String[] args) {
        int depth = 10;
        for(int lambda = 2; lambda < depth; lambda++){
            try {
                int numberOfWorkers = depth-lambda-1;
                int index = 0; 
                String[] commands = new String[numberOfWorkers];
                for(int i = lambda + 1; i < depth; i++){
                    char lm = (lambda + "").toCharArray()[0]; 
                    char dp = (i + "").toCharArray()[0];
                    String command = combineArgs(args, 16, dp, lm);   
                    commands[index] = command;   
                    index++;      
                }
    
                Worker[] workers = new Worker[numberOfWorkers];
    
                //create and start workers; 
                for(int i = 0; i < workers.length; i ++){
                    Worker worker = new Worker(i, commands[i]);
                    workers[i] = worker;
                    worker.start();
                }
    
                //wait for workers to finish.
                for(int i = 0; i < workers.length; i++){
                    workers[i].join();  
                }
                System.out.println("Iteration done for lambda: " + lambda);
                System.out.println();
    
            } catch (Exception e) {
                e.printStackTrace();
                System.out.println(e.getMessage());
            }
        }
    }
        

    public static String combineArgs(String[] args, int block, char depth, char lambda){
        String vars = initArg.replace("*", block + "").replace('#', depth).replace('!', lambda);
        String command = args[0] + " " + vars; 
        System.out.println(command);
        return command;
    }
}