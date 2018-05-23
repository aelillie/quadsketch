# QuadSketch with Random Bits

A University project on the QuadSketch algorithm from the paper:
"Piotr Indyk, Ilya Razenshteyn, Tal Wagner, "Practical Data-Dependent Metric Compression with Provable Guarantees", NIPS 2017"

The following is the abstract from our paper, which summarized our work.

**Abstract:**
*This paper analyzes, experiments, and verifies the relatively new algorithm QuadSketch proposed in [Indyk et al.] and makes a contribution by modifying the decompression step of QuadSketch, inserting random bits instead of zeros. A baseline algorithm has been implemented, which verifies the results from [Indyk et al.] on SIFT and MNIST datasets. Additional experiments have been performed on GIST and a synthetic dataset, CLUSTERS. Experiments compare the performance of QuadSketch against the modified algorithm, named QSR. The experiments shows a better average case performance of QSR in regards to accuracy and distortion over QuadSketch, in particular on MNIST. Finally a theoretical proof of QSR is given, showing a better expected distance preservation than QuadSketch.*

## Report
"report/" contains the actual report of the project, including the findings, contributions and results.

## Source code
"src/" contains the original algorithm by Piotr Indyk, Ilya Razenshteyn, and Tal Wagner. Additionally it contains our baseline implementation of *Grid*, and the modified *QuadSketch* algorithm, *QSR*.

## Datasets
Run /datasets/download.sh to download the datasets. 

Note that this does not download CLUSTERS, which needs to be generated manually by gen.cpp. Run datasets/raw/clusters/setup.sh to do this.

## Instructions for execution
All source code for the algorithms are located in src/.

### QuadSketch
Compile with "make qs" and run qs.

For example: qs -i ../datasets/sift -o sift_results.txt -n 8 -d 7 -l 5

### QSR
Compile with "make qsr" and run qsr.

For example: qsr -i ../datasets/clusters -o clusters_results.txt -n 8 -d 7 -l 5 -t 4

The extra "-t" argument specifies the number of threads for running the ANN search.

### Grid
Compile with "make grid" and run grid.

For example: grid -i ../datasets/mnist -o mnist_results.txt -l 32 -t 4

In this case "-l" is the number of landmarks, and again "-t" is the number of threads.

## Results
The test results run by the test suite in test/ are located in dataAnalysis/. CSV files exist for each dataset an are located in their respective folders with corresponding names. Furthermore Graphs/ contain graphs of the results, which are also presented in the report.
