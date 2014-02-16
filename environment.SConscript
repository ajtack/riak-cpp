import os
import sys

platform_buildscript = 'buildscripts' + os.sep + sys.platform + '.SConscript'
if os.path.exists(platform_buildscript):
	print "Building for platform " + sys.platform

	# Only has effect under win32, obviously.
	AddOption('--with-msvc-version',
	          dest='msvc_version',
	          default=Environment()['MSVC_VERSION'] if 'MSVC_VERSION' in Environment() else None,
	          type='choice',
	          choices=['10.0', '11.0', '12.0'],
	          metavar='VERSION')

	# Has no effect yet on platforms other than win32, but the documentation says it may eventually.
	AddOption('--address-model',
	          dest='address_model',
	          default=Environment()['HOST_ARCH'],
	          type='choice',
	          choices=['x86', 'amd64'],
	          metavar='ARCH')

	basic_env = Environment(
	        MSVC_VERSION = GetOption('msvc_version'),
	        TARGET_ARCH = GetOption('address_model'),
	        ENV = os.environ
		)

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
