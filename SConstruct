import os

VariantDir('build/library', 'source')
common_env = Environment(
        ENV = os.environ,
        CXXFLAGS = ['--std=c++0x'],
        CPPPATH = ['/opt/local/include', '#source', '#build/library'],
        LIBPATH = ['/opt/local/lib'])

if ARGUMENTS.get('VERBOSE') != 'yes':
    common_env.Append(
            CXXCOMSTR =    '(compile)  $SOURCES',
            LINKCOMSTR =   '(link)     $TARGET',
            ARCOMSTR =     '(archive)  $TARGET',
            RANLIBCOMSTR = '(ranlib)   $TARGET',
            PROTOCCOMSTR = '(protobuf) $TARGET'
    )

debug_env = common_env.Clone(CCFLAGS = ['-O0', '-g'])

if ARGUMENTS.get('DEBUG') == 'yes':
    env = debug_env
else:
    env = common_env
    
sources = Glob('build/library/*.cxx', 'build/library/*.proto')
env.Command('build/library/riakclient.pb.h', 'build/library/riakclient.proto', "protoc $SOURCE --cpp_out=.")
env.Command('build/library/riakclient.pb.cc', 'build/library/riakclient.proto', "protoc $SOURCE --cpp_out=.")
riak_protocol = env.Object('build/library/riakclient.pb.o', 'build/library/riakclient.pb.cc')
library = env.StaticLibrary('riak', [sources, riak_protocol], build_dir='build/library')

