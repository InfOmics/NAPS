#ifndef States_Network_H
#define States_Network_H

/*#define DEBUG_ON
#define TRACE_ON*/

#ifdef DEBUG_ON
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif


#define PERIOD 10 //sync with tb PERIOD


//definition of a generic states

	typedef enum {NOT_AVAILABLE, AVAILABLE} MET_STATES; //common between mets and reactions
	typedef enum {RESET, DONE} REAC_SIGNALS;			//common between mets and reactions

//definition of the network

#endif
