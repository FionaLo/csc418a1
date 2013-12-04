import re
from optparse import OptionParser


if __name__ == '__main__':
	
	parser = OptionParser()
	parser.add_option("-f", "--file", dest="filename",
	                  help="input stl file name", metavar="FILE")

	(options, args) = parser.parse_args()

	output_file = open('converted.stl','w')

	with open(options.filename) as f:
		for line in f.readlines():
			if "normal" in line or "vertex" in line:
				output_file.write(' '.join(line.split()[-3:]) + '\n')

	output_file.close()

