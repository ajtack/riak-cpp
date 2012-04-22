import os

library_build_path = '#build/riak/'
VariantDir(library_build_path, 'riak')
common_env = Environment(
        ENV = os.environ,
        CXXFLAGS = ['--std=c++0x'],
        CPPPATH = ['/opt/local/include', '#', '#build/', '/usr/include'],
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

headers = Glob(library_build_path + '*.hxx') + \
          Glob(library_build_path + '*.pb.h')
transports = Glob(library_build_path + 'transports/*.hxx')
sources = Glob(library_build_path + '*.cxx') + \
          Glob(library_build_path + '*.proto') + \
          Glob(library_build_path + 'transports/*.cxx')
env.Command(library_build_path + 'riakclient.pb.h', library_build_path + 'riakclient.proto', "protoc $SOURCE --cpp_out=.")
env.Command(library_build_path + 'riakclient.pb.cc', library_build_path + 'riakclient.proto', "protoc $SOURCE --cpp_out=.")
riak_protocol = env.Object(library_build_path + 'riakclient.pb.o', library_build_path + 'riakclient.pb.cc')
library = env.StaticLibrary('riak', [sources, riak_protocol], build_dir=library_build_path)

# Unit tests are compiled and run every time the program is compiled.
Export('env')
Export('library')

if 'debian' in COMMAND_LINE_TARGETS:
      SConscript("deb/SConscript")

unit_tests = SConscript('test/SConscript', variant_dir='build/test/units')
AddPostAction(unit_tests, unit_tests[0].path)

Default(library, unit_tests)

#
# Installation
#
prefix = '/usr/local'
env.Alias('install', env.Install('/usr/local/lib', library))
env.Alias('install', env.Install('/usr/local/include/riak', headers))
env.Alias('install', env.Install('/usr/local/include/riak/transports', transports))
