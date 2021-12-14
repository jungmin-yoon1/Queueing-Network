//I bet you didn't read these notes, did you?  Delete this line to compile
/* READ THIS BEFORE COMPILING AND RUNNING THIS PROGRAM.
  - This program was developed using the Borland C++ Rel. 2.0 compiler
  - Read the note on how to convert to workstation-ready code if you want
    to run this on a Sun or mainframe.  It's right at the top of the code
  - This is an M/M/1 simulator.  Conversion to M/M/1/K is pretty easy, if
    messages are just rejected when the population count reaches a certain
    level.  Conversion to M/M/m is a little trickier, and requires the use
    of an array for the servers to keep an account of the status of each.
    As a suggestion, the status would include the termination time of the
    element in service at that particular server.  When the server is
    empty, perhaps the time to completion of service could be made negative,
    to reflect empty status.  Note that there is no such time as a negative
    time.
  - This simulator incarnation seems to work, but I make no claim as to the
    perfection of it.  Nor do I make any claim as to preference of design
    approach.  In fact, already this is a collection of the work of a couple of
    people.  Copy this to a safe file that won't be corrupted.  As I said, it
    seems to work as is.  But then, don't hesitate to hack it up and make it
    your own.  It's fun to express yourself with creative designs.
*/


/*******************************************************************/
/*                                                                 */
/* PC USERS:  This program contains frequent usage of the          */
/* "long int" data type.  This is because while the involved values*/
/* are discrete, they are too large to fit into a 16-bit value.    */
/* EVERYONE ELSE: (Take note, workstation users) To get rid of the */
/* "long int" declaration, simply grep for "long", then do the     */
/* substitution of "int" for "long int".  I don't think that the   */
/* Sun compliler, in particular, recognizes the "long int" data    */
/* type							           */
/*******************************************************************/

/*******************************************************************/
/*                                                                 */
/*          Queueing Network Simulation Program                    */
/*                                                                 */
/*******************************************************************/
/*                                                                 */
/*          For  Class  EE381K   Project Assignment                */
/*******************************************************************/
/*       Author : Hong-Dah Sheng                                   */
/*command format:						   */
/* mm1c [arrival rate][service rate][run length][initial queue size]*/
/*								   */
/*******************************************************************/
/*	 Appended and changed for EE381K by Peter J. Zievers	   */
/*	 November-December 1992					   */
/*******************************************************************/


#include <stdio.h>
#include <math.h>  

/******************************************************************/
/*                                                                */
/*               Define System Parameter                          */
/*                                                                */
/******************************************************************/

#define maxcenter 20          /*maximum number of service center    */
#define modulus  1000000000   /*modulus of random number generator  */
#define maximum  1.0e9        /*maximun of the random integer       */
/*******************************************************************/
/*                                                                 */
/*		RANDOM NUMBER GENERATOR SEED RIGHT HERE		   */
/*                                                                 */
/*******************************************************************/
#define seed     3            /*seed of the random number generator */
/* Note that if one wanted, one could input this value at the command
line.  If that is a desired mode, then one would have to establish a
global data element of type double or type int called "seed" in the
global variable section below.  Then, one would have to use the atof()
or atoi() function in the math.h library to convert, because the input
held in the argv[] array is held as a character string.  Note that if
the seed is input from the command line, the #define would have to be
removed or commented out.  pjz 12/10/92
*/

#define population_limit 500  /*maximum number of customer allowed  */
/*check for unbounded parameter       */
#define TRUE     1


/*******************************************************************/
/*                                                                 */
/*       Define Data Structure                                     */
/*                                                                 */
/*******************************************************************/

typedef double TIME_TYPE;      /* basic time unit                   */
typedef int    CENTER_IDX;     /* type for center I.D.              */

struct event_list {           /* element of the event list         */

    TIME_TYPE           time;    /* time at which event will transpire*/
    CENTER_IDX         queue;    /* specific index of involved queue  */
    struct event_list* next;    /* pointer for linkage to next event */

};

typedef struct event_list ELEMENT;   /* Compiler directives for convenience */
typedef ELEMENT* LINK;     /* pointer to event element            */


/*****************************************************************/
/*                                                               */
/*           Global   Variable                                   */
/*                                                               */
/*****************************************************************/

int        center_no = 0;   /*actual number of service center to be run */
CENTER_IDX current_center = 0; /* center whose event is being processed */
TIME_TYPE  clock = 0;       /*the present simulation time   		*/
TIME_TYPE  sim_end = 0;     /*the amount of time to simulate 		*/
LINK       event_head;     /*pointer to header of the event list 	*/
double     arrive_rate;     /*arrival rate of customer   		*/
double	   service_rate;    /*service rate at server			*/


/*************************** My code *****************************/

int				num_servers = 5;
int				idle_servers = 5;
int				queue_size = 6;
int				population = 20;
int				service_customer_number = 0;
TIME_TYPE		p0_t = 0, p1_t = 0, p2_t = 0, p3_t = 0, p4_t = 0, p5_t = 0, p6_t = 0;
int				iter = 0;

/*****************************************************************/

    /*  system statistics*/
long int      total_customer;     /*customer present in system	*/
long int      total_event;        /*total event had been processed 	*/
long int      total_completion;   /*number of completion 		*/
TIME_TYPE total_wait_time;   /*total wait time 			*/
TIME_TYPE total_busy_time;   /*total busy time 			*/
TIME_TYPE sys_last_changed;  /*time of last change of system state */

/** Trace switch - for debugging purposes **/
int rswitch;          /* report statistics every 500 completion*/
int dswitch;          /*dump event and center state at every step */

/*******************************************************************/
/*                                                                 */
/*             Main    Program                                     */
/*                                                                 */
/*******************************************************************/

main(argc, argv)
int   argc;              /* number of optional argument */
char* argv[];           /* argument list */
{
    /*** Function ********************/
    int     init_rand();
    void report();
    int     init_query(), check_option();
    int     init_buffer();
    int     remove_first_event(), insert_event();
    double  uniform01(), mexpntl();
    long	int     srand(), subtract();
    int     in_system(), out_system();
    CENTER_IDX	dummy;
    long int	start_level;

    FILE* fp_mean_customer;
    fp_mean_customer = fopen("C:\\Users\\NESL\\source\\repos\\Jungmin\\Jungmin\\customer.txt", "a");


    /***** Execution Begin *******/

    /* First, we have to initialize everything.  In this section, we check for
    the following things:

        (1) 	Do we want to activate the debugging features, so we can see
            can see incremental progress of the system?
        (2)	If we haven't input from the command line, we need to
            parameterize the model in some way.
        (3)	All buffer structures need to be initialized.  Recall that
            this is a model of a queueing system, and queues entail buffers.
            Also, the system uses an event queue apart from the system
            under simulation, and this control structure must also be
            initialized.
        (4)	The random number generator must be initialized, and the
            transients from cold start must be purged.
        (5)	The first event must be provided, since events are produced
            in a stream, where one event leads to the creation of the
            next event.  Without the first event, the stream would never
            get started
    */

    /*        check_option(argc,argv);*/  /*check option 'r' and 'd' 	*/
    if (argc == 5)
    {
        arrive_rate = atof(argv[1]);
        service_rate = atof(argv[2]);
        sim_end = atof(argv[3]);
        start_level = atol(argv[4]);
    }
    else

        init_query(&start_level);             /*query model to be simulate  */
    init_buffer();            /*initialize buffer   	*/
    init_rand();              /*initialize random number gen. */
    dummy = 0;
    insert_event(mexpntl(1.0 / arrive_rate), dummy);
    /*generate first arrival 	*/
/* If the starting queue level is greater than 0, then have to issue a service
event to get the service distribution launched.  Otherwise, the first arrival
will trigger the start of the service distribution
*/
    if (start_level != 0)
    {
        insert_event((clock + mexpntl(1.0 / service_rate)), 1);
        total_customer = start_level;
    }

    /* The next section of code simulates the target system.  The conditionals in
    the while statement are interesting because they signal termination of the
    simulation run.  Note the visual structure of these conditionals.  Each
    conditional is stacked up so each condition is readable and obvious.  To add
    to this list of conditionals, add a "&&" to the end of the last conditional
    and insert a line below with your own conditional.
    */

    while (
        (clock < sim_end) && 	/*End of simulated run time? */
        (total_customer < population_limit)/* Queue overflow? */
        )
    {
        remove_first_event(&clock, &current_center); /*get next event and
                           update the clock in
                           the simulated system */
                           /* This switch/case structure branches control of the simulator to the
                           appropriate place to deal with the event just removed from the event queue.
                           Adding treatments to this is as easy as assigning a code number to the type of
                           treatment and putting in a new case() statement similar to those shown below.
                           If you aren't sure how the switch statement works, check a C programming text
                           for lots of detail on how to handle switch structures.  Kelley and Pohl is a
                           pretty good text, or Lippman if you like to code in C++.
                           */
        switch (current_center) {
        case(0): /* an arrival into system*/
            in_system();    /*do system statistics for newcomer */
            break;
        case(1): /* A completion of service */
            out_system();    /* leave the system */
            break;
        default:
            printf("There's a screwy element in the event queue\n");
            exit(1);
        }

        /* This stuff below is for debugging purposes.  If you activated the debugging
        information from the command line, this piece will be activated and stuff will
        print out when you run the simulator.  If not, this piece will not be executed
        */

        if (dswitch)
            dump_event_list();

        iter++; 

    };

    /**************************** My code ****************************/
    printf("Iteration : %d \n\n", iter);
    printf("\n");
    printf("p0 : %lf\n", p0_t / clock);
    printf("p1 : %lf\n", p1_t / clock);
    printf("p2 : %lf\n", p2_t / clock);
    printf("p3 : %lf\n", p3_t / clock);
    printf("p4 : %lf\n", p4_t / clock);
    printf("p5 : %lf\n", p5_t / clock);
    printf("p6 : %lf\n", p6_t / clock);
    printf("\n");
    /*****************************************************************/


    if (total_customer > population_limit) /* Did the simulator stop because
                        the max. queue level was
                        exceeded? */
    {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("!		BLOWUP                    !\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    /* end of simulation*/
    report();     /*generate final report */
    fprintf(fp_mean_customer, "%lf ", total_wait_time / clock);
    fclose(fp_mean_customer);
    system("pause");
}
/********************************************************************/
/*     Function :  check_option()                                   */
/********************************************************************/

check_option(argc, argv)

int  argc;
char* argv[];

{
    int  i;
    char c;
    for (i = 1; i < argc; ++i) {
        c = *argv[i];
        switch (c) {
        case 'd': dswitch = TRUE;  /*enable dump at each step */
            break;
        default: printf("\nOnly: 'd' and 'r' allowed\n");
            break;
        }
    }
}
/***************************************************************/
/*  Function : init_query                                     */
/***************************************************************/
/* ask for configuration and parameter to simulate */
init_query(begin)
long int* begin;	/* starting queue level */
{
    int     i, j;
    double  pb_sum;

    center_no = 1;

    /*ask arrival rate and mean service time */
    printf("\nPlease enter the arrival rate of customer?   ");
    scanf("%lf", &arrive_rate);
    printf("\nPlease enter the mean service time of queue? ");
    scanf("%lf", &service_rate);

    /*ask for amount of time to simulate */
    printf("\nPlease enter the amount of time to be simulated?");
    scanf("%lf", &sim_end);
    printf("\nPlease enter the starting queue length ");
    scanf("%ld", begin);
}

/**********************************************/
/* Function:init_buffer                       */
/**********************************************/

init_buffer()

{
    int i;                        /* the center index */

                           /* initialize system state and statistics */
    event_head = NULL;
    total_customer = 0;
    total_event = 0;
    current_center = 0;
    clock = 0.0;
    total_completion = 0;
    total_wait_time = 0.0;
    total_busy_time = 0.0;
    sys_last_changed = 0.0;

    /*****************************Change******************************/
    num_servers = 5;
    idle_servers = 5;
    service_customer_number = 0;
    /*****************************************************************/

}


/********************************************/
/* Function: remove_first_event             */
/********************************************/

remove_first_event(t, x)

TIME_TYPE* t;
CENTER_IDX* x;
{
    if (event_head == NULL)        /* warning for error */
        printf("empty event_list");
    else {                    /* get the first event in the list */
        LINK t1 = event_head;
        *t = t1->time;	/* Extract time from event */
        *x = t1->queue;	/* Extract indicator from event */
        event_head = event_head->next; /* Link in new head of list */
        free(t1);         /* deallocate memory to OS */
        ++total_event; /* increment count for processed event*/
    }
}

/*******************************************/
/* Function: insert_event                  */
/*******************************************/

insert_event(t, x)

TIME_TYPE    t;
CENTER_IDX   x;

{
    CENTER_IDX	holder;
    LINK t2;
    LINK t1 = (LINK)malloc(sizeof(ELEMENT));
    /*obtain buffer from OS */
 /* fill data in this buffer */
    holder = x;
    t1->next = NULL;
    t1->queue = holder;
    t1->time = t;

    if (event_head == NULL)  /* the list is free,put on head*/
        event_head = t1;
    else {
        if (t < event_head->time) {
            /*timer smaller than other event*/
            /*put in head of list*/
            t1->next = event_head;
            event_head = t1;
        }
        else {
            /* put this element in the list */
            /* in order of time sequence    */
            t2 = event_head;
            while ((t2->next != NULL) && (t > t2->next->time))
                t2 = t2->next;
            t1->next = t2->next;
            t2->next = t1;
        };
    };
}


/***********************************************************/
/*Function: in_system                                      */
/**********************************************************/
/* A new customer arrives at the system. Update system statistics */
/*Schedule next arrival                                     */

in_system()
{
    double	mexpntl();
    int i, suc;
    LINK dum;
    TIME_TYPE temp;

    suc = 0;

    total_busy_time += (clock - sys_last_changed) * service_customer_number;
    total_wait_time += (clock - sys_last_changed) * total_customer;

    if (total_customer == 1)
        p1_t += (clock - sys_last_changed);
    else if (total_customer == 2)
        p2_t += (clock - sys_last_changed);
    else if (total_customer == 3)
        p3_t += (clock - sys_last_changed);
    else if (total_customer == 4)
        p4_t += (clock - sys_last_changed);
    else if (total_customer == 5)
        p5_t += (clock - sys_last_changed);
    else if (total_customer == 6)
        p6_t += (clock - sys_last_changed);
    else
        p0_t += (clock - sys_last_changed);
    if (idle_servers > 0)
    {
        insert_event(clock + mexpntl(1.0 / service_rate), 1);			// Create Departure event
        service_customer_number++;
        idle_servers--; // idle_servers = num_servers - customer_in_service;
    }

    // Increment total customer count
    total_customer++;

    // Update the time marker
    sys_last_changed = clock;

    //seed = rand() % 1000;

    // Create next Arrive event
    if (total_customer == queue_size)
    {
        temp = clock;

        while (suc == 0)
        {
            temp = temp + mexpntl(1.0 / (arrive_rate * (population - total_customer)));
            dum = event_head;

            if (temp > dum->time)
                suc = 1;
        }
        insert_event(temp, 0);				// Create Arrival event
    }

    else
        insert_event(clock + mexpntl(1.0 / (arrive_rate * (population - total_customer))), 0);  // Create Arrival event
}

/*****************************************************************/
/*Function: out_system                                           */
/****************************************************************/
/* A customer leaves the system. Do system statistics             */
/* If interim report needed (at every 500 completion),           */
/*   call report function                                        */

out_system()
{
    void report();
    double mexpntl(TIME_TYPE);
    int i;

    total_completion++;

    total_busy_time += (clock - sys_last_changed) * service_customer_number;
    total_wait_time += (clock - sys_last_changed) * total_customer; //(total_customer - service_customer_number);


    if (total_customer == 1)
        p1_t += (clock - sys_last_changed);
    else if (total_customer == 2)
        p2_t += (clock - sys_last_changed);
    else if (total_customer == 3)
        p3_t += (clock - sys_last_changed);
    else if (total_customer == 4)
        p4_t += (clock - sys_last_changed);
    else if (total_customer == 5)
        p5_t += (clock - sys_last_changed);
    else if (total_customer == 6)
        p6_t += (clock - sys_last_changed);
    else
        p0_t += (clock - sys_last_changed);

    //seed = rand() % 1000;

    if (total_customer - service_customer_number > 0)
        insert_event(clock + mexpntl(1.0 / service_rate), 1);

    else
    {
        idle_servers++;
        service_customer_number--;
    }

    // Decrement total customer count
    total_customer--;

    // Update the time marker
    sys_last_changed = clock;

    /* If the debugging info. is asked for, the next statement activates */
    if (rswitch && ((total_completion % 500) == 0))
        report();
}

/*****************************************************************/
/* Function: report                                              */
/******************************************************************/
/* generate report of (1) total completion (2)total event         */
/* (3) present clock (4) average system performance parameters    */

void report()
{
    int i;
    printf("\n\n\nTotal Completion:%7ld", total_completion);
    printf("\nNumber of Events Processed:%7ld", total_event);
    printf("\nSimulated time : %8.1f seconds", clock);

    printf("\n\n\nPerformance of the System\n");

    /*overall system   performance */
    printf("  Average    Average     Average   		    Average\n");
    printf("  Service    Arrival     Waiting    Utilization     System \n");
    printf("    Rate       Rate        Rate       Factor        Population\n");
    printf("\n------------------------------------------------------------------------");
    printf("\n %7.3f        %6.3f     %5.3f       %3.4f         %5.3f\n",
        total_busy_time / (double)total_completion,
        (double)total_completion / clock,
        total_wait_time / (double)total_completion,
        arrive_rate / service_rate,
        total_wait_time / clock);
}


/***************************************************************/
/* Function: Dump_event_list                                   */
/****************************************************************/
/* dump all event                                               */

dump_event_list()
{
    LINK t1;
    printf("\nEvent_list=");
    t1 = event_head;
    while (t1 != NULL) {
        printf("!%10f>%2d ", t1->time, t1->queue);
        t1 = t1->next;
    }
}
/***************************************************************/
/* Data structure for random number generator                  */
/***************************************************************/

long int   y[55];

/***************************************************************/
/* Function: init_rand()                                       */
/***************************************************************/
init_rand()

{
    long	int subtract();

    long 	int x1, x2, x3, i, temp, srand();
    /*generate initial data for the array  */
    x1 = y[34 * 21 % 55] = seed;
    x2 = y[(1 + 34) * 21 % 55] = 1;
    for (i = 2; i <= 54; ++i)
    {
        x3 = y[(i + 34) * 21 % 55] = subtract(x1, x2);
        x1 = x2;
        x2 = x3;
    };
    /* skip first 165 data  */
    for (i = 1; i <= 165; srand(), ++i);
}

/**********************************************************************/
/* Function: mexpntl(mean)                                             */
/**********************************************************************/
/* a negative exponential random variable generator                   */

TIME_TYPE	mexpntl(mean)
TIME_TYPE	mean;

{
    double uniform01();

    return((-mean) * log(uniform01())); }

/**********************************************************************/
/* Function: uniform01()                                              */
/**********************************************************************/
/* a uniform randon number generator in [0,1]                        */

double	uniform01()

{
    long int	srand();

    return((double)srand() / maximum);
}

/********************************************************************/
/* Function: srand()                                                */
/********************************************************************/
/* generate a random integer number from 0 to 1000000000            */
/* The range of the number depends on the modulus that is operated  */
/* on in subtract().  This modulus is set in a #define at the top   */
/* of the code						            */

long int srand()
{
    static	long int	n_minus_55;
    long	int t1, subtract();
    t1 = subtract(y[n_minus_55], y[(n_minus_55 + 31) % 55]);
    y[n_minus_55] = t1;
    n_minus_55 = (n_minus_55 + 1) % 55;
    return (t1);
}

/********************************************************************/
/* Function: subtract(a,b)                                         */
/********************************************************************/

long int	subtract(a, b)
long int a, b;
{
    if (a >= b)
        return ((a - b));
    else
        return ((modulus - (b - a)));
}
