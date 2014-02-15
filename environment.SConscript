import os
import sys

platform_buildscript = 'buildscripts' + os.sep + sys.platform + '.SConscript'
if os.path.exists(platform_buildscript):
	print "Building for platform " + sys.platform
	basic_env = Environment(ENV = os.environ)

	if ARGUMENTS.get('VERBOSE') != 'yes':
		basic_env.Append(
		        CXXCOMSTR    = '(compile)  $SOURCES',
		        LINKCOMSTR   = '(link)     $TARGET',
		        ARCOMSTR     = '(archive)  $TARGET',
		        RANLIBCOMSTR = '(ranlib)   $TARGET',
		        PROTOCCOMSTR = '(generate) $SOURCES'
		)

	release_env, debug_env = SConscript(platform_buildscript, exports={'env': basic_env})
	Return('release_env debug_env')
else:
	print "No environment construction defined for " + sys.platform
