import ntpath
import os
import shutil
import subprocess
import sys

def usage():
	print('mingw_deploy application [target-directory]')
	print()
	print('copy dependent mingw dlls needed by application in target-directory (default to near the application)')
	sys.exit(-1)

# main
if __name__ == "__main__":
	if len(sys.argv) < 2 or len(sys.argv) > 3:
		print('invalid argument')
		usage()
	application = sys.argv[1]
	target_directory = len(sys.argv) > 2 and sys.argv[2] or os.path.dirname(application)
	if not os.path.exists(application):
		print(application, "not found")
		exit(-1)
	
	# Retrieve dependencies from mingw
	# so the ones from $(InstallPath)\msys64\mingw32\bin\$(dependencies.dll)
	ret = subprocess.run(['cygcheck', application], capture_output=True)
	dlls = [
		line.strip() for line in ret.stdout.decode().split('\n')
		  if line.find('clang32') != -1
			or line.find('clang64') != -1
			or line.find('mingw32') != -1
			or line.find('mingw64') != -1
			or line.find('ucrt64') != -1
	]
	# copy dlls in target-directory
	if len(dlls) > 0:
		print("copy in", target_directory)
		for file in dlls:
			shutil.copyfile(ntpath.abspath(file), target_directory + '/' + ntpath.basename(file))
			print(file, "copied")
