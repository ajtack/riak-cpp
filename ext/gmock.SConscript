Import('env')
build_path = '#build/ext'

gmock_env = env.Clone()
gmock_env.Append(
	CPPPATH = [
		'gmock-1.7.0',
		'gmock-1.7.0/include',
		'gmock-1.7.0/gtest/include'
		],
	)

gmock_root = 'gmock-1.7.0/'
gmock_sources = [
		gmock_root + 'src/gmock-all.cc'
	]

gmock = gmock_env.StaticLibrary('gmock', gmock_sources)

Return('gmock')
