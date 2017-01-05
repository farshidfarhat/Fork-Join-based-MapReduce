//===========================================---========= file = n_mm1.c =====
//=  A CSIM18 simulation of a N parallel M/M/1 queueing systems              =
//============================================================================
//=  Notes: Program written to evaluate scaling of CSIM process model        =
//=--------------------------------------------------------------------------=
//=  History:   KJC (06/30/02) - Genesis                                     =
//============================================================================
#pragma warn -pro        // Disable function prototype warn (bcc)
#pragma warn -stu        // Disable undefined structure warn (bcc)

//----- Includes --------------------------------------------------------------
#include <stdio.h>       // Needed for printf()
#include "csim.h"        // Needed for CSIM18 stuff

//----- Constants -------------------------------------------------------------
#define N           20 // Number of M/M/1 systems to simulate

//----- Globals ---------------------------------------------------------------
FACILITY Server[N];      // Pointer for N Server facilities
double   Sim_time;       // Total simulation time in seconds
double   Lambda;         // Mean arrival rate (cust/sec)
double   Mu;             // Mean service rate (cust/sec)
int      Arrival_count;  // Count of all arrivals

//----- Prototypes ------------------------------------------------------------
void generate(int sta_id);                         // Generators
void (* gen[(N)])(int sta_id);
void queue_node(int sta_id, double service_time);  // Nodes (queues)
void (* node[N])(int sta_id, double service_time);

//=============================================================================
//==  Main program                                                           ==
//=============================================================================
void sim(void)
{
  int     i;             // Loop counter
  double  qmean;         // Mean queue length

  // Create the simulation
  create("sim");

  // Output begin-of-simulation banner
  printf("*** BEGIN SIMULATION *** \n");

  // Increase maximums
  max_processes(1000000);
  max_events(1000000);
  max_servers(100000);
  max_facilities(1000000);

  // Paramter initializations
  Sim_time = 1.0e5;
  Lambda = 0.20;
  Mu = 1.00;

  // CSIM, variable, and pointer inializations
  Arrival_count = 0;
  for (i=0; i<N; i++)
  {
    Server[i] = facility("Server for modeling of node queue");
    gen[i] = generate;
    node[i] = queue_node;
  }

  // Initiate generate functions and hold for Sim_time
  for (i=0; i<N; i++)
    gen[i](i);
  hold(Sim_time);

  // Compute global results
  qmean = 0.0;
  for (i=0; i<N; i++)
    qmean = qmean + qlen(Server[i]);
  qmean = qmean / N;

  // Output results
  printf("============================================================= \n");
  printf("==   *** CSIM18 N x M/M/1 queueing system simulation ***   == \n");
  printf("============================================================= \n");
  printf("=  Num systems     = %d             \n", N);
  printf("=  Lambda          = %6.3f cust/sec \n", Lambda);
  printf("=  Mu              = %6.3f cust/sec \n", Mu);
  printf("=  Offered load    = %6.3f %%       \n", 100.0 * (Lambda / Mu));
  printf("============================================================= \n");
  printf("=  Total sim time  = %6.3f sec      \n", clock);
  printf("=  Total arrivals  = %d             \n", Arrival_count);
  printf("=  Total CPU time  = %6.3f sec      \n", cputime());
  printf("=------------------------------------------------------------ \n");
  printf("= >>> Simulation results                                    - \n");
  printf("=------------------------------------------------------------ \n");
  for (i=0; i<N; i++)
    printf("=  Qlen[%4d]      = %6.3f cust     \n", i, qlen(Server[i]));
  printf("============================================================= \n");
  printf("=  Mean queue len       = %6.3f cust     \n", qmean);
  printf("=  CPU time per arrival = %6.3f microsec \n",
    1.0e6 * cputime() / Arrival_count);
  printf("============================================================= \n");

  report_facilities();

  // Output end-of-simulation banner
  printf("*** END SIMULATION *** \n");
}

//=============================================================================
//==  Function to generate Poisson customers                                 ==
//=============================================================================
void generate(int sta_id)
{
  double  service_time;  // Service time

  create("generate");

  // Loop forever to create Poisson customers
  while(1)
  {
    hold(expntl(1.0 / Lambda));
    service_time = expntl(1.0 / Mu);
    node[sta_id](sta_id, service_time);
  }
}

//=============================================================================
//==  Function for single server queue                                       ==
//=============================================================================
void queue_node(int sta_id, double service_time)
{
  create("queue_node");

  // Increment global Arrival_count
  Arrival_count++;

  // Reserve, hold, and release server
  reserve(Server[sta_id]);
  hold(service_time);
  release(Server[sta_id]);
}
