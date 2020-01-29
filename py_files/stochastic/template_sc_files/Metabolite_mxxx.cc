#include "Metabolite_mxxx.hh"

//implementation of a specific metabolite
Metabolite_mxxx::Metabolite_mxxx(sc_module_name name) : Metabolite(name) {

	SC_METHOD(datapath);
	sensitive << t;

	SC_METHOD(fsm);
	sensitive << conc_to_tb << t; //in this way only when concentration in a clock time is defined status are updated, t now is used to ensure to have print like before and to have an additional print only when conc_to_tb is necessary
	
}

Metabolite_mxxx::~Metabolite_mxxx() {
}

int Metabolite_mxxx::get_num_input_reac(void) {
	//cout<<std::string(this->name())<<Metabolite_mxxx::NUM_INPUT_R<<endl;
	return Metabolite_mxxx::NUM_INPUT_R;
}

int Metabolite_mxxx::get_num_output_reac(void) {
	//cout<<std::string(this->name())<<Metabolite_mxxx::NUM_OUTPUT_R<<endl;
	return Metabolite_mxxx::NUM_OUTPUT_R;
}

void Metabolite_mxxx::datapath(void) {

	if (params_ready.read() == 0) {
		DEBUG_MSG("\t" <<sc_time_stamp()<< " - " + std::string(this->name()) + ": params not ready");
		first_time.write(true);

		next_trigger(5, SC_NS);									//wait until params are all ready

	} else {

		if(first_time.read() == true) {
			concentration = initial_concentration.read();
			first_time.write(false);
			DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": init concentration: "<< concentration);
		}

		//increase concentration from reaction in which met is a product
		for (int i = 0; i < NUM_INPUT_R; i++) {
			if (reaction_inc[i].read() == DONE) {
				//if (get_num_output_reac() == 0) 
					concentration = concentration + m_zero_as_product[i].read();
				DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": concentration increased: "<< concentration);
				break;
			}
		}
		//done_out_reaction = 0; //used for warnings (useless)

		for (int i = 0; i < NUM_OUTPUT_R; i++) {

			switch(STATUS[i]){

			case NOT_AVAILABLE:
				DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + " " + std::to_string(i) + ": not available");

				if (concentration >= threshold[i].read()) {
					send_available[i].write(AVAILABLE);
					send_available_to_tb[i].write(true);
				} else {
					send_available[i].write(NOT_AVAILABLE);
					send_available_to_tb[i].write(false);
				}
				break;

			case AVAILABLE:
				DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + " " + std::to_string(i) + ": available");

				send_available[i].write(AVAILABLE);

				if (reaction_dec[i].read() == DONE) {					 //waiting for reaction delay
					/*****************
					warning about resource conflict between output reactions
					*****************/
					/*
					done_out_reaction++;
					if (done_out_reaction > 1) {
						DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nWARNING! Possible resource conflict in " + std::string(this->name()) + "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
						//concentration = 0;
						//next_trigger(50, SC_NS);
					}
					*/
					/*****************
					end warning: useless now
					*****************/
					if (get_num_input_reac() != 0)
						concentration = concentration - m_zero_as_reactant[i].read();
					DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": concentration decreased: "<< concentration);
				}

				//warning about resource conflict between output reactions
				if (concentration < threshold[i].read()) {//suppose that threshold is positive
					send_available[i].write(NOT_AVAILABLE);
					send_available_to_tb[i].write(false);
					if(concentration < 0) {
						cout<<"\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": ERROR! NEGATIVE CONCENTRATION "<< concentration<<endl;
						concentration = 0;
					}
				}
				break;
			}
		}
		conc_to_tb.write(concentration); //here to send to tb also concentration of metabolites that are products only

		//inhibition signal (to be discussed...)
		if (concentration >= initial_concentration.read() * 30 / 100) { //as assumed in paper for CIBB
			for (int i = 0; i < NUM_OUTPUT_INH; i++) {
				inhibition_out[i].write(true);
			}
			not_inhibiting_sig_to_tb.write(false);
		} else {
			for (int i = 0; i < NUM_OUTPUT_INH; i++) {
				inhibition_out[i].write(false);
			}
			not_inhibiting_sig_to_tb.write(true);
		}

	}
}

void Metabolite_mxxx::fsm(void) {

	if (params_ready.read() == 0) {
		DEBUG_MSG("\t" <<sc_time_stamp()<< " - " + std::string(this->name()) + ": fsm params not ready" );
		for (int i = 0; i < NUM_OUTPUT_R; i++) {
			STATUS[i] = NOT_AVAILABLE;
		}

		next_trigger(5, SC_NS);									  //wait until params are all ready

	} else {

		if(first_time.read() == true) {
			//set initial states based on concentration
			for (int i = 0; i < NUM_OUTPUT_R; i++) {
				if(concentration >= threshold[i].read()) {
					STATUS[i] = AVAILABLE;
				} else {
					STATUS[i] = NOT_AVAILABLE;
				}
				DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + " " + std::to_string(i) + ": fsm set init status: " << (STATUS[i] == 0 ? "NOT AVAILABLE" : "AVAILABLE"));
			}
		}

		for (int i = 0; i < NUM_OUTPUT_R; i++) {
			switch(STATUS[i]){

			case NOT_AVAILABLE:
				DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + " " + std::to_string(i) + ": fsm in not available");

				if (concentration >= threshold[i].read()) {
					STATUS[i] = AVAILABLE;
				}
				break;

			case AVAILABLE:

				#ifdef DEBUG_ON
				if (prev_t == t.read() && concentration < threshold[i].read()) {
					DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + " " + std::to_string(i) + ": fsm change to not available");
				} else if (prev_t == t.read() && t.read() > 0 /*to avoid print first time*/) {
					DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + " " + std::to_string(i) + ": fsm still in available");
				} else if (t.read() > 0 /*to avoid print first time*/) {
					DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + " " + std::to_string(i) + ": fsm in available");
				}
				prev_t = t.read();
				#endif

				if (concentration < threshold[i].read()) {
					STATUS[i] = NOT_AVAILABLE;
				}
				break;
			}
		}
	}
}
