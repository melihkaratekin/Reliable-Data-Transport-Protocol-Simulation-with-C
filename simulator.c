#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Message Struct
struct msg {
  char data[20];
  };

// Packet Struct
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };

// Functions
void starttimer(int, float);
void stoptimer(int);
void tolayer3(int,struct pkt);
void tolayer5(int, struct msg);
int check_checksum(struct pkt);
int generate_checksum(struct pkt);
int flip_number(int);


#define A 0
#define B 1
#define TIMER 20
#define MESSAGE_SIZE 20

int A_STATE = 0;
int B_STATE = 0;

int count = 0;

int ACK = 0;
int SEQ = 0;
int prev_sequence_number;
struct msg prev_message;
struct pkt prev_packet;
struct pkt B_prev_packet;

/* called from layer 5, passed the data to be sent to other side.
Return a 1 if data is accepted for transmission, negative number otherwise */
int A_output(message)
  struct msg message;
{
    //check if we are waiting for ack?
    //return -1
    //printf("\n----------------------------------------------------------------------------------------\nTHIS IS THE MESSAGE FROM THE UPPER LAYERS\n%s\n----------------------------------------------------------------------------------------\n",message.data);
    if(A_STATE)
	{
        printf("\n----------------------------------------------------------------------------------------\nA is currently waiting for an ACK# %d\n----------------------------------------------------------------------------------------\n",ACK);
        return -1;
    }
	else
	{
        //Build the packet
        struct pkt packet;
        strcpy(packet.payload,message.data);
        packet.seqnum = SEQ;
        packet.acknum = flip_number(ACK);
        packet.checksum = generate_checksum(packet);
    
        prev_packet = packet;
        
        A_STATE = flip_number(A_STATE);
        SEQ = flip_number(SEQ);

        printf("\n----------------------------------------------------------------------------------------\nA is going to send the following packet:\nSequence number: %d\nChecksum: %d\nMessage: %s\nThe ACK A is expecting to see is %d\n----------------------------------------------------------------------------------------\n",packet.seqnum,packet.checksum,packet.payload,ACK);

        tolayer3(A, packet); 
        starttimer(A,TIMER);
        
        return 1;
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    printf("\n----------------------------------------------------------------------------------------\nA has received the ACK packet!\nACK: %d\nCurrent expected ACK: %d\n----------------------------------------------------------------------------------------\n",packet.acknum,ACK);
    if(packet.acknum == ACK && check_checksum(packet)){
        stoptimer(A);
        
        struct msg message;
        strcpy(message.data,packet.payload);
      	printf("\n----------------------------------------------------------------------------------------\nA has received the ACK # %d \n----------------------------------------------------------------------------------------\n", ACK);
        
        ACK = flip_number(ACK);
        A_STATE = flip_number(A_STATE);
        tolayer5(A,message);
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    printf("\n----------------------------------------------------------------------------------------\nA timer has been interrupted, A will now resend the packet. \n----------------------------------------------------------------------------------------\n");
    printf("\n----------------------------------------------------------------------------------------\nA is going to send the following packet:\nSequence number: %d\nChecksum: %d\nMessage: %s\nThe ACK A is expecting to see is %d\n----------------------------------------------------------------------------------------\n",prev_packet.seqnum,prev_packet.checksum,prev_packet.payload,ACK);

    tolayer3(A,prev_packet);
    starttimer(A,TIMER);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    A_STATE = 0;
    ACK = 0;
    SEQ = 0;
}

/* used to check the checksum */
int check_checksum(struct pkt package){
    int i;
    int sum = (package.seqnum + package.acknum);
    for(i = 0; i < MESSAGE_SIZE; i++){
        sum += (int)package.payload[i];
    }
    return (sum == package.checksum);
}
/* used to generate checksum */
int generate_checksum(struct pkt package){
    int i;
    int sum = (package.seqnum + package.acknum);
    for(i = 0; i < MESSAGE_SIZE; i++){
        sum += (int)package.payload[i];
    }
    return sum;            
}
/* used to alternate the number*/
int flip_number(int number){
    if(number == 0) return 1;
    else return 0;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    printf("\n----------------------------------------------------------------------------------------\nB input here is the packet I received:\nSequence number: %d\nChecksum: %d\nMessage: %s\nThe ACK B is expecting to send is %d\n----------------------------------------------------------------------------------------\n",packet.seqnum,packet.checksum,packet.payload,ACK);

    printf("B the checksum is %d\nB current state is %d\n",generate_checksum(packet),B_STATE);
        if(check_checksum(packet) && packet.seqnum == B_STATE){
            struct msg message;
            strcpy(message.data,packet.payload);
            
            B_STATE = flip_number(B_STATE);

            struct pkt ack_packet;
            ack_packet.acknum = packet.seqnum;
            ack_packet.checksum = generate_checksum(ack_packet);

            B_prev_packet = ack_packet;

            printf("\n----------------------------------------------------------------------------------------\n B is sending the ACK \n----------------------------------------------------------------------------------------\n");

            count++;
            tolayer5(B,message);
            tolayer3(B, ack_packet);
            stoptimer(B);
            starttimer(B,TIMER);
        }else{printf("----------------------------------------------------------------------------------------\nB is NOT accepting this packet!\n B has accepted the %d letter of the alphabet and is expecting the next\n----------------------------------------------------------------------------------------\n",count);
        }

}

/* called when B's timer goes off */
void B_timerinterrupt()
{
    printf("\n----------------------------------------------------------------------------------------\nB timer interrupt, B is now resending the packet.\n----------------------------------------------------------------------------------------\n");
    tolayer3(B,B_prev_packet);
    starttimer(B,TIMER);
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    B_STATE = 0;
}

// Event Struct
struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */

/* use for bidirectional transfer of data */
#define BIDIRECTIONAL 0 

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define   A    0
#define   B    1

int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from layer 5 to 4 so far */ 
int nsimmax = 0;           /* number of msgs to generate, then stop */
float simul_time = 0.000;  /* global simulation simul_time */
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */   
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int   ncorrupt;            /* number corrupted by media*/
int   randseed;            /* random number seed */


/* use only for biderectional data transfer */
int B_output(message)  /* need be completed only for extra credit */
  struct msg message;
{

  return 0;
}



/****************** EVENT LIST ROUTINE  *************/
/* Event list manipulation routines                 */
/****************************************************/
void insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",simul_time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime); 
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q; 
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

void printevlist()
{
  struct event *q;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



void stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q;

 if (TRACE>2)
    printf("          STOP TIMER: stopping timer at %f\n",simul_time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

 struct event *q;
 struct event *evptr;

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",simul_time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)  
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }
 
/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  simul_time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
} 


/************************** TOLAYER3 ***************/
double random_number() {

  // generate a uniform random number in the interval [0,1)
  return (double)1.0*rand()/(RAND_MAX+1.0);
}

void init_random(unsigned int randseed) {
  
  // initialize the random number generator
  if (!randseed) {
    srand((unsigned int)time(NULL));
  } else
    srand(randseed);
}

void tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 float lastime, x;
 int i;

 ntolayer3++;

 /* simulate losses: */
 if (random_number() < lossprob)  {
      nlost++;
      if (TRACE>0)    
	printf("          TOLAYER3: packet being lost\n");
      return;
 }  

 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = simul_time;
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) 
      lastime = q->evtime;
 evptr->evtime =  lastime + 1.0 + 9.0*random_number();
 

 /* simulate corruption: */
 if (random_number() < corruptprob)  {
    ncorrupt++;
    if ( (x = random_number()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)    
	printf("          TOLAYER3: packet being corrupted\n");
    }  

  if (TRACE>2)  
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
} 

void tolayer5(AorB, msgReceived)
  int AorB;
  struct msg msgReceived;
{
  int i;  
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)  
        printf("%c",msgReceived.data[i]);
     printf("\n");
  }
  
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
 
void generate_next_arrival(int entity)
{
   double x;
   struct event *evptr;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
 
   x = lambda*random_number()*2;   /* x is uniform on [0,2*lambda] */
                             		/* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  simul_time + x;
   evptr->evtype =  FROM_LAYER5;

   if (entity) 
     evptr->eventity = entity;
   else {
     if (BIDIRECTIONAL && (random_number()>0.5) )
       evptr->eventity = B;
     else
       evptr->eventity = A;
   }
   insertevent(evptr);
} 


/*************** INITIALIZATION ROUTINE  *************/
/* Read input from user and initalize parameters     */
/*****************************************************/
void init()                         
{
   int i;
   float sum, avg;

   printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("Enter the number of messages to simulate: ");
   scanf("%d",&nsimmax);
   printf("Enter packet loss probability [enter 0.0 for no loss]: ");
   scanf("%f",&lossprob);
   printf("Enter packet corruption probability [0.0 for no corruption]: ");
   scanf("%f",&corruptprob);
   printf("Enter average time between messages from sender's layer5 [ > 0.0]: ");
   scanf("%f",&lambda);
   printf("Enter a seed for the random number generator [0 will provide a random seed]: ");
   scanf("%d",&randseed);
   printf("Enter TRACE [0,1,2,3]: ");
   scanf("%d",&TRACE);

   /* init random number generator */
   init_random(randseed);

   sum = 0.0;         
   for (i=0; i<1000; i++)
      sum=sum+random_number();     /* should be uniform in [0,1) */
   avg = sum/1000.0;
   if ((avg < 0.25) || (avg > 0.75)) {
    printf("It is likely that random number generation on your machine\n" ); 
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine random_number() in the emulator code. Sorry. \n");
    exit(0);
   }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   simul_time=0.0;                     /* initialize simul_time to 0.0 */
   generate_next_arrival(0);     /* initialize event list */
}

/********************* MAIN ROUTINE  *****************/
/* Main simulation loop and handling of events       */
/*****************************************************/
int main(void)
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;
   
   int i,j;

   /* initialize our data structures and read parameters */
   init();

   /* call the user's init functions */
   A_init();
   B_init();
   
   /* loop forever... */
   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL) 
		{
	  		printf("INTERNAL PANIC: Event list is empty! This should NOT have happened.\n");
	  		break;
		}
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
           
        if (TRACE>=2) 
		{
           	printf("\nEVENT time: %f,",eventptr->evtime);
        	printf("  type: %d",eventptr->evtype);
	        if (eventptr->evtype==0)
		    	printf(", timerinterrupt  ");
	        else if (eventptr->evtype==1)
	            printf(", fromlayer5 ");
	        else
		     	printf(", fromlayer3 ");
	        printf(" entity: %d\n",eventptr->eventity);
        }
        simul_time = eventptr->evtime;        /* update simul_time to next event time */
        
        if (nsim==nsimmax)
	  		break;                        /* all done with simulation */
	  		
        if (eventptr->evtype == FROM_LAYER5 )
		{
            /* fill in msg to give with string of same letter */    
            j = nsim % 26; 
            for (i=0; i<20; i++)  
               msg2give.data[i] = 97 + j;
            if (TRACE>2)
			{
               printf("          MAINLOOP: data given: ");
                for (i=0; i<20; i++) 
                  printf("%c", msg2give.data[i]);
               printf("\n");
	    	}

            if (eventptr->eventity == A) 
               j = A_output(msg2give);
            else
               j = B_output(msg2give);
           
	    if (j < 0)
		{
	      if (TRACE>=1)
			printf("          MAINLOOP: data NOT accepted by layer 4 \n");
	      /* set up future arrival for the same entity*/
	      generate_next_arrival(eventptr->eventity);   

	    }
		else
		{
	      nsim++;
	      if (TRACE>=1)
			printf("          MAINLOOP: data accepted by layer 4 \n");
	      /* set up future arrival */
	      generate_next_arrival(0);   
	    }
	}
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)  
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    	if (eventptr->eventity == A)      /* deliver packet by calling */
   	       		A_input(pkt2give);             /* appropriate entity */
            else
   	       		B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT)
		  {
            if (eventptr->eventity == A) 
	       		A_timerinterrupt();
        	else
	       		B_timerinterrupt();
          }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
	     break;
	  }
        free(eventptr);
   }

   printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",simul_time,nsim);
   return 0;
}

