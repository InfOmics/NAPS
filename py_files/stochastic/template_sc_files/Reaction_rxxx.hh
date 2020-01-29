#ifndef Reaction_rxxx_H
#define Reaction_rxxx_H

#include "Reaction.hh"

//definition of a specific reaction

class Reaction_rxxx : public Reaction {

	public:

		//different input metabolites
		typedef enum {
					  NUM_INPUT_M
		} input_metabolites;

		//output metabolites
		typedef enum {
					  NUM_OUTPUT_M
		} output_metabolites;

		//different input inhibition
		typedef enum {
					  NUM_INPUT_INH
		} input_inhibitions;

		//topological inputs
		sc_in<MET_STATES>				available[NUM_INPUT_M];			//indicates signals from metabolites in input to reaction
		sc_in<bool>					    inhibition_in[NUM_INPUT_INH];   //input signals of inhibition from different metabolites

		//topological outputs
		sc_out<REAC_SIGNALS>			next_done[NUM_OUTPUT_M];		//indicates metabolites in output to reaction. It is sent to the next metabolite
		sc_out<REAC_SIGNALS>			prev_done[NUM_INPUT_M];		    //indicates metabolites in input to reaction. It is sent to the previous metabolites

		SC_HAS_PROCESS(Reaction_rxxx);

		Reaction_rxxx(sc_module_name name);
		~Reaction_rxxx();
		
		bool check_availability(void);
		bool check_inhibitions(void);

	private:

		void fsm(void);
		void datapath(void);
};

#endif
