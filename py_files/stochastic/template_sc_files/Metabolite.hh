#ifndef Metabolite_H
#define Metabolite_H

#include "systemc.h"
#include "States_Network.h"

//definition of a generic metabolite
//then for each metabolite create files metabolite_M#
SC_MODULE(Metabolite) {						//abstract class

	public:

		//parameters
		sc_in<sc_uint<32> >	  initial_concentration;
		//sc_in<sc_uint<32> >	  m_zero;			//increment or decrement quantity
		//sc_in<sc_uint<32> >	  threshold;		//min quantity for reaction (consumption) to happen

		sc_in<bool>			  params_ready;			//used to start, sync and restart simulation after out range
		sc_in<sc_uint<32> >	  t;					//like a clk but as a positive counter, indicates time passed

		sc_out<sc_uint<32> >  conc_to_tb;		    //to send current concentration to tb
		
		sc_out<bool>		  not_inhibiting_sig_to_tb;		//indicates to the tb if the met is inhibiting or not reactions, if true is not inhibiting (in this way we can use this signal like send available in tb). As true is 1 this signal is used in reaction formulas as not inhibited met, false is 0 so it invalidates the formula (as it is a product)

		SC_HAS_PROCESS(Metabolite);

		Metabolite(sc_module_name name);
		~Metabolite();

		int					  concentration;

		sc_signal<bool>		  first_time;			//to set the initial concentration passed by tb (TRUE = 1, FALSE = 0) //only the first process do this

		virtual void fsm(void) = 0;
		virtual void datapath(void) = 0;
		
		virtual int get_num_input_reac(void) = 0;
		virtual int get_num_output_reac(void) = 0;

//		void update_signals_for_gtk(void);

	protected:

		int					  done_out_reaction;	//keep track of the out reaction finished in the same instant of time, used for warnings

		int					  prev_t;				//to print out correct messages

};

#endif
