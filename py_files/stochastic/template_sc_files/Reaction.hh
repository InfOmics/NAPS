#ifndef Reaction_H
#define Reaction_H

#include "systemc.h"
#include "States_Network.h"

//definition of a generic Reaction
//then for each Reaction create files Reaction_R#
SC_MODULE(Reaction) {						//abstract class

	public:

		//parameters
		sc_in<sc_uint<32> >	  reaction_time;

		sc_in<bool>			  params_ready;     //used to start, sync and restart simulation after out range
		sc_in<sc_uint<32> >	  t;				//like a clk but as a positive counter, indicates time passed
		
		SC_HAS_PROCESS(Reaction);

		Reaction(sc_module_name name);
		~Reaction();

		typedef enum {READY, IN_REACTION} REAC_STATES;
		sc_signal<REAC_STATES> STATUS;
		
		sc_signal<REAC_STATES> PREV_STATUS;		//store previous status to sync with testbench, makes reaction wait in ready state to let testbench update the chosen reaction signal


		int					     t_start;	    //keep track of passed time in reaction
		bool					 all_available; //keep track of available metabolites
		bool					 any_inh;		//keep track of possible inhibitions

		virtual void fsm(void) = 0;
		virtual void datapath(void) = 0;

		virtual bool check_availability(void) = 0;
		virtual bool check_inhibitions(void) = 0;

//		void update_signals_for_gtk(void);

};

#endif
