import os
import sys
environments = SConscript('environment.SConscript')

if environments is not None:
    release_env, debug_env = environments[0], environments[1]

    def build_variant(script_path, name, *additional_exports):
        #
        # Allow either command-line or environment selection of DEBUG build.
        #
        if ARGUMENTS.get('DEBUG', os.environ['RIAK_CPP_BUILD_DEBUG'] if 'RIAK_CPP_BUILD_DEBUG' in os.environ else 'no') == "yes":
            variant = 'debug'
        else:
            variant = 'release'

        #
        # Every variant (release, debug, etc.) gets build into its own variant directory.
        #
        env = debug_env if variant == 'debug' else release_env
        variant_root = \
                '#build' + os.sep + \
                sys.platform + os.sep + \
                ((env['TARGET_ARCH'] + os.sep) if env['TARGET_ARCH'] else '') + \
                variant + os.sep
        build_root = variant_root + name + os.sep
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
else:
    print "Skipping all targets."
