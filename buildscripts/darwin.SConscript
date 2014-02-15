Import('env')

darwin_env = env.Clone()
darwin_env.Append(
		CPPPATH = ['/opt/local/include'],
		CXXFLAGS = ['--std=c++11', '-DBOOST_ALL_DYN_LINK', '-Wall', '-Werror'],
		LINKFLAGS = ['-L/opt/local/lib'],
		LIBS = ['boost_log-mt', 'boost_log_setup-mt', 'boost_thread-mt', 'pthread', 'boost_system-mt', 'protobuf']
	)

release_env = darwin_env.Clone()

debug_env = darwin_env.Clone()
debug_env.Append(
		CXXFLAGS = ['-g', '-O0']
	)

Return('release_env debug_env')
