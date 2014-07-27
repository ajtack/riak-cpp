import commands
import os
import string

Import('env')
naked_env = env.Clone(CXX = os.environ['CXX'])

#
# G++ Uses different flags to trigger C++11 support, for historical reasons. Here,
# detect which one were appropriate for our current version, or default to --std=c++11
# if we're not using g++
#
if naked_env['CXX'].startswith('g++'):
	gxx_version = string.split(commands.getoutput(naked_env['CXX'] + ' -dumpversion'), '.')
	major, minor, sub = gxx_version[0], gxx_version[1], (gxx_version[2] if len(gxx_version) > 2 else 0)
	if major < '4':
		print "gcc 3.x is not supported"
		Exit(1)
	else:
		if minor < '4':
			print 'riak-cpp requires basic c++11 support, which is not supported by g++ v' + string.join(gxx_version, '.')
			Exit(1)
		else:
			if minor <= '6':
				cxx11_flag = '--std=c++0x'
			else:
				cxx11_flag = '--std=c++11'
else:
	cxx11_flag = '--std=c++11'

#
# Prepare the variant environments.
#
naked_env.Append(
		CPPPATH = ['/opt/local/include'],
		CXXFLAGS = [cxx11_flag, '-DBOOST_ALL_DYN_LINK', '-Wall', '-Werror'],
		LINKFLAGS = ['-L/opt/local/lib'],
		LIBS = ['boost_thread-mt', 'pthread', 'boost_system-mt', 'protobuf']
	)

if GetOption('logging_enabled') == 'yes':
	naked_env.Append(LIBS = ['boost_log-mt', 'boost_log_setup-mt']

release_env = naked_env.Clone()
debug_env = naked_env.Clone()
debug_env.Append(
		CXXFLAGS = ['-g', '-O0']
	)

Return('release_env debug_env')
