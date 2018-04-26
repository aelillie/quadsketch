public class Worker extends Thread {

    private String command; 
    private int tid;

    public Worker(int tid, String command){
        this.command = command;
        this.tid = tid;
    }

    @Override
    public void run() {
        try{
            System.out.println("Starting command: " + command + " from worker: " + tid );
            Process pro = Runtime.getRuntime().exec(command);
            pro.waitFor();
            System.out.println(tid + " done.");
        }catch(Exception ex){System.out.println("Error in tid: " + tid + " Ex: " + ex.toString());}
       
    }
}