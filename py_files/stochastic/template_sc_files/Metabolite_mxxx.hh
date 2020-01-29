#ifndef Metabolite_mxxx_H
#define Metabolite_mxxx_H

#include "Metabolite.hh"

//definition of a specific metabolite

class Metabolite_mxxx : public Metabolite {

	public:

		//different input reactions
		typedef enum {
					  NUM_INPUT_R
		} input_reactions;

		//different output reactions
		typedef enum {
					  NUM_OUTPUT_R
		} output_reactions;

		//different output inhibition
		typedef enum {
					  NUM_OUTPUT_INH
		} output_inhibition;

		sc_signal<MET_STATES>		  STATUS[NUM_OUTPUT_R];

		//parameters
		sc_in<sc_uint<32> >			  m_zero_as_product[NUM_INPUT_R];    //increment quantity of met in a reaction
		sc_in<sc_uint<32> >			  m_zero_as_reactant[NUM_OUTPUT_R];	 //decrement quantity of met in a reaction
		sc_in<sc_uint<32> >			  threshold[NUM_OUTPUT_R];			 //min quantity for reaction (consumption) to happen

		//topological inputs
		sc_in<REAC_SIGNALS>			  reaction_inc[NUM_INPUT_R];		 //indicates the state of input reaction of a metabolite
		sc_in<REAC_SIGNALS>			  reaction_dec[NUM_OUTPUT_R];		 //indicates the state of output reaction of a metabolite

		//topological outputs
		sc_out<MET_STATES>			  send_available[NUM_OUTPUT_R];		 //indicates output signals to reaction of a metabolite. It is sent to the next reaction
		sc_out<bool>				  inhibition_out[NUM_OUTPUT_INH];	 //output signals of inhibition to different reactions

		sc_out<bool>				  send_available_to_tb[NUM_OUTPUT_R]; //indicates output signals to reaction of a metabolite. It is sent to the testbench to allows it to choose only feasible reactions

		SC_HAS_PROCESS(Metabolite_mxxx);

		Metabolite_mxxx(sc_module_name name);
		~Metabolite_mxxx();
		
		int get_num_input_reac(void);
		int get_num_output_reac(void);

	private:

		void fsm(void);
		void datapath(void);
};

#endif
