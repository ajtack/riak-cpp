import os

VariantDir('build', 'source')
common_env = Environment(
        ENV = os.environ,
        CXXFLAGS = ['--std=c++0x'],
        CPPPATH = ['/opt/local/include', '#source'],
        LIBPATH = ['/opt/local/lib'])

if ARGUMENTS.get('VERBOSE') != 'yes':
    common_env.Append(
            CXXCOMSTR =  '(compile)  $SOURCES',
            LINKCOMSTR = '(link)     $TARGET'
    )

debug_env = common_env.Clone(CCFLAGS = ['-O0', '-g'])
sources = Glob('build/*.cxx')

if ARGUMENTS.get('DEBUG') == 'yes':
    debug_env.StaticLibrary('storage', sources)
else:
    common_env.StaticLibrary('storage', sources)
