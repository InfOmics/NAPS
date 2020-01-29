#include "Testbench.hh"
// using ofstream constructors.
#include <iostream>
#include <fstream>  
//used for kinetic law
#include <math.h>

#include <time.h>

int factorial(int n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

Testbench::Testbench(sc_module_name name) : sc_module(name) {

	SC_METHOD(run);
	sensitive << t;

	SC_THREAD(clk_gen);
}

Testbench::~Testbench() {
}

void Testbench::clk_gen() {

	while (true) {
		clk.write(SC_LOGIC_1);
		wait(PERIOD / 2, SC_NS);
		clk.write(SC_LOGIC_0);
		wait(PERIOD / 2, SC_NS);
		
		if(reset) {
			wait(2 * PERIOD, SC_NS);
			t.write(0);
			reset = false;
			first_time = true;
			cout<<"RESTARTING"<<endl;
			
		} else {
			t.write(t.read() + 1);
		}
	}
}

bool Testbench::abv_checker(double *init_conc, double *delta_up, double *delta_down) { //met_conc is public
	for (int i = 0; i < NUM_METABOLITES; i++) {
		
		if (met_conc[i].read() < init_conc[i] - init_conc[i] * delta_down[i] / 100) {
			cout<<"down "<<i<<endl;
			return false;
		}
		if (met_conc[i].read() > init_conc[i] + init_conc[i] * delta_up[i] / 100) {
			cout<<"up "<<i<<endl;
			return false;
		}
	}
	return true;
}


int Testbench::update_reac_time(double *reac_time, double *reac_rate_const, bool reset = false) {
	//(solved) segmentation fault on network with less than 2 reactions 
	// because try to access to the second element
	// but still segmentation fault and message when no reactions detected
	
	if(reset) {
		time_passed.write(0);
		return 0;
	}

	//call abv, pass concentration for fitness and reset simulation if changed
	int k;
	min_propensity = SIM_DURATION + 1;
	if (NUM_REACTIONS > 1) {
		for(int i = 0; i < NUM_REACTIONS; i++) {
			reaction_time[i].write(SIM_DURATION); //to block reaction

			double propensity_i = reac_time[i] * reac_rate_const[i] * 1.0;
			double r = 1.0 * (rand()/(double)RAND_MAX); //always the same in each iteration
			if (1.0 * reac_time[i] == 0.0) {
				tau_i = -1;
			} else {
				tau_i = ((1.0 / propensity_i) * log(1.0 / r)) * 999 + 1; //c discretization constant
				//tau_i = (1.0 / propensity_i) * log(1.0 / r);
			}

			if (tau_i >= 0 && tau_i < min_propensity) {
				min_propensity_index = i;
				min_propensity = tau_i;

				possible_reaction[0] = i;
				for(int j = 1; j < NUM_REACTIONS; j++) {
					possible_reaction[j] = -1;
				}
			} else if (tau_i >= 0 && tau_i == min_propensity) {
				for (k = 0; k < NUM_REACTIONS; k++) { //should always be something in index 0
					if (possible_reaction[k] == -1) {
						break;
					}
				}
				possible_reaction[k] = i;
			}
		}
		for (k = 0; k < NUM_REACTIONS; k++) { //should always be something in index 0
			if (possible_reaction[k] == -1) { //useless to check max because all should be equal
				break;
			}
		}
		int chosen_index = rand() % k;
		min_propensity_index = possible_reaction[chosen_index];
	} else {
		min_propensity_index = 0;
		if (NUM_REACTIONS < 1) {
			cout<<"\n\nUnable to detect any reaction; possible segmentation_fault\n\n"<<endl;
		}
	}

	//when metabolites are all not available min_propensity is SIM_DURATION + 1 so we reset the value to 1
	if (min_propensity == SIM_DURATION + 1){
		min_propensity = 1;
	}

	//to update simulation time passed with the time for the chosen reaction
	time_passed.write(time_passed.read() + min_propensity);

	//comment prev line and uncomment following line to plot correctly
	//time_p += min_propensity;

	return min_propensity_index;
}

void Testbench::run() {

	//to print info
	const char* met_names[] = {};
	const char* reac_names[] = {};

	//listOfCompartments

	//listOfParameters

	double init_conc[] = {};
	double thr[] = {};
	//is called reac_time (also the signal and the port) but is more like a score
	double reac_time[] = {};
	//to be modified by abv, start all with 1?
	double reac_rate_const[] = {};
	
	//take deltas
	double delta_up[] = {50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0};
	double delta_down[] = {50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0};

	//code file pointer (all code can be in a separate function because in tb run conc are only read so no problem like fsm and datapath)

	bool any_available;

	if(first_time) {//if firstfirst time
		srand((unsigned)time(NULL));

		any_available = true;
		first_time = false;
//		DEBUG_MSG("\t" <<sc_time_stamp()<< " - tb: begin run()");
		for(int i = 0; i < NUM_METABOLITES; i++) {
			params_ready_m[i].write(0);
		}
		for(int i = 0; i < NUM_REACTIONS; i++) {
			params_ready_r[i].write(0);
		}

		double m_z_reac[] = {};
		double m_z_prod[] = {};

		for(int i = 0; i < NUM_METABOLITES; i++) {
			initial_concentration[i].write(init_conc[i]);
			prev_met_conc[i] = init_conc[i];
		}
		for(int i = 0; i < NUM_REACTIONS; i++) {
			possible_reaction[i] = -1;
		}

		for(int i = 0; i < NUM_INTERACTIONS_PRODUCTS; i++) {
			m_zero_prod[i].write(m_z_prod[i]);
		}
		for(int i = 0; i < NUM_INTERACTIONS_REACTANTS; i++) {
			m_zero_reac[i].write(m_z_reac[i]);
		}
		for(int i = 0; i < NUM_INTERACTIONS_REACTANTS; i++) {
			threshold[i].write(thr[i]); //threshold is equal to m_zero_reac for now
		}

		for(int i = 0; i < NUM_METABOLITES; i++) {
			params_ready_m[i].write(1);
		}
		for(int i = 0; i < NUM_REACTIONS; i++) {
			params_ready_r[i].write(1);
		}

		//code for writing concentration first time on text file to visualize on a graph
		outfile = fopen("../met_concentrations.txt", "w");
		if (!outfile) {
			cout << "Error Opening File" << endl;
			sc_stop();
		}
		fprintf(outfile,"steps, time, ");
		for(int i = 0; i < NUM_METABOLITES; i++) {
			fprintf(outfile, "%s, ", met_names[i]);
		}
		fprintf(outfile, "\n%d, ", (int) t.read());
		fprintf(outfile, "%d, ", (int) time_passed.read());

		//comment prev line and uncomment following line to plot time correctly
		//fprintf(outfile, "%f, ", time_p);

		for(int i = 0; i < NUM_METABOLITES; i++) {
			fprintf(outfile,"%d, ", (int) init_conc[i]);
		}
		fprintf(outfile,"\n");
		fclose(outfile);
		//reopen for append next steps
		outfile = fopen("../met_concentrations.txt", "a");

	} else {
		any_available = false;
		//check if dead state
		/*
		bool all_equal = true;
		for(int i = 0; i < NUM_METABOLITES; i++) {
			if(met_conc[i].read() != prev_met_conc[i]) {
				all_equal = false;
				//break; //if break needs another for loop below
			}
			prev_met_conc[i] = met_conc[i].read();
		}
		
		if (all_equal) {
			dead_state_counter++;
		} else {
			dead_state_counter = 0;
		}
		*/
		for(int i = 0; i < NUM_INTERACTIONS_REACTANTS; i++) {
			if(met_available[i] == true) {
				any_available = true;
			}
		}
		if (!any_available) {
			dead_state_counter++;
		} else {
			dead_state_counter = 0;
		}
		//exit condition at the end (if here can avoid following computation)

		//start update using kinetic law//end update using kinetic law

		//start update using propensity//end update using propensity

		min_propensity_index = update_reac_time(reac_time, reac_rate_const);
		reaction_time[min_propensity_index].write(0); //to unlock the execution of the reaction with the max score
//		DEBUG_MSG("chosen reaction at time " << t.read() << ": " << reac_names[min_propensity_index] << "\n");

		//code for writing concentration on text file to visualize on a graph
		if (!outfile) {
			cout << "Error Opening File" << endl;
		}
		fprintf(outfile, "%d, ", (int) t.read());
		fprintf(outfile, "%d, ", (int) time_passed.read());

		//comment prev line and uncomment following line to plot time correctly
		//fprintf(outfile, "%f, ", time_p);

		for(int i = 0; i < NUM_METABOLITES; i++) {
			fprintf(outfile, "%d, ", (int) met_conc[i].read());
		}
		fprintf(outfile,"\n");
		
		/*
		//call abv checker function
		if (t.read() % 100 == 0) {
			if (abv_checker(init_conc, delta_up, delta_down) != true) {
				//call modify parameters
				//sc_stop(); with error?
				cout<<t.read()<<"outrange"<<endl;
				//reset used in tb
				reset = true;
				//reset met and reaction to avoid read done signals
				for(int i = 0; i < NUM_METABOLITES; i++) {
					params_ready_m[i].write(0);
				}
				for(int i = 0; i < NUM_REACTIONS; i++) {
					params_ready_r[i].write(0);
				}
				
				//reset time_passed
				update_reac_time(reac_time, reac_rate_const, true);
				
				//close file because of useless trends
				fclose(outfile);
			}
			//otherwise ok and continue simulation
		}
		*/
	}
//end condition
	if (t.read() * 10 >= SIM_DURATION || dead_state_counter >= 5) {
		for(int i = 0; i < NUM_METABOLITES; i++) {
			params_ready_m[i].write(0);
		}
		for(int i = 0; i < NUM_REACTIONS; i++) {
			params_ready_r[i].write(0);
		}
		fclose(outfile);
		sc_stop();
	}
}
