#include <stdio.h>      // Needed for printf()
#include "csim.h"       // Needed for CSIM18 stuff

//----- Defines ---------------------------------------------------------------
#define SIM_TIME 1.0e6  // Total simulation time in seconds
#define atom_service_time 1.5 // 0.8
#define xeon_service_time 1.0 // 1.0
#define Nm_max 10
#define Nm_min 10
//#define	MapTaskNum 50
#define Lambda 10.0 // / MapTaskNum) //10 * (0.5 ... 1+0.5) = 5 ... 15
#define NARS 5000000
#define K 1.0 // should be one in FJQ
#define	Tcoef 0.01
//const double service_times[] = { 1, 2, 3, 4, ... };

//----- Globals ---------------------------------------------------------------
//Facilities
FACILITY atom[Nm_max];          // Declaration of CSIM Atom facility
FACILITY xeon[Nm_max];          // Declaration of CSIM Xeon facility

//Queue Lengths
QTABLE qtbl_atom_qlen[Nm_max];
QTABLE qtbl_xeon_qlen[Nm_max];

//Throughputs
TABLE tbl_atom_tput[Nm_max];
TABLE tbl_xeon_tput[Nm_max];

//Utilizations
TABLE tbl_atom_util[Nm_max];
TABLE tbl_xeon_util[Nm_max];
TABLE tbl_mst[3][Nm_max];

//Event
EVENT done;		// pointer for counter

//Functions
void CSIM_INIT();
void traffic();
void customer(); // arriving customer
void mapper(double type, double mid, EVENT* task_done_event);

double L[3],T1,T2;
double Miu_Sum = 1/atom_service_time + 1/xeon_service_time;
double Arrival_Time; // (1 / Lambda)
int SAc;
int Nm;
int mapc;
double jobclock;

void sim(void)
{
	// Create the simulation
	create("sim");
	done = event("done");
	max_events(1000000000l);
	max_processes(1000000000l);
	//trace_on();

	T1 = Tcoef * xeon_service_time;
	T2 = Tcoef * atom_service_time;

	CSIM_INIT();
	
	for(Nm = Nm_min; Nm <= Nm_max; Nm++)
	{
		// Common Parameters
		// l1=l2=...=ln
		L[0] = 0.5;
		// u1=u2=...=un
		L[1] = 1.0 / (xeon_service_time * Miu_Sum);
		// r1=r2=...=rn
		//L[2] = (1.0 / xeon_service_time) - (Miu_Sum * Nm - Lambda * MapTaskNum) / (2* Nm);
		L[2] = (1.0 / xeon_service_time) - (Miu_Sum * Nm - Lambda) / (2* Nm);
		//L[2] = L[2] / (L[2] + 1.0 / atom_service_time - (Miu_Sum * Nm - Lambda * MapTaskNum) / (2* Nm));
		L[2] = L[2] / (L[2] + 1.0 / atom_service_time - (Miu_Sum * Nm - Lambda) / (2* Nm));
		printf("Miu_Sum = %6.3f, Lambda = %6.3f\n", Miu_Sum, Lambda);
		printf("xeon_service_time = %6.3f, atom_service_time = %6.3f, T1 = %6.3f, T2 = %6.3f\n", xeon_service_time, atom_service_time, T1, T2);
		printf("L[0] = %6.3f, L[1] = %6.3f, L[2] = %6.3f, \n", L[0], L[1], L[2]);
		for(SAc = 0; SAc < 3; SAc++)
		{
			printf("*** BEGIN SIMULATION SA%d with %d Mappers *** \n",SAc, Nm);
			clear(done);
			traffic();	// generate traffic
			wait(done);	// wait for last customer to depart
			//reset();
			//CSIM_INIT();
			report_facilities();
			report_table(tbl_mst[SAc][Nm-1]);

			// sprintf(buf, "%d", Nm);
			// strcpy(str,"output_");
			// strcat(str,buf);
			// strcat(str,"_");
			// sprintf(buf, "%d", SAc);
			// strcat(str,buf);
			// FILE *fp = fopen (str, "w"); 
			// set_output_file (fp);
			// close(fp);

			printf("*** END SIMULATION SA%d with Mappers *** \n",SAc, Nm);
			//rerun();
			reset();
			//CSIM_INIT();
			// done = event("done");
			// max_events   (1000000000l);
			// max_processes(1000000000l);
		}
	}
	report();
	mdlstat();	// model statistics
}

void CSIM_INIT()
{
	//CSIM initializations
	int Nmc;
	char buf[10];
	char str[14];
	create("init");
	for(Nmc = 0; Nmc < Nm_max; Nmc++)
	{
		//Initialize and Name Facilities
		sprintf(buf, "%d", Nmc+1);
		strcpy(str,"ATOM_");
		strcat(str,buf);
		atom[Nmc] = facility(str);
		strcpy(str,"XEON_");
		strcat(str,buf);
		xeon[Nmc] = facility(str);

		//Initialize and Name Tables
		sprintf(buf, "%d", Nmc+1);
		strcpy(str,"MST_Table_L_");
		strcat(str,buf);
		tbl_mst[0][Nmc] = table(str);
		strcpy(str,"MST_Table_U_");
		strcat(str,buf);
		tbl_mst[1][Nmc] = table(str);
		strcpy(str,"MST_Table_R_");
		strcat(str,buf);
		tbl_mst[2][Nmc] = table(str);
	}
}

void traffic()
{
	int i;
	create("traffic");
	mapc = 0;
	jobclock = clock;
	Arrival_Time = 1 / Lambda; //K*2*Nm / Lambda;
    for (i=1; i<= NARS;i++)
	{
		hold(expntl(Arrival_Time));//(K*Nm*2*Arrival_Time));
		customer();
	}
	//printf("traffic is done\n");
	set(done);
}

void customer()				// arriving customer
{
	//double mst0;
	int mtc;
	create("customer");
    EVENT task_done_event;
	task_done_event = event("task_done"); // create event for task synchronization
	//mst0 = clock;			// record start time
	for (mtc = 0; mtc < 1; mtc++) //K*Nm*2; mtc++)
		//hold(expntl(Arrival_Time));
		mapper(uniform(0,1), uniform(0, Nm), &task_done_event);
	mtc = 0;
	//printf("before while in customer\n");
	while (mtc < 1) //< K*Nm*2)// wait for all tasks to complete 
	{
		clear(task_done_event);
		wait(task_done_event);
		mtc++;
		mapc++;
	}
	// the customer can leave the system 
	if(mapc == K*Nm*2)
	{
		record(clock - jobclock, tbl_mst[SAc][Nm-1]);
		mapc = 0;
		jobclock = clock;
	}
}

void mapper(double type, double mid, EVENT* task_done_event)//, unsigned int jid, double service_time)
{
	create("mapper");
	//printf("type = %6.3f, mid = %6.3f, int(mid) = %d\n", type, mid, (int)mid);
	if(type < L[SAc])
	{//Xeon
		reserve(xeon[(int)mid]);
		hold(expntl(xeon_service_time)+T1);
		release(xeon[(int)mid]);
	}
	else
	{//ATOM
		reserve(atom[(int)mid]);
		hold(expntl(atom_service_time)+T2);
		release(atom[(int)mid]);
	}
	set(*task_done_event);
}
