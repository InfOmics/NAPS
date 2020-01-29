from libsbml import *
from .utils import createEnum, getTemplateFile, generateFromTemplate, \
		getIdName, get_inc_path_to_generated, get_src_path_to_generated

def generateReactionsHeaders(model):

	reactions = model.getListOfReactions()

	'get template header file and put correct data'
	template_filename = 'Reaction_rxxx.hh'
	template_file = getTemplateFile(template_filename)

	'generate header files for each reaction'
	for reac in reactions:
		'print(reac)'
		reactants = [item.getSpecies() for item in reac.getListOfReactants()]
		products = [item.getSpecies() for item in reac.getListOfProducts()]
		inhibitors = [item.getSpecies() + "_inh" for item in reac.getListOfModifiers()]
		# for now considered only inhibitors, then check SBO

		'generate reactions header file'
		template_header = template_file
		'put correct data: reaction names and enum'
		template_header = template_header.replace("rxxx", getIdName(reac))

		'enum of input metabolites'
		template_header = createEnum(template_header, reactants, "NUM_INPUT_M\n")
		'enum of output metabolites'
		template_header = createEnum(template_header, products, "NUM_OUTPUT_M\n")
		'enum of input inhibitions'
		template_header = createEnum(template_header, inhibitors, "NUM_INPUT_INH\n")

		'generate new specific reaction file from template'
		filename = 'Reaction_' + getIdName(reac) + '.hh'
		generateFromTemplate(get_inc_path_to_generated(), filename, template_header)

		# TODO reversible


def generateReactionsCFiles(model):
	
	reactions = model.getListOfReactions()

	'get template c file and put correct data'
	template_filename = 'Reaction_rxxx.cc'
	template_file = getTemplateFile(template_filename)

	'generate c files for each reaction'
	for reac in reactions:
		'print(reac)'

		'generate reactions c file'
		template_c = template_file
		'put correct data: reaction names'
		template_c = template_c.replace("rxxx", getIdName(reac))

		'generate new specific reaction file from template'
		filename = 'Reaction_' + getIdName(reac) + '.cc'
		generateFromTemplate(get_src_path_to_generated(), filename, template_c)


def generateReactions(model):
	generateReactionsHeaders(model)
	generateReactionsCFiles(model)
