import os

def parse():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-p","--platform")
    parser.add_argument("--with_gpu", action='store_true')
    parser.add_argument("--package_version", default='1.10.0')
    args = parser.parse_args()

    return args


def setup(platform, with_gpu, package_version):
    full_uri = ''
    base_uri = 'https://github.com/microsoft/onnxruntime/releases/download'
    
    if platform.lower() == 'windows':
        package_name = 'onnxruntime-win-x64'
        if with_gpu:
            package_name += '-gpu'
        
        archive_extension = 'zip'
        archive_name = f'{package_name}-{package_version}'
        full_uri   = f'{base_uri}/v{package_version}/{archive_name}.{archive_extension}'

    elif platform.lower() == 'linux':
        package_name = 'onnxruntime-linux-x64'
        if with_gpu:
            package_name += '-gpu'
        
        archive_extension = 'tgz'
        archive_name = f'{package_name}-{package_version}'
        full_uri = f'{base_uri}/v{package_version}/{archive_name}.{archive_extension}'

    elif platform.lower() == 'macos':
        package_name = 'onnxruntime-osx-universal2'
        if with_gpu:
            print("GPU support not available")
        
        archive_extension = 'tgz'
        archive_name = f'{package_name}-{package_version}'
        full_uri = f'{base_uri}/v{package_version}/{archive_name}.{archive_extension}'

    pwd = os.path.abspath(os.getcwd())
    print(f'Downloading onnxruntime binaries: {full_uri} in: {pwd}')

    return pwd, full_uri, archive_name, archive_extension


def download(pwd, full_uri, archive_extension):
    import requests
    file_path = os.path.join(pwd, f'onnxruntime.{archive_extension}')
    with open(file_path, "wb") as file:
        response = requests.get(full_uri)
        file.write(response.content)


def unzip(pwd, archive_extension):
    import shutil
    file_path = os.path.join(pwd, f'onnxruntime.{archive_extension}')
    extract_dir = os.path.join(pwd, 'onnxruntime')
    shutil.unpack_archive(file_path, extract_dir)
    os.remove(file_path)


def rename(pwd, archive_name, platform):
    src_dir = os.path.join(pwd, 'onnxruntime', archive_name)
    dst_dir = os.path.join(pwd, 'onnxruntime', platform)
    os.rename(src_dir, dst_dir)


def main():
    args = parse()
    pwd, full_uri, archive_name, archive_extension = setup(args.platform, args.with_gpu, args.package_version)
    download(pwd, full_uri, archive_extension)
    unzip(pwd, archive_extension)
    rename(pwd, archive_name, args.platform)

main()
