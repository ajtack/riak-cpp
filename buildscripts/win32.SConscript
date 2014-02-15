import glob
import os
Import('env')

def EnvironmentSearchPathOrDefault(comm, environment_vars, potential_paths, default_path):
	"""
	Searches the system environment for any of envoronment_vars and returns the path keyed
	by the first match. If none of those match, then potential_paths are searched until an
	existing directory is found. If nothing matches, defaults to default_path.

	For more flexible queries, potential_paths is interpreted as a glob path.

	The parameter comm should describe what kind of search path this is.

	"""
	def most_likely_path():
		viable_paths = [os.environ[var] for var in environment_vars if var in os.environ]
		if not viable_paths:
			viable_paths = [path
					for potential_path in potential_paths
					for matches in glob.glob(potential_path)
					for path in ([matches] if isinstance(matches, str) or isinstance(matches, unicode) else matches)]

			if not viable_paths:
				return default_path
			else:
				return viable_paths[0]
		else:
			return viable_paths[0]

	likely_path = most_likely_path()
	print "Detected " + comm + " search path '" + likely_path + "'"
	return likely_path

#
# The inventors of the win32 header include system should be ashamed. This will likely need
# to be adjusted for situations I have not foreseen.
#
boost_header_search_path = EnvironmentSearchPathOrDefault( 'header',
		['BOOST_ROOT'],
		[r'C:\Program Files*\boost_[1-9]_[5-9][4-9]_*', r'C:\Program Files*\boost'],
		r'C:\boost'
	)
boost_library_search_path = EnvironmentSearchPathOrDefault( 'library',
		[],
		[boost_header_search_path + r'\lib', boost_header_search_path + r'\stage\lib'],
		r'C:\boost\stage\lib'
	)

protobuf_header_search_path = EnvironmentSearchPathOrDefault( 'header',
		['PROTOBUFROOT', 'PROTOBUF_ROOT'],
		[r'C:\Program Files*\google\include'],
		r'C:\Program Files\google\include'
	)
protobuf_library_search_path = EnvironmentSearchPathOrDefault( 'library',
		['PROTOBUF_ROOT'],
		[r'C:\Program Files*\google\lib'],
		r'C:\Program Files\google\include'
	)


common_win_env = env.Clone()
common_win_env.Append(
		CXXFLAGS = ['/EHsc', '/Zi', '/D_WIN32_WINNT=0x0501', '/Gm',
		            '/Dnot="!"', '/Dand="&&"', '/Dor="||"', '/D_VARIADIC_MAX=10'],
		CPPPATH = [boost_header_search_path, protobuf_header_search_path],
		LIBPATH = [boost_library_search_path, protobuf_library_search_path],
	)

release_env = common_win_env.Clone()
release_env.Append(
		CXXFLAGS = ['/O2', '/MD'],
		LIBS = ['libprotobuf']
	)

debug_env = common_win_env.Clone()
debug_env.Append(
		CXXFLAGS = ['/MDd', '/D_SCL_SECURE_NO_WARNINGS', '/WX'],
		LINKFLAGS = ['/DEBUG']
		# Configure libprotobuf as below...
	)

# It is a weak convention to use a 'd' suffix to differentiate the debug version of a library.
# We will try to be tolerant of non-believers.
#
debug_conf = Configure(debug_env)
if debug_conf.CheckLib('libprotobufd'):
	debug_env = debug_conf.Finish()
else:
	debug_env.Append(LIBS = ['libprotobuf'])

Return('release_env debug_env')
