import subprocess
import argparse
import pathlib

def parse():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("compiler", help="Compiler name", choices=['gcc', 'clang', 'Visual Studio'])
    parser.add_argument("compiler_version", help="Compiler version")
    return parser.parse_args()

def run(command):
    try:
        ret = subprocess.run(command)
        ret.check_returncode()
    except Exception as e:
        print(f'Unhandled Exception: {e}')

def conan_profile_create(profile_path):
    command = ([ 'conan', 'profile', 'new', f'{profile_path}', '--force', '--detect'])
    run(command)

def conan_profile_update(profile_path, args):
    command = ([ 'conan', 'profile', 'update', f'settings.compiler={args.compiler}', f'{profile_path}' ])
    run(command)

    command = ([ 'conan', 'profile', 'update', f'settings.compiler.version={args.compiler_version}', f'{profile_path}'])
    run(command)

    # for conan v1 recipes
    command = ([ 'conan', 'profile', 'update', f'env.CONAN_CMAKE_GENERATOR=Ninja', f'{profile_path}'])
    run(command)
    
    # for conan v2 recipes
    command = ([ 'conan', 'profile', 'update', f'conf.tools.cmake.cmaketoolchain:generator=Ninja', f'{profile_path}'])
    run(command)

    if args.compiler == 'gcc' or args.compiler == 'clang':
        command = ([ 'conan', 'profile', 'update', f'settings.compiler.libcxx=libstdc++11', f'{profile_path}'])
        run(command)
        
        cc_compiler = f'{args.compiler}-{args.compiler_version}'
        command = ([ 'conan', 'profile', 'update', f'env.CC={cc_compiler}', f'{profile_path}'])
        run(command)

        cxx_compiler = ''
        if args.compiler == 'gcc':
            cxx_compiler = f'g++-{args.compiler_version}'

        if args.compiler == 'clang':
            cxx_compiler = f'clang++-{args.compiler_version}'

        command = ([ 'conan', 'profile', 'update', f'env.CXX={cxx_compiler}', f'{profile_path}'])
        run(command)



def conan_profile_show(profile_path):
    command = ([ 'conan', 'profile', 'show', f'{profile_path}' ])
    run(command)

args = parse()

conan_directory = pathlib.Path(__file__).resolve().parent
profile_path = pathlib.PurePath(conan_directory, "profiles", f'{args.compiler}{args.compiler_version}')

conan_profile_create(profile_path)
conan_profile_update(profile_path, args)
conan_profile_show(profile_path)