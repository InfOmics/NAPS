#include "Testbench.hh"
#include "Metabolite.hh"
#include "Metabolite_mxxx.hh"
#include "Reaction.hh"
#include "Reaction_rxxx.hh"

int sc_main(int argc, char **argv) {

	sc_signal<sc_uint<32> >	t_signal;

	//signals for parameters, for metabolites and reactions
	sc_signal<sc_uint<32> >	initial_concentration_signal[Testbench::NUM_METABOLITES];
	sc_signal<sc_uint<32> >	m_zero_reac_signal[Testbench::NUM_INTERACTIONS_REACTANTS];
	sc_signal<sc_uint<32> >	m_zero_prod_signal[Testbench::NUM_INTERACTIONS_PRODUCTS];
	sc_signal<sc_uint<32> >	threshold_signal[Testbench::NUM_INTERACTIONS_REACTANTS];

	sc_signal<sc_uint<32> >	reaction_time_signal[Testbench::NUM_REACTIONS];
	
	sc_signal<sc_uint<32> >	met_conc_signal[Testbench::NUM_METABOLITES];

	sc_signal<bool>			params_ready_m[Testbench::NUM_METABOLITES];
	sc_signal<bool>			params_ready_r[Testbench::NUM_REACTIONS];

	//topological signals
	sc_signal<REAC_SIGNALS>	done_signal_prod[Testbench::NUM_INTERACTIONS_PRODUCTS];
	sc_signal<REAC_SIGNALS>	done_signal_reac[Testbench::NUM_INTERACTIONS_REACTANTS];
	sc_signal<MET_STATES>	available_signal[Testbench::NUM_INTERACTIONS_REACTANTS];
	
	sc_signal<bool>			available_signal_to_tb[Testbench::NUM_INTERACTIONS_REACTANTS];

	sc_signal<bool>			inhibition_signal[Testbench::NUM_INH_REACTIONS];
	
	sc_signal<bool>			inh_to_tb_signal[Testbench::NUM_METABOLITES];

//metabolites, reactions and testbench instances
	Testbench				tb("tb");	// testbench source module

	Metabolite_mxxx_class	metabolite_mxxx("metabolite_mxxx");

	Reaction_rxxx_class		reaction_rxxx("reaction_rxxx");

	//bind parameters signals from testbench
	tb.t(t_signal);
	for(int i = 0; i < Testbench::NUM_METABOLITES; i++) {
		tb.initial_concentration[i](initial_concentration_signal[i]);
		tb.params_ready_m[i](params_ready_m[i]);
		
		tb.met_conc[i](met_conc_signal[i]);
		tb.met_not_inhibiting[i](inh_to_tb_signal[i]);
	}
	for(int i = 0; i < Testbench::NUM_REACTIONS; i++) {
		tb.reaction_time[i](reaction_time_signal[i]);
		tb.params_ready_r[i](params_ready_r[i]);
	}
	for(int i = 0; i < Testbench::NUM_INTERACTIONS_PRODUCTS; i++) {
		tb.m_zero_prod[i](m_zero_prod_signal[i]);
	}
	for(int i = 0; i < Testbench::NUM_INTERACTIONS_REACTANTS; i++) {
		tb.m_zero_reac[i](m_zero_reac_signal[i]);

		tb.threshold[i](threshold_signal[i]); //threshold is equal for now
		
		tb.met_available[i](available_signal_to_tb[i]);
	}


	//start met_code do not change
//bind parameters and topological ports to metabolite modules
	//bind tb signals towards metabolites modules
	metabolite_mxxx.t(t_signal);
	metabolite_mxxx.params_ready(params_ready_m[Testbench::mxxx]);
	metabolite_mxxx.initial_concentration(initial_concentration_signal[Testbench::mxxx]);
	
	metabolite_mxxx.conc_to_tb(met_conc_signal[Testbench::mxxx]);
	metabolite_mxxx.not_inhibiting_sig_to_tb(inh_to_tb_signal[Testbench::mxxx]);

	//binding input signals towards (product) metabolite modules (from reaction modules)
	metabolite_mxxx.m_zero_as_product[Metabolite_mxxx::rxxx](m_zero_prod_signal[Testbench::rxxx_mxxx]);
	metabolite_mxxx.reaction_inc[Metabolite_mxxx::rxxx](done_signal_prod[Testbench::rxxx_mxxx]);
	
	//binding output signals from (reactant) metabolite modules (towards reaction modules)
	metabolite_mxxx.m_zero_as_reactant[Metabolite_mxxx::rxxx](m_zero_reac_signal[Testbench::mxxx_rxxx]);
	metabolite_mxxx.reaction_dec[Metabolite_mxxx::rxxx](done_signal_reac[Testbench::mxxx_rxxx]);
	metabolite_mxxx.send_available[Metabolite_mxxx::rxxx](available_signal[Testbench::mxxx_rxxx]);

	metabolite_mxxx.threshold[Metabolite_mxxx::rxxx](threshold_signal[Testbench::mxxx_rxxx]); //threshold is equal for now
	
	metabolite_mxxx.send_available_to_tb[Metabolite_mxxx::rxxx](available_signal_to_tb[Testbench::mxxx_rxxx]);


	//end met_code do not change


	//start reac_code do not change
//bind parameters and topological ports to reaction modules
	reaction_rxxx.t(t_signal);
	reaction_rxxx.params_ready(params_ready_r[Testbench::rxxx]);
	reaction_rxxx.reaction_time(reaction_time_signal[Testbench::rxxx]);
	
	//binding input signals towards reaction modules (from (reactant) metabolite modules)
	reaction_rxxx.available[Reaction_rxxx::mxxx](available_signal[Testbench::mxxx_rxxx]);
	reaction_rxxx.prev_done[Reaction_rxxx::mxxx](done_signal_reac[Testbench::mxxx_rxxx]);
	
	//binding output signals from reaction modules (towards (product) metabolite modules)
	reaction_rxxx.next_done[Reaction_rxxx::mxxx](done_signal_prod[Testbench::rxxx_mxxx]);


	//end reac_code do not change

	//start met_inh do not change
//bind (out) inhibitions topological port to metabolite modules
	metabolite_mxxx.inhibition_out[Metabolite_mxxx::rxxx_inh](inhibition_signal[Testbench::mxxx_rxxx_inh]);
	
	//end met_inh do not change
	
	//start reac_inh do not change
//bind (in) inhibitions topological port to reaction modules
	reaction_rxxx.inhibition_in[Reaction_rxxx::mxxx_inh](inhibition_signal[Testbench::mxxx_rxxx_inh]);
	
	//end reac_inh do not change


//tracefiles
	//sc_trace_file * fp = sc_create_vcd_trace_file("metabolite_network");
	//sc_trace(fp, t_signal, "t");

	//start met trace do not change
	
	//sc_trace(fp, metabolite_mxxx.concentration, "metabolite_mxxx.conc_mxxx");
	//end met trace do not change
	
	//start reac trace do not change
	
	//sc_trace(fp, reaction_rxxx.t_start, "reaction_rxxx.t_start_rxxx");
	//end reac trace do not change
/*
	sc_trace(fp, metabolite_M1.inhibition_in[Metabolite_M1::M4_INH], "metabolite_M1.inh_in_from_M4");
	sc_trace(fp, metabolite_M4.inhibition_out[Metabolite_M4::M1_INH], "metabolite_M4.inh_out_to_M1");
*/

	sc_start();

	//sc_close_vcd_trace_file(fp);

	return 0;
}

