import os

common_env = Environment(
        ENV = os.environ,
        CXXFLAGS = ['--std=c++11', '-DBOOST_ALL_DYN_LINK'],
        CPPPATH = ['/opt/local/include', '#', '/usr/include'],
        LIBPATH = ['/opt/local/lib'])

if ARGUMENTS.get('VERBOSE') != 'yes':
    common_env.Append(
            CXXCOMSTR =    '(compile)  $SOURCES',
            LINKCOMSTR =   '(link)     $TARGET',
            ARCOMSTR =     '(archive)  $TARGET',
            RANLIBCOMSTR = '(ranlib)   $TARGET',
            PROTOCCOMSTR = '(generate) $SOURCES'
    )

debug_env = common_env.Clone(CCFLAGS = ['-O0', '-g'])

def build_variant(script_path, name, *additional_exports):
    variant = 'debug' if ARGUMENTS.get('DEBUG') == 'yes' else 'release'
    env = debug_env.Clone() if variant == 'debug' else common_env.Clone()
    variant_root = '#build/' + variant + '/'
    build_root = variant_root + name + '/'
    env.Append(CPPPATH = [variant_root])
    
    exports = list(additional_exports)
    exports.append('env')
    return SConscript(script_path, exports, variant_dir=build_root)

library = build_variant('riak/SConscript', 'riak')
unit_tests = build_variant('test/SConscript', 'test', 'library')
AddPostAction(unit_tests, unit_tests[0].path)
Default(library, unit_tests)

if 'debian' in COMMAND_LINE_TARGETS:
      SConscript("deb/SConscript")
