// To run the program, enter "fjq <rho> <output filename>" where
// rho = lambda/mu is the value of utilization
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LAMBDA 1.0 // Mean job arrival rate
#define K 10 //Number of queues we simulate
#define WARM_UP_JOBS 5000000 //Warming-up jobs before analysis
#define ANALYSIS_JOBS 10000000 // Analysis jobs after warming-up
#define constN 69069 // Refer to SIMSRICPT II.5

double rho; // Value of utilization, rho = lambda/mu
double serviceTimeAvg; // Average task service time for a node
double pastQresponseT; // Q response time for past task in the same Q
unsigned long int seed = constN; // Refer to SIMSRICPT II.5
double warmUpInterval[WARM_UP_JOBS+1]; // warmUpInterval[j]: arrival interval for warming-up job j
double analysisInterval[ANALYSIS_JOBS+1]; // analysisInterval[j]: arrival interval for analysis job j
double longest2QresponseT[3][ANALYSIS_JOBS+1]; // Ref to paper
double x2[ANALYSIS_JOBS+1]; // x2[j]=longest2QresponseT[1][j]-longest2QresponseT[2][j]
double x2Avg[K+1]; // x2Avg[k]: avg of x2[1], ..., x2[ANALYSIS_JOBS] for a k-Q system
double simuT[K+1]; // T[k] from simulation for a k-queue system
double ourT[K+1]; // T[k] from Theorem 1 for a k-queue system
double ourUp[K+1]; // ourUp[k]: Upper bound from theorem 2 for a k-queue system
double ourOff[K+1]; // ourOff[k] = (ourT[k] - simuT[k])*100.0/simuT[k] for a k-Q system
double upOff[K+1]; // upOff[k] = (ourUp[k] - simuT[k])*100.0/simuT[k] for a k-Q system

/* This function returns the average value of a double array */
double arrayAvg(double array[], unsigned long length)
{
	double sum = 0.0, subsum;
	// subsum is used to avoid round-off of small values
	unsigned long outLoops, i, j, index=0;
	outLoops = (length + 9999) / 10000;
	for (i = 0; i < outLoops; i++){
		subsum = 0.0;
		for (j = 0; j < 10000 && index < length; j++)
		subsum += array[index++];
		sum +=subsum;
	}
	return (sum / length);
}

// This function is based on SIMSRICPT II.5
double exponential(double mu)
{
	const double MAX_NUM = 4294967296.0; // 2 ** 32
	seed = seed * constN;
	return -mu*log(seed / MAX_NUM);
}

int main(int argc, char *argv[])
{
	unsigned long k, j; // k is index for K Qs or the number of parallel Qs; j is the index for jobs
	char *filename;
	FILE *fp_outfile;
	// Step 1. Initialization
	if (argc != 3){
		printf("\n You forgot to input rho and file name!\n"); exit(1);
	}
	rho = atof(argv[1]);
	serviceTimeAvg = rho;	// serviceTimeAvg = 1/mu = LAMBDA/mu = rho since LAMBDA = 1.0
	filename = argv[2]; // The output file to record the results
	fp_outfile = fopen(filename, "w");
	printf( "\nM/M/1 system, rho =%.1lf, outputFile=%s\nWait for the result ......", rho, filename);

	// Step 2. Calculate arrival intervals for warming-up jobs and then analysis jobs
	for ( j = 1; j <= WARM_UP_JOBS; j++)
		warmUpInterval[j] = exponential(1.0);
	for ( j = 1; j <= ANALYSIS_JOBS; j++)
		analysisInterval[j] = exponential(1.0);

	// Step 3. Simulate the first queue to obtain simuT[1], k=1;
	pastQresponseT = 0.0; // Initialization
	// Calculate task response time for WARM_UP_JOBS tasks in Q 1
	for (j = 1; j <= WARM_UP_JOBS; j++)	// there are WARM_UP_JOBS tasks for each queue
	{
		if (pastQresponseT > warmUpInterval[j])// if > next-arrival-interval
			pastQresponseT = exponential(serviceTimeAvg) + (pastQresponseT - warmUpInterval[j]);
		else
			pastQresponseT = exponential(serviceTimeAvg);
	}
	// The last value of pastQresponseT in warming-up is used
	// as the context for the first analysis task in queue 1
	longest2QresponseT[1][0] = pastQresponseT;
	// Recursively calculate task response time for WARM_UP_JOBS
	// tasks in queue 1
	for (j = 0; j < ANALYSIS_JOBS; j++)
	{
		if (longest2QresponseT[1][j] > analysisInterval[j+1])// if "waiting time" > "next arrival interval"
			longest2QresponseT[1][j+1] = exponential(serviceTimeAvg) + (longest2QresponseT[1][j]-analysisInterval[j+1]);
		else
			longest2QresponseT[1][j+1] = exponential(serviceTimeAvg);
	}
	// simuT[1] is average of Q response times of ANALYSIS_JOBS jobs
	simuT[1] = arrayAvg(longest2QresponseT[1], ANALYSIS_JOBS);
	
	// Step 4. Recursively simulate remaining Qs to obtain simuT[k] and x2Avg[k] for k = 2, 3, …, K
	for (k = 2; k <= K; k++)
	{ 
		// Step 4.1. Recursively calculate task response time for
		// WARM_UP_JOBS tasks in queue k
		pastQresponseT = 0.0;
		for (j = 1; j <= WARM_UP_JOBS; j++)
		// there are WARM_UP_JOBS tasks for each Q k
		{
			if (pastQresponseT > warmUpInterval[j])
				pastQresponseT = exponential(serviceTimeAvg) + (pastQresponseT - warmUpInterval[j]);
			else
				pastQresponseT = exponential(serviceTimeAvg);
		}

		// Step 4.2. Recursively calculate task response time
		// for ANALYSIS_JOBS tasks in queue k
		for (j = 1; j <= ANALYSIS_JOBS; j++)
		{ // Calculate the task response time for task j in queue k
			if (pastQresponseT > analysisInterval[j]) 
			// if "waiting time" > "next arrival interval"
				pastQresponseT = exponential(serviceTimeAvg)+ (pastQresponseT - analysisInterval[j]);
			else pastQresponseT = exponential(serviceTimeAvg) ;
			// Adjust the longest and the second longest task response times from queue 1 to k for job j
			if (pastQresponseT > longest2QresponseT[1][j])
			{ // insert the pastQresponseT on the top level
				longest2QresponseT[2][j] = longest2QresponseT[1][j];
				longest2QresponseT[1][j] = pastQresponseT;
			}
			else if (pastQresponseT > longest2QresponseT[2][j])
				longest2QresponseT[2][j] = pastQresponseT;
			// replace 2nd level
			// x2[k] = longest2QresponseT[1][j] - longest2QresponseT[2][j]
			// for the k-queue system
			x2[j] = longest2QresponseT[1][j] - longest2QresponseT[2][j];
		}

		// Step 4.3. Calculate x2Avg[k]
		x2Avg[k] = arrayAvg(x2, ANALYSIS_JOBS);
		
		// Step 4.4. Calculate simuT[k]
		simuT[k] = arrayAvg(longest2QresponseT[1], ANALYSIS_JOBS);
	}

	// Step 5. Calculate others' results based on theorems 1 and 2
	ourT[1] = simuT[1];
	for (k = 2; k <= K; k++)
		ourT[k] = ourT[k-1] + x2Avg[k]/k; // Theorem 1
	ourUp[1] = simuT[1];
	
	for (k = 2; k <= K; k++)
		ourUp[k] = ourUp[k-1] + x2Avg[2]/k; // based on Theorem 2

	for (k = 1; k <= K; k++)
		ourOff[k] = (ourT[k] - simuT[k])*100.0/simuT[k];

	for (k = 1; k <= K; k++)
		upOff[k] = (ourUp[k] - simuT[k])*100.0/simuT[k];

	// Step 6. Output result to a text file
	fprintf(fp_outfile, "\n\nM/M/1, Lambda=1.0, Rho=%4.2lf, WarmupJobs=%9ld, AnalysisJobs=%9ld, K=%d\n", rho, WARM_UP_JOBS, ANALYSIS_JOBS, K);
	fprintf(fp_outfile, "\n k simuT[k] ourT[k] ourUp[k] x2Avg[k] ourOff[k]%% upOff[k]%%\n\n");
	fprintf(fp_outfile, "1 %7.3lf, %7.3lf, %7.3lf, %7.3lf, %7.3lf\n", simuT[1], ourT[1], ourUp[1], ourOff[1], upOff[1] );
	
	for (k = 2; k <= K; k++) 
		fprintf(fp_outfile, "%4d %7.3lf, %7.3lf, %7.3lf, %7.3lf, %7.3lf, %7.3lf\n",k,simuT[k],ourT[k],ourUp[k], x2Avg[k],ourOff[k],upOff[k]);
		

	fprintf(fp_outfile, "\n k simuT[k] ourT[k] ourUp[k] x2Avg[k] ourOff[k]%% upOff[k]%%\n\n");

	fclose(fp_outfile);
	return 0;
}
