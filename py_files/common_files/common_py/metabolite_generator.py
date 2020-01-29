from libsbml import *
from .utils import createEnum, getTemplateFile, generateFromTemplate, \
		getIdName, get_inc_path_to_generated, get_src_path_to_generated


def generateMetabolitesHeaders(model):

	species = model.getListOfSpecies()
	speciesId = []

	'get template header file and put correct data'
	template_filename = 'Metabolite_mxxx.hh'
	template_file = getTemplateFile(template_filename)

	# TODO check hasOnlySubstanceUnits="true" otherwise there is a continuous
	# concentration (or density)

	'''use id to generate mets, maybe can use also names in some ways but
	not always defined'''
	for e in species:
		'print(e.getId())'
		speciesId.append(getIdName(e))

	'generate header files for each metabolite'
	for met in speciesId:
		'print(met)'
		metAsReactant = []
		metAsProduct = []
		metAsInhibitions = [] # for now considered only inhibitors, then check SBO

		'get all reactions in which a met m is involved'
		for reaction in (model.getListOfReactions()):
			if reaction.getReactant(met) is not None:
				metAsReactant.append(getIdName(reaction))
			if reaction.getProduct(met) is not None:
				metAsProduct.append(getIdName(reaction))
			if reaction.getModifier(met) is not None:
				metAsInhibitions.append(getIdName(reaction) + "_inh")

		'generate metabolite header file'
		template_header = template_file
		'put correct data: metabolite names and enum'
		template_header = template_header.replace("mxxx", met)

		'enum of input reactions'
		template_header = createEnum(template_header, metAsProduct, "NUM_INPUT_R\n")
		'enum of output reactions'
		template_header = createEnum(template_header, metAsReactant, "NUM_OUTPUT_R\n")
		'enum of output inhibitions'
		template_header = createEnum(template_header, metAsInhibitions, "NUM_OUTPUT_INH\n")

		'generate new specific metabolite file from template'
		filename = 'Metabolite_' + met + '.hh'
		generateFromTemplate(get_inc_path_to_generated(), filename, template_header)


def generateMetabolitesCFiles(model):
	
	species = model.getListOfSpecies()
	speciesId = []

	'get template c file and put correct data'
	template_filename = 'Metabolite_mxxx.cc'
	template_file = getTemplateFile(template_filename)

	'''use id to generate mets c files, maybe can use also names
	(as can be also in headers) in some ways but
	not always defined'''
	for e in species:
		'print(e.getId())'
		speciesId.append(getIdName(e))

	'generate c files for each metabolite'
	for met in speciesId:
		'print(met)'

		'generate metabolite c file'
		template_c = template_file
		'put correct data: metabolite names'
		template_c = template_c.replace("mxxx", met)

		'generate new specific metabolite file from template'
		filename = 'Metabolite_' + met + '.cc'
		generateFromTemplate(get_src_path_to_generated(), filename, template_c)


def generateMetabolites(model):
	generateMetabolitesHeaders(model)
	generateMetabolitesCFiles(model)
