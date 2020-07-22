#!/usr/bin/env python3

import os, shutil, subprocess, sys, tempfile, urllib.error, urllib.request

def stripPackExt(name):
    for fmt in shutil.get_unpack_formats():
        for ext in fmt[1]:
            if name.endswith(ext):
                return name.rstrip(ext)
    return name

def getBinutilsBaseFilename(verStr):
    return 'binutils-{}'.format(verStr)

def getGccBaseFilename(verStr):
    return 'gcc-{}'.format(verStr)

def getBinutilsBaseUrl(verStr):
    filename = getBinutilsBaseFilename(verStr)
    return 'https://mirrors.ocf.berkeley.edu/gnu/binutils/{}'.format(filename)

def getGccBaseUrl(verStr):
    filename = getGccBaseFilename(verStr)
    return 'https://mirrors.ocf.berkeley.edu/gnu/gcc/gcc-{}/{}'.format(verStr, filename)

def downloadFile(url, filename):
    try:
        urllib.request.urlretrieve(url, filename)
    except urllib.error.HTTPError:
        return False
    else:
        return True

class Builder:
    CACHE_DIRECTORY = os.path.expanduser(os.path.join('~', '.cache', 'build-gcc'))
    ARCHIVE_EXTENSIONS = ('.tar.xz', '.tar.bz2', '.tar.gz')

    def __init__(self, args):
        self.args = args
        self.tmpDirs = []
        self.binutilsPath = None
        self.gccPath = None
        self.subprocStdout = None if self.args.verbose else subprocess.DEVNULL
        self.cachedFiles = []

    def _downloadDependency(self, name, baseUrl, basePath):
        print('Downloading {}...'.format(name), end='', flush=True)

        ok = False
        for ext in Builder.ARCHIVE_EXTENSIONS:
            url = baseUrl + ext
            path = basePath + ext
            ok = downloadFile(url, path)
            if ok:
                break

        if not ok:
            print('error.')
            sys.exit(1)

        print('done.')
        return path

    def _getDependency(self, name, baseFilename, baseUrl):
        isCached = False

        # check for cached file
        for ext in Builder.ARCHIVE_EXTENSIONS:
            filename = baseFilename + ext
            path = os.path.join(Builder.CACHE_DIRECTORY, filename)
            if os.path.exists(path):
                isCached = True
                break

        # download file
        if not isCached:
            basePath = os.path.join(Builder.CACHE_DIRECTORY, baseFilename)
            path = self._downloadDependency(name, baseUrl, basePath)
            self.cachedFiles.append(path)

        return path

    def _getDependencies(self):
        # create cache directory if it does not already exist
        os.makedirs(Builder.CACHE_DIRECTORY, exist_ok=True)

        # get binutils
        binutilsBaseFilename = getBinutilsBaseFilename(self.args.binutils_version)
        binutilsBaseUrl = getBinutilsBaseUrl(self.args.binutils_version)
        self.binutilsPath = self._getDependency('Binutils', binutilsBaseFilename, binutilsBaseUrl)

        # get GCC
        gccBaseFilename = getGccBaseFilename(self.args.gcc_version)
        gccBaseUrl = getGccBaseUrl(self.args.gcc_version)
        self.gccPath = self._getDependency('GCC', gccBaseFilename, gccBaseUrl)

    def _processSrc(self, src):
        if os.path.isdir(src):
            return src
        else:
            shutil.unpack_archive(src, tempfile.gettempdir())
            extractDir = os.path.join(tempfile.gettempdir(), stripPackExt(os.path.basename(src)))
            self.tmpDirs.append(extractDir)
            return extractDir

    def _makeTempDir(self, name):
        # set path in temp directory
        path = os.path.join(tempfile.gettempdir(), name)

        # remove path if it already exits
        if os.path.exists(path):
            shutil.rmtree(path)

        # create the directory
        os.makedirs(path)

        # save directory so we can delete it later
        self.tmpDirs.append(path)

        return path

    def _buildBinutils(self, target, njobs=12):
        print('Building Binutils...', end='\n' if self.args.verbose else '', flush=True)

        srcPath = self._processSrc(self.binutilsPath)
        buildPath = self._makeTempDir('build-{}-{}'.format(stripPackExt(os.path.basename(self.binutilsPath)), target))

        # configure
        configCmd = [os.path.join(srcPath, 'configure'),
                     '--target={}'.format(target),
                     '--prefix={}'.format(self.args.output),
                     '--with-sysroot',
                     '--disable-nls',
                     '--disable-werror']
        proc = subprocess.Popen(configCmd, cwd=buildPath, stdout=self.subprocStdout)
        proc.wait()

        # build
        proc = subprocess.Popen(['make', f'-j{njobs}'], cwd=buildPath, stdout=self.subprocStdout)
        proc.wait()

        # install
        proc = subprocess.Popen(['make', f'-j{njobs}', 'install'], cwd=buildPath, stdout=self.subprocStdout)
        proc.wait()

        print('done.')

    def _buildGcc(self, target, njobs=12):
        print('Building GCC...', end='\n' if self.args.verbose else '', flush=True)

        srcPath = self._processSrc(self.gccPath)
        buildPath = self._makeTempDir('build-{}-{}'.format(stripPackExt(os.path.basename(self.gccPath)), target))

        # set environment vars
        binutilsBinDir = os.path.join(self.args.output, 'bin')
        env = os.environ
        env['PATH'] = binutilsBinDir + ':' + env['PATH']

        # get dependencies
        proc = subprocess.Popen([os.path.join('.', 'contrib', 'download_prerequisites')], cwd=srcPath, stdout=self.subprocStdout)
        proc.wait()

        # configure
        configCmd = [os.path.join(srcPath, 'configure'),
                     '--target={}'.format(target),
                     '--prefix={}'.format(self.args.output),
                     '--disable-nls',
                     '--enable-languages=c,c++',
                     '--without-headers']
        proc = subprocess.Popen(configCmd, cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        # build
        proc = subprocess.Popen(['make', f'-j{njobs}', 'all-gcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        proc = subprocess.Popen(['make', f'-j{njobs}', 'all-target-libgcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        # install
        proc = subprocess.Popen(['make', f'-j{njobs}', 'install-gcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        proc = subprocess.Popen(['make', f'-j{njobs}', 'install-target-libgcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        print('done.')

    def _cleanUp(self):
        # delete temp dirs
        for d in self.tmpDirs:
            try:
                shutil.rmtree(d)
            except FileNotFoundError:
                pass
        self.tmpDirs = []

        # delete cached files
        if self.args.no_cache:
            for f in self.cachedFiles:
                os.remove(f)

    def build(self):
        self._getDependencies()

        for target in self.args.targets:
            self._buildBinutils(target)
            self._buildGcc(target)

        self._cleanUp()

        print('Binutils and GCC were installed in the following directory:', end='\n\n')
        print('  ', os.path.join(self.args.output, 'bin'), sep='', end='\n\n')

def parseArgs():
    import argparse

    DESCRIPTION = '''
Tool to build GCC and its dependencies.
'''
    EPILOG = '''Examples:

1. Build default version of GCC:

  build-gcc.py

2. Build GCC 6.3.0 using Binutils 2.28:

  build-gcc.py -g 6.3.0 -b 2.28

3. Build GCC for the i686-elf architecture:

  build-gcc.py -t i686-elf
'''

    parser = argparse.ArgumentParser(description=DESCRIPTION,
                                     epilog=EPILOG,
                                     formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-b', '--binutils-version', default='2.29.1', help='the binutils version')
    parser.add_argument('-g', '--gcc-version', default='7.2.0', help='the GCC version')
    parser.add_argument('-t', '--targets', nargs='+', metavar='TARGET', default=['i686-elf'], help='the target processors (e.g. i686-elf, x86_64-elf, etc.)')
    parser.add_argument('--no-cache', action='store_true', help="don't cache downloaded files")
    parser.add_argument('-o', '--output', default=None, help='the output directory where GCC will be installed')
    parser.add_argument('-v', '--verbose', action='store_true', help='enable verbose output')

    args = parser.parse_args()

    # expand output path
    if args.output is None:
        args.output = os.path.join('~', 'opt', 'cross')
    args.output = os.path.expanduser(args.output)
    args.output = os.path.abspath(args.output)

    # add GCC sub-dir to path
    outDir = 'gcc-{}'.format(args.gcc_version)
    args.output = os.path.join(args.output, outDir)

    return args

def main():
    args = parseArgs()
    builder = Builder(args)
    builder.build()

if __name__ == '__main__':
    main()
