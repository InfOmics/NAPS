from libsbml import *
from .utils import getTemplateFile, generateFromTemplate, getIdName, \
		get_main_template_path, get_src_path_to_generated


def generateIncludeHeaders(model, template_main):

	'add include of specific metabolite headers'
	met_include = ['#include "Metabolite_' + str(getIdName(i)) + '.hh"' for i in model.getListOfSpecies()]
	met_include = '\n'.join(met_include)

	template_main = template_main.replace("#include \"Metabolite_mxxx.hh\"", met_include)

	'add include of specific reaction headers'
	reac_include = ['#include "Reaction_' + str(getIdName(i)) + '.hh"' for i in model.getListOfReactions()]
	reac_include = '\n'.join(reac_include)

	template_main = template_main.replace("#include \"Reaction_rxxx.hh\"", reac_include)
	return template_main


def generateInstances(model, template_main):

	'add instantiation of specific metabolites'
	met_instances = ['\tMetabolite_' + str(getIdName(i)) + '\tmetabolite_' + str(getIdName(i)) + '("metabolite_' + str(getIdName(i)) + '");' for i in model.getListOfSpecies()]
	met_instances = '\n'.join(met_instances)

	template_main = template_main.replace("	Metabolite_mxxx_class	metabolite_mxxx(\"metabolite_mxxx\");", met_instances)

	'add instantiation of specific reaction'
	reac_instances = ['\tReaction_' + str(getIdName(i)) + '\treaction_' + str(getIdName(i)) + '("reaction_' + str(getIdName(i)) + '");' for i in model.getListOfReactions()]
	reac_instances = '\n'.join(reac_instances)

	template_main = template_main.replace("	Reaction_rxxx_class		reaction_rxxx(\"reaction_rxxx\");", reac_instances)
	return template_main


def generateCodeTracePart(listOfSpecies, listOfReactions, template_code, template_main, start_delim, end_delim, xxx_element):

	'get dinamically lines of template code from main template file'
	line_num = 0
	start, end, start_bind_input, start_bind_output = -1, -1, -1, -1
	for line in template_code:
		if line.find(start_delim) >= 0:
			start = line_num
		if line.find(end_delim) >= 0:
			end = line_num
		line_num += 1
	# error if one not found (-1)
	template_code = template_code[start + 2 : end]
	base_code = ''.join(template_code)
	line_num = 0
	for line in template_code:
		if line.find("binding input") >= 0 or line.find("trace input") >= 0: 
			start_bind_input = line_num
		if line.find("binding output") >= 0 or line.find("trace output") >= 0: 
			start_bind_output = line_num
		line_num += 1
	# error if one not found (-1)
	tb_bind_code = ''.join(template_code[0 : start_bind_input])
	base_code_input = ''.join(template_code[start_bind_input : start_bind_output])
	base_code_output = ''.join(template_code[start_bind_output : len(template_code)])
	generated_code = ''

	'based on value of xxx_element generate code for metabolites or reactions'
	if xxx_element is 'mxxx':
		for met in listOfSpecies:
			single_met_code = tb_bind_code
			'get all reactions in which a met m is involved'
			for reaction in listOfReactions:
				if reaction.getReactant(getIdName(met)) is not None:
					single_met_code += base_code_output.replace('rxxx', getIdName(reaction))
				if reaction.getProduct(getIdName(met)) is not None:
					single_met_code += base_code_input.replace('rxxx', getIdName(reaction))
			generated_code += single_met_code.replace(xxx_element, getIdName(met))

	elif xxx_element is 'rxxx':
		for r in listOfReactions:
			single_reaction_code = tb_bind_code
			for reactant in r.getListOfReactants():
				single_reaction_code += base_code_input.replace('mxxx', reactant.getSpecies())
			for product in r.getListOfProducts():
				single_reaction_code += base_code_output.replace('mxxx', product.getSpecies())
			generated_code += single_reaction_code.replace(xxx_element, getIdName(r))

	template_main = template_main.replace(base_code, generated_code)
	return template_main


def generateCodeTraceInhPart(listOfSpecies, listOfReactions, template_code, template_main, start_delim, end_delim):

	'get dinamically lines of template code from main template file'
	line_num = 0
	start, end, start_bind_input, start_bind_output = -1, -1, -1, -1
	for line in template_code:
		if line.find(start_delim) >= 0:
			start = line_num
		if line.find(end_delim) >= 0:
			end = line_num
		line_num += 1
	# error if one not found (-1)
	template_code = template_code[start + 2 : end]
	base_code = ''.join(template_code)
	"""
	line_num = 0
	for line in template_code:
		if line.find("binding input") >= 0: 
			start_bind_input = line_num
		if line.find("binding output") >= 0: 
			start_bind_output = line_num
		line_num += 1
	# error if one not found (-1)

	base_code_input = ''.join(template_code[start_bind_input : start_bind_output])
	base_code_output = ''.join(template_code[start_bind_output : len(template_code)])
	"""
	tb_bind_code = ''.join(template_code[0 : start_bind_input])
	generated_code = ''
	single_met_code = ''

	for reac in listOfReactions:
		reac_inh = reac.getListOfModifiers()
		if reac_inh:
			for modif in reac_inh:
				single_met_code += (tb_bind_code.replace('rxxx', getIdName(reac))).replace('mxxx', modif.getSpecies())
			generated_code += single_met_code
			single_met_code = ''

	template_main = template_main.replace(base_code, generated_code)
	return template_main


def generateBindingTraceCode(model, template_main):

	'get template code for binding metabolites and reaction modules'
	'get template main file on a different way to get template code and put correct data'
	template_path = get_main_template_path()
	template_filename = 'main_template.cc'
	with open(template_path + template_filename, 'r') as template_file:
		template_code = template_file.readlines()  # in this case different from the others, not need to be closed

	species = model.getListOfSpecies()
	reactions = model.getListOfReactions()

	# pay attention to put met and reaction delim code alternated
	delim_regions = [('//start met_code', '//end met_code'),
					 ('//start reac_code', '//end reac_code'),
					 ('//start met_trace', '//end met_trace'),
					 ('//start reac_trace', '//end reac_trace')]

	delim_regions_inh = [('//start met_inh', '//end met_inh'),
						 ('//start reac_inh', '//end reac_inh'),
						 ('//start trace_met_inh', '//end trace_met_inh'),
						 ('//start trace_reac_inh', '//end trace_reac_inh')]

	for i in range(len(delim_regions)):
		template_main = generateCodeTracePart(species, reactions,
												template_code, template_main,
												delim_regions[i][0], delim_regions[i][1],
												'mxxx' if i % 2 == 0 else 'rxxx')

	for i in range(len(delim_regions_inh)):
		template_main = generateCodeTraceInhPart(species, reactions,
												template_code, template_main,
												delim_regions_inh[i][0], delim_regions_inh[i][1])

	return template_main


def generateMain(model):

	'get template main file and put correct data'
	template_path = get_main_template_path()
	template_filename = 'main_template.cc'
	template_main = getTemplateFile(template_filename)

	template_main = generateIncludeHeaders(model, template_main)
	template_main = generateInstances(model, template_main)
	template_main = generateBindingTraceCode(model, template_main)

	'generate new main file from template'
	filename = 'main.cc'
	generateFromTemplate(get_src_path_to_generated(), filename, template_main)
