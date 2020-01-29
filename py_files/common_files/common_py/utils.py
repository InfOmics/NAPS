import os # list files
from os import path # check path existence
import shutil # copy base files

supported_sim_types = ['det', 'det_thread', 'sto']
# list all supported simulation types args
sim_type = 'sto'
# used by utils function to choose correctly template files according to chosen reaction
path_to_generated = "./generated_systemc/"
# contains base path to generated systemc files
base_template_path = '/template_sc_files/'
# concat template path to base sim type folder

def set_sim_type(given_type):
	global sim_type
	sim_type = given_type


def get_sim_type():
	return sim_type


def get_supported_sim_types():
	return supported_sim_types


def get_path_to_generated():
	return path_to_generated


def get_inc_path_to_generated():
	return path_to_generated + "include/"


def get_src_path_to_generated():
	return path_to_generated  + "src/"


def substituteEnum(template, enum, to_substitute):
	enum += to_substitute
	template = template.replace("typedef enum {\n" +
								"					  " +
								to_substitute, enum)
	return template


def createEnum(template, listOf, to_substitute):
	enum = "typedef enum {"
	for s in listOf:
		enum += s
		enum += ",\n					  "
	template = substituteEnum(template, enum, to_substitute)
	return template


def getTemplateFile(template_filename):
	#common path
	if get_sim_type().startswith('det'): # get_supported_sim_types()[0]):
		template_path = './py_files/deterministic' + base_template_path
		if not path.exists(template_path + template_filename):
			if not get_sim_type().startswith('det_'): # get_supported_sim_types()[1]):
				template_path = './py_files/deterministic' + base_template_path + 'using_method/'
			else:
				template_path = './py_files/deterministic' + base_template_path + 'using_thread/'
	else:
		template_path = './py_files/stochastic' + base_template_path
	with open(template_path + template_filename, 'r') as template_file:
			template = template_file.read()  # not need to be closed
	return template


def generateFromTemplate(path, filename, template):
	writer = open(path + filename, "w")
	writer.write(template)
	writer.close()


def removeEndComma(string):
	string += "};"
	string = string.replace(", }", "}")  # maybe find a better way
	return string


def getChooseIdName(obj):
	if obj.isSetName():
		return "Id" # "Name"
	else: return "Id" # supposing that name is used instead of id also in getSpecies from reference, if not print names but use ids or directly only ids


def getIdName(obj):
	if getChooseIdName(obj) is "Id":
		return obj.getId()
	else:
		return obj.getName() # check with getSpecies because returns Id not Name


def getInitialQty(obj):
	if obj.isSetInitialAmount():
		return obj.getInitialAmount()
	else: return obj.getInitialConcentration()


def includeBaseFiles():
	src_path = './py_files/common_files/common_c/'
	for file in os.listdir(src_path):
		if str(file).endswith('c'):
			shutil.copy(src_path + file, get_src_path_to_generated())
		elif str(file).endswith('h'):
			shutil.copy(src_path + file, get_inc_path_to_generated())
	#maybe we can remove the following and create a single met base and reac base
	base_hh_files = ['Metabolite.hh', 'Reaction.hh']
	if get_sim_type().startswith('det'):
		if get_sim_type().startswith('det_t'):
			template_path = './py_files/deterministic' + base_template_path + 'using_thread/'
		else:
			template_path = './py_files/deterministic' + base_template_path + 'using_method/'
	else: template_path = './py_files/stochastic' + base_template_path
	for file in base_hh_files:
		shutil.copy(template_path + file, get_inc_path_to_generated())
		 


def get_main_template_path():
	if get_sim_type().startswith('det'): # get_supported_sim_types()[0]):
		template_path = './py_files/deterministic' + base_template_path
	else:
		template_path = './py_files/stochastic' + base_template_path
	return template_path
