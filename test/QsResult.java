public class QsResult {
    //Fields are public for generic read in writer.
    public double size; 
    public int lambda; 
    public int depth; 
    public String dataset; 
    public double B; 
    public double accuracy;
    public double distortion;
    public int dim; 
    public double aspectRatio; 
    public int blocks; 

    public QsResult(String dataset, int blocks, int depth, int lambda, double size, int dim, double aspectRatio, double accuracy, double distortion){
        this.size = size; 
        this.lambda = lambda; 
        this.depth = depth;
        this.dataset = dataset; 
        this.dim = dim; 
        this.aspectRatio = aspectRatio; 
        this.blocks = blocks; 
        this.distortion = distortion; 
        this.accuracy = accuracy;
        this.B = this.calculateB();
    }

    private double calculateB(){
        return (this.size / bytesMaxDataset());
    }

    //Theorem 3 states size is O=(n * dim * lamda + blocks * n * log n )
    private double bytesMaxDataset(){
        int sift = 1_010_000; 
        int mnist = 60_000;
        switch(this.dataset){
            case "../datasets/sift": return sift * 128;
            case "../datasets/mnist": return mnist * 784;
            default : return -1.0;
        }
    }


    @Override
    public String toString(){
        return "Dataset: " + dataset + "\n" + 
               "Blocks: " + blocks + "\n" +
               "Depth: " + depth + "\n" +
               "Lambda: " + lambda + "\n" +
               "Size: " + size + "\n" +
               "Dimensions: " + dim + "\n" +
               "Aspect Ratio: " + aspectRatio + "\n" +
               "Accuracy: " + accuracy + "\n" +
               "Distortion: " + distortion + "\n" +
               "Bits pr coord: " + this.B + "\n";
    }
}