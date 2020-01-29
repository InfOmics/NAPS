#ifndef Testbench_H
#define Testbench_H

#define PERIOD period_value
#define SIM_DURATION sim_duration_value
#define NUM_SIMULATIONS num_sim_value
//#define MAX_NUM 32

#include "systemc.h"

SC_MODULE(Testbench) {

	private:

		sc_signal<sc_logic> clk;

		void run();
		void clk_gen();

	public:

		//different metabolites in the network
		typedef enum {
					  NUM_METABOLITES
		} metabolites;

		//reactions between metabolites: description of the network
		typedef enum {
					  NUM_REACTIONS
		} reactions;

		//interactions between reactions and metabolites, description of the network: list of products
		typedef enum {
					  NUM_INTERACTIONS_PRODUCTS
		} interactions_products;

		//interactions between metabolites and reactions, description of the network: list of reactants
		typedef enum {
					  NUM_INTERACTIONS_REACTANTS
		} interactions_reactants;

		//reactions with inhibitions
		typedef enum {
					  NUM_INH_REACTIONS
		} inhibited_reactions;

		//parameters
		sc_out<sc_uint<32> >	initial_concentration[NUM_METABOLITES];	//initial concentration of each metabolite
		sc_out<sc_uint<32> >	m_zero_reac[NUM_INTERACTIONS_REACTANTS];//decrement quantity
		sc_out<sc_uint<32> >	m_zero_prod[NUM_INTERACTIONS_PRODUCTS];	//increment quantity
		
		sc_out<sc_uint<32> >	threshold[NUM_INTERACTIONS_REACTANTS];	//min quantity for reaction (consumption) to happen
		sc_out<sc_uint<32> >	reaction_time[NUM_REACTIONS];			//time needed for reaction

		sc_out<bool>			params_ready_m[NUM_METABOLITES];
		sc_out<bool>			params_ready_r[NUM_REACTIONS];

		sc_out<sc_uint<32> >	t;										//indicates the steps passed

		sc_signal<sc_uint<32> > time_passed;							//indicates the real time passed

		sc_in<sc_uint<32> >	 met_conc[NUM_METABOLITES];					//get concentration of each metabolite to compute SCORE for each reaction to choose reaction to fire
		sc_in<bool>			 met_available[NUM_INTERACTIONS_REACTANTS]; //get met availability for each reaction to allow tb to choose only feasible reactions
		sc_in<bool>			 met_not_inhibiting[NUM_METABOLITES];		//indicates if a met is inhibiting reactions

		bool first_time = true;
		
		bool reset = false;

		int dead_state_counter = 0;				 //(useless?)			//used for dead state
		double prev_met_conc[NUM_METABOLITES];							//used for dead state

		int possible_reaction[NUM_REACTIONS];							//to keep track of indexes of reaction with (same) max score

		int min_propensity;
		int min_propensity_index = -1;									//both used to keep track of best reactions

		int tau_i;														//indicates the time of the chosen (min) reaction

		//comment prev line and min_propensity and uncomment following lines when you want to plot exact times
		//double tau_i;
		//double time_p = 0.0;
		//double min_propensity;
		
		FILE * outfile;            //used for saving metabolites concentration csv file.
		//double *delta_up;         //used to store delta upper ranges
		//double *delta_down;       //used to store delta lower ranges

		SC_HAS_PROCESS(Testbench);
		Testbench(sc_module_name name);
		~Testbench();

		//utils function
		int update_reac_time(double *, double *, bool);
		bool abv_checker(double *init_conc, double *delta_up, double *delta_down);
};

#endif
