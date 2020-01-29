from libsbml import *
from common_files.common_py.utils import substituteEnum, createEnum, getTemplateFile, \
	 generateFromTemplate, removeEndComma, getIdName, getInitialQty, \
	 get_src_path_to_generated, get_inc_path_to_generated
import re #for correct substitution of met names
import math

def createInteractionEnum(listOf, r_id, id_before):
	enum = ""
	for s in listOf:
		if id_before:
			interaction = r_id + "_" + s
		else:
			interaction = s + "_" + r_id
		interaction += ",\n					  "
		enum += interaction
	return enum


def getCompartments(model): #check again and integrate name or id?
	i = 0
	compartments = {}
	while model.getCompartment(i) is not None:
		compartments.update({getIdName(model.getCompartment(i)) : model.getCompartment(i).getSize()})#getName())
		i = i + 1
	return compartments


def getParameters(model): #check again and integrate
	params = {}
	for param in model.getListOfParameters():
		params.update({getIdName(param) : param.getValue()})
	return params


def insertPower(formula):

	#to substitute power (^) with pow function
	pow_text = "pow((int) ("
	formula = formula.split("^", 1)
	while len(formula) > 1:
		base_elem = formula[0][::-1] #reverse with slicing
		i = 0
		while base_elem[i] is ")": #count from the end closed parenthesis to know where to put pow
			i = i + 1
		if i == 0: #if no parenthesys
			base_elem = base_elem.split(" ", 1)
			base_elem.insert(1, pow_text[:-1][::-1]) #insert pow without additional parenthesys
			if len(base_elem) > 2: #there is something before pow
				base_elem[1] += " "
		else:
			base_elem = base_elem.split("(", i) #split at the corresponding block to put before pow reversed
			base_elem.insert(i, pow_text[::-1])

		base_elem = "".join(base_elem)
		base_elem = base_elem[::-1]

		formula = "(double) (" + base_elem + ", (int) (" + formula[1] + ")))"
		formula = "".join(formula)
		formula = formula.split("^", 1)
	formula = "".join(formula)

	return formula


def generateTestbenchHeader(model):

	# TODO get period, sim_duration, num sim
	period_value = 10
	sim_duration_value = 30000
	num_sim_value = 1
	'max_num_value = 32 #not used'

	'generate testbench header file'
	'get template header file and put correct data'
	template_filename = 'Testbench_template.hh'
	template_header = getTemplateFile(template_filename)

	'put correct data: sim parames and enum for description of the network'
	template_header = template_header.replace("period_value", str(period_value))
	template_header = template_header.replace("sim_duration_value", str(sim_duration_value))
	template_header = template_header.replace("num_sim_value", str(num_sim_value))

	'enum of metabolites'
	species = [getIdName(item) for item in model.getListOfSpecies()]
	template_header = createEnum(template_header, species, "NUM_METABOLITES\n")

	'enum of reactions'
	reactions = model.getListOfReactions()
	reactions_id = [getIdName(item) for item in model.getListOfReactions()]
	template_header = createEnum(template_header, reactions_id, "NUM_REACTIONS\n")

	'enum of interactions'
	product_enum = "typedef enum {"
	reactant_enum = "typedef enum {"
	inh_enum = "typedef enum {"

	for r in reactions:
		'print(r)'
		r_id = getIdName(r)

		products = [item.getSpecies() for item in r.getListOfProducts()]
		product_enum += createInteractionEnum(products, r_id, True)

		reactants = [item.getSpecies() for item in r.getListOfReactants()]
		reactant_enum += createInteractionEnum(reactants, r_id, False)

		# for now considered only inhibitors, then check SBO
		inhibitors = [item.getSpecies() for item in r.getListOfModifiers()]
		inh_enum += createInteractionEnum(inhibitors, r_id, False).replace(",", "_inh,")
																		   
	template_header = substituteEnum(template_header, product_enum, "NUM_INTERACTIONS_PRODUCTS\n")
	template_header = substituteEnum(template_header, reactant_enum, "NUM_INTERACTIONS_REACTANTS\n")
	template_header = substituteEnum(template_header, inh_enum, "NUM_INH_REACTIONS\n")

	'generate new testbench file from template'
	filename = 'Testbench.hh'
	generateFromTemplate(get_inc_path_to_generated(), filename, template_header)


def generateTestbenchCFile(model):

	'generate testbench c file'
	'get template c file and put correct data'
	template_filename = 'Testbench_template.cc'
	template_c = getTemplateFile(template_filename)

	'get parameters like initial amount, m_z, threshold'
	'get initial amount of each met'
	species = model.getListOfSpecies()
	init_conc_data = "init_conc[] = {"
	for s in species:
		init_conc_data += str(getInitialQty(s)) + ", "
	init_conc_data += "};\n"
	init_conc_data = init_conc_data.replace(", }", "}")  # maybe find a better way

	template_c = template_c.replace("init_conc[] = {};\n", init_conc_data)

	'get m_z for each interaction, in and out separately'
	reactions = model.getListOfReactions()
	prod_values = []
	reac_values = []
	products_stoichiometry = "double m_z_prod[] = {"
	reactants_stoichiometry = "double m_z_reac[] = {"
	for r in reactions:
		'print(r.getId())'

		prod_values.extend([item.getStoichiometry() for item in r.getListOfProducts()])
		reac_values.extend([item.getStoichiometry() for item in r.getListOfReactants()])

		'double check to avoid errors while sending stochiometry to metabolite ports (can be avoided)'
		# prod_values.append(-1)
		# reac_values.append(-1)
		'end of double check part of code'

	products_stoichiometry += ", ".join([str(item) for item in prod_values])
	reactants_stoichiometry += ", ".join([str(item) for item in reac_values])

	products_stoichiometry += "};"
	reactants_stoichiometry += "};"

	template_c = template_c.replace("double m_z_reac[] = {};", reactants_stoichiometry)
	template_c = template_c.replace("double m_z_prod[] = {};", products_stoichiometry)

	'get thresholds for each reactants in a reaction'
	'default: same of m_zero of reactants'
	thrs = reactants_stoichiometry
	thrs = thrs.replace(", -1", "")  # not need in this case
	thrs = thrs.replace("double m_z_reac[]", "double thr[]")
	template_c = template_c.replace("double thr[] = {};", thrs)

	
	"""
	
	'get reaction time'  # TODO need more care
	reac_time = "double reac_time[] = {"
	update_reac_time = "//start update using propensity\n		" #pay attention to spaces
	reac_rate_const = "double reac_rate_const[] = {"
	for r in reactions:
		print(formulaToL3String(r.getKineticLaw().getMath()))
		involved_met_onlyreac = [item for item in r.getListOfReactants()]
		#involved_met_onlyreac = set(involved_met_onlyreac)
		reac_rate_const += "1.0, "
		update_reac_time += "reac_time[" + getIdName(r) + "] = "
		formula = ""
		formula_init = ""
		for met in involved_met_onlyreac:
			for i in range(int(met.getStoichiometry())):
				formula += "(" + "met_conc[" + met.getSpecies() + "].read()" + " - " + str(i) + ") * "
				formula_init += "(" + "init_conc[" + met.getSpecies() + "]" + " - " + str(i) + ") * "

			last_piece = "1.0/" + str(math.factorial(met.getStoichiometry())) + " * met_available[" + met.getSpecies() + "_" + getIdName(r) + "].read()" + " * met_not_inhibiting[" + met.getSpecies() + "].read()" + " * "
			formula += last_piece
			formula_init += last_piece
		formula = formula[:-3] #remove last *
		formula_init = formula_init[:-3] #remove last *
		
		update_reac_time += formula + ";\n		"
		reac_time += formula_init + ", "
	
	reac_rate_const = removeEndComma(reac_rate_const)
	reac_time = removeEndComma(reac_time)
	"""
	
	'get reaction time'  # TODO need more care
	reac_time = "double reac_time[] = {"
	update_reac_time = "//start update using kinetic law\n		" #pay attention to spaces
	reac_rate_const = "double reac_rate_const[] = {"
	for r in reactions:
		#involved_met_onlyreac = [item.getSpecies() for item in r.getListOfReactants()]
		involved_met_onlyreac = [item for item in r.getListOfReactants()]
		involved_met = [item.getSpecies() for item in r.getListOfProducts()]
		#involved_met.extend([item.getSpecies() for item in r.getListOfModifiers()])
		#involved_met_onlyreac = set(involved_met_onlyreac)
		involved_met = set(involved_met)
		reac_rate_const += "1.0, "
		# use get formula instead of get math for sbml level 1
		# print(formulaToL3String(r.getKineticLaw().getMath()))
		form_from_kl = formulaToL3String(r.getKineticLaw().getMath())

		if "^" in form_from_kl:
			form_from_kl = insertPower(form_from_kl)

		
		#'a = r.getKineticLaw().getMath().isCiNumber()'
		#'print(obj.__class__.__name__)'
		
		update_formula = "reac_time[" + getIdName(r) + "] = " + form_from_kl
		#
		for met in involved_met_onlyreac:
			update_formula = re.sub(r"\b%s\b" % met.getSpecies(), "met_conc[" + met.getSpecies() + "].read() * met_available[" + met.getSpecies() + "_" + getIdName(r) +"].read()", update_formula)
			form_from_kl = re.sub(r"\b%s\b" % met.getSpecies(), "init_conc[" + met.getSpecies() + "] * met_available[" + met.getSpecies() + "_" + getIdName(r) +"].read()", form_from_kl)
		#
		"""
		for met in involved_met_onlyreac:
			formula = "("
			formula_init = "("
			for i in range(int(met.getStoichiometry())):
				formula += "(" + "met_conc[" + met.getSpecies() + "].read()" + " - " + str(i) + ") * "
				formula_init += "(" + "init_conc[" + met.getSpecies() + "]" + " - " + str(i) + ") * "

			#formula = formula[:-3] #remove last *
			#formula_init = formula_init[:-3] #remove last *

			update_formula = re.sub(r"\b%s\b" % met.getSpecies(), formula, update_formula)
			form_from_kl = re.sub(r"\b%s\b" % met.getSpecies(), formula_init, form_from_kl)			
        """
#remove last
			
		
		for met in involved_met:
			update_formula = re.sub(r"\b%s\b" % met, "met_conc[" + met + "].read()", update_formula)
			form_from_kl = re.sub(r"\b%s\b" % met, "init_conc[" + met + "]", form_from_kl)
			
		reac_time += form_from_kl + ", "
		update_reac_time += update_formula + ";\n		"   #pay attention to spaces
		
	reac_rate_const = removeEndComma(reac_rate_const)
	reac_time = removeEndComma(reac_time)
	
	
	template_c = template_c.replace("double reac_time[] = {};", reac_time)
	#template_c = template_c.replace("//start update using kinetic law", update_reac_time)
	template_c = template_c.replace("//start update using propensity", update_reac_time)
	
	template_c = template_c.replace("double reac_rate_const[] = {};", reac_rate_const)

	'get met and reac names and write to testbench to print info'
	met_names = "const char* met_names[] = {"
	for s in species:
		met_names += "\"" + getIdName(s) + "\", "
	met_names = removeEndComma(met_names)
	template_c = template_c.replace("const char* met_names[] = {};", met_names)

	reac_names = "const char* reac_names[] = {"
	for r in reactions:
		reac_names += "\"" + getIdName(r) + "\", "
	reac_names = removeEndComma(reac_names)
	template_c = template_c.replace("const char* reac_names[] = {};", reac_names)

	'get list of compartments and parameters for reactions'
	compartments = getCompartments(model)
	if compartments:
		compartments = ";\n	double ".join(" = ".join((str(k),str(v))) for k,v in compartments.items())
		compartments = "//listOfCompartments\n	double " + compartments + ";\n"
		template_c = template_c.replace("//listOfCompartments\n", compartments)
	
	params = getParameters(model)
	if params:
		params = ";\n	double ".join(" = ".join((str(k),str(v))) for k,v in params.items())
		params = "//listOfParameters\n	double " + params + ";\n"
		template_c = template_c.replace("//listOfParameters\n", params)

	'generate new testbench file from template'
	filename = 'Testbench.cc'
	generateFromTemplate(get_src_path_to_generated(), filename, template_c)


def generateTestbench(model):
	generateTestbenchHeader(model)
	generateTestbenchCFile(model)
