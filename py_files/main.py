import os
from libsbml import *
from common_files.common_py.metabolite_generator import generateMetabolites
from common_files.common_py.reaction_generator import generateReactions
from common_files.common_py.main_generator import generateMain
from common_files.common_py.utils import set_sim_type, get_inc_path_to_generated,\
		get_src_path_to_generated, get_supported_sim_types, includeBaseFiles


def main(args):
	
	"""Usage: run main with an sbml xml file as arg and an optional sim_type arg
	"""

	if len(args) < 2:
		print("\n" + "Usage: main.py sbml_filename (optional)sim_type")
		return 1

	filename = args[1]
	document = readSBML(filename)

	if document.getNumErrors() > 0:
		print("Encountered the following SBML errors:")
		document.printErrors()
		return 1

	# TODO validator
	# check compartment and unit definitions

	level = document.getLevel()
	version = document.getVersion()

	print("\n" +
		  "File: " + filename +
		  " (Level " + str(level) + ", version " + str(version) + ")")

	model = document.getModel()
	print(model.getName())
	#get description or other data if needed

	if model is None:
		print("No model present.")
		return 1

	'delete previous generated network files'
	'header files'
	path = get_inc_path_to_generated()
	for f in os.listdir(path):
		if os.path.isfile(path + f):
			os.remove(path + f)
	'source files'
	path = get_src_path_to_generated()
	for f in os.listdir(path):
		if os.path.isfile(path + f):
			os.remove(path + f)

	if len(args) == 3 and args[2] in get_supported_sim_types():
		set_sim_type(args[2])
		print('set simulation type to: ' + args[2])
	else:
		print('use stochastic simulation as default')

	'call functions for systemc files generators'
	print('including base files...')
	includeBaseFiles()
	print('generating metabolites...')
	generateMetabolites(model)
	print('generating reactions...')
	generateReactions(model)
	if len(args) == 3 and args[2] in get_supported_sim_types()[0:2]:
		print('generating deterministic testbench...')
		from deterministic.testbench_generator import generateTestbench
		generateTestbench(model)
	else:
		print('generating stochastic testbench...')
		from stochastic.testbench_generator import generateTestbench
		generateTestbench(model)
	print('generating main file and link modules...')
	generateMain(model)


if __name__ == '__main__':
	main(sys.argv)
