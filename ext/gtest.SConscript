Import('env')

gtest_env = env.Clone()
gtest_env.Append(
	CPPPATH = [
		'gmock-1.7.0/gtest',
		'gmock-1.7.0/gtest/include'
		],
	)

gtest_root = 'gmock-1.7.0/gtest/'
gtest_sources = [
		gtest_root + 'src/gtest-all.cc'
	]

gtest = gtest_env.StaticLibrary('gtest', gtest_sources)

Return('gtest')
