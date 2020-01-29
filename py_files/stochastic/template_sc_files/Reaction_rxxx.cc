#include "Reaction_rxxx.hh"

//implementation of a specific Reaction
Reaction_rxxx::Reaction_rxxx(sc_module_name name) : Reaction(name) {

	SC_METHOD(datapath);
	sensitive << t;

	SC_METHOD(fsm);
	sensitive << t;
	
}

Reaction_rxxx::~Reaction_rxxx() {
}

bool Reaction_rxxx::check_availability(void) {
	all_available = true;
	for (int i = 0; i < NUM_INPUT_M; i++) {
		if (available[i].read() == NOT_AVAILABLE) {
			all_available = false;
			break;
		}
	}
	return all_available;
}

bool Reaction_rxxx::check_inhibitions(void) {
	any_inh = false;
	for (int i = 0; i < NUM_INPUT_INH; i++) {
		if (inhibition_in[i].read() == true) {
			any_inh = true;
			break;
		}
	}
	return any_inh;
}

void Reaction_rxxx::datapath(void){
	
	for (int i = 0; i < NUM_INPUT_M; i++) {
		prev_done[i].write(RESET);
	}
	for (int i = 0; i < NUM_OUTPUT_M; i++) {
		next_done[i].write(RESET);
	}

	if (params_ready.read() == 0) {
		DEBUG_MSG("\t" <<sc_time_stamp()<< " - " + std::string(this->name()) + ": params not ready");
		t_start = -1;

		next_trigger(5, SC_NS);									  //wait until params are all ready

	} else {

		all_available = check_availability();

		switch(STATUS){

		case IN_REACTION:
			DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": in reaction");

			if(reaction_time.read() <= t.read() - t_start && all_available) {
				for (int i = 0; i < NUM_INPUT_M; i++) {
					prev_done[i].write(DONE);
				}
				for (int i = 0; i < NUM_OUTPUT_M; i++) {
					next_done[i].write(DONE);
				}
				t_start = -1;
			} else if(!all_available /*or any inh if possible, ask before*/){
				t_start = -1;
			}
			break;

		case READY:
			DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": in ready");

			any_inh = check_inhibitions();
			
			if(all_available && any_inh == false) {
				t_start = t.read();
			}
			break;
		}
	}
}

void Reaction_rxxx::fsm(void){

	if(params_ready.read() == 0) {
		DEBUG_MSG("\t" <<sc_time_stamp()<< " - " + std::string(this->name()) + ": fsm params not ready");
		STATUS = READY;	   

		PREV_STATUS = READY;

		next_trigger(5, SC_NS);									  //wait until params are all ready

	} else {

		all_available = check_availability();

		switch(STATUS){

		case IN_REACTION:

			#ifdef DEBUG_ON
			if (all_available) {
				DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": fsm in reaction");
			} else {
				DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": fsm will change to ready (from in reaction)");
			}
			#endif

			//to better distinguish cases
			if(reaction_time.read() <= t.read() - t_start && all_available) {
				STATUS = READY;
			} else if(!all_available /*or any inh if possible, ask before*/){
				STATUS = READY;
			}		   
			//else {
				//cout<<"\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": fsm no reaction happens"<<endl;
				STATUS = READY;
				PREV_STATUS = IN_REACTION;
			//}
			break;

		case READY:
			DEBUG_MSG("\t"<<sc_time_stamp()<<" - "<<t.read()<< " - " + std::string(this->name()) + ": fsm in ready");

			any_inh = check_inhibitions();
			
			if(all_available && any_inh == false) {
				if(PREV_STATUS == READY) { //more readable
					STATUS = IN_REACTION;
				}
			}
			PREV_STATUS = READY;
			break;
		}
	}
}
