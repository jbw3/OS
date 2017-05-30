#!/usr/bin/env python3

import os, shutil, subprocess, sys, tempfile, urllib.error, urllib.request

def stripPackExt(name):
    for fmt in shutil.get_unpack_formats():
        for ext in fmt[1]:
            if name.endswith(ext):
                return name.rstrip(ext)
    return name

def getBinutilsFilename(verStr):
    return 'binutils-{}.tar.bz2'.format(verStr)

def getGccFilename(verStr):
    return 'gcc-{}.tar.bz2'.format(verStr)

def getBinutilsUrl(verStr):
    filename = getBinutilsFilename(verStr)
    return 'http://ftp.gnu.org/gnu/binutils/{}'.format(filename)

def getGccUrl(verStr):
    filename = getGccFilename(verStr)
    return 'http://mirrors.concertpass.com/gcc/releases/gcc-{}/{}'.format(verStr, filename)

def downloadFile(name, url, filename):
    ok = False
    print('Downloading {}...'.format(name), end='', flush=True)

    try:
        urllib.request.urlretrieve(url, filename)
    except urllib.error.HTTPError:
        print('error.')
    else:
        ok = True
        print('done.')

    return ok

class Builder(object):
    CONFIG_DIRECTORY = os.path.expanduser('~/.build-gcc')
    CACHE_DIRECTORY = os.path.join(CONFIG_DIRECTORY, 'cache')

    def __init__(self, args):
        self.args = args
        self.tmpDirs = []
        self.binutilsPath = None
        self.gccPath = None
        self.subprocStdout = None if self.args.verbose else subprocess.DEVNULL
        self.cachedFiles = []

    def _getDependencies(self):
        # create cache directory if it does not already exist
        os.makedirs(Builder.CACHE_DIRECTORY, exist_ok=True)

        binutilsFilename = getBinutilsFilename(self.args.binutils_version)
        gccFilename = getGccFilename(self.args.gcc_version)

        self.binutilsPath = os.path.join(Builder.CACHE_DIRECTORY, binutilsFilename)
        self.gccPath = os.path.join(Builder.CACHE_DIRECTORY, gccFilename)

        binutilsUrl = getBinutilsUrl(self.args.binutils_version)
        gccUrl = getGccUrl(self.args.gcc_version)

        # download binutils if needed
        if not os.path.exists(self.binutilsPath):
            ok = downloadFile('Binutils', binutilsUrl, self.binutilsPath)
            if not ok:
                sys.exit(1)
            self.cachedFiles.append(self.binutilsPath)

        # download GCC if needed
        if not os.path.exists(self.gccPath):
            ok = downloadFile('GCC', gccUrl, self.gccPath)
            if not ok:
                sys.exit(1)
            self.cachedFiles.append(self.gccPath)

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

    def _buildBinutils(self):
        print('Building Binutils...', end='\n' if self.args.verbose else '', flush=True)

        srcPath = self._processSrc(self.binutilsPath)
        buildPath = self._makeTempDir('build-' + stripPackExt(os.path.basename(self.binutilsPath)))

        # configure
        configCmd = [os.path.join(srcPath, 'configure'),
                     '--target={}'.format(self.args.target),
                     '--prefix={}'.format(self.args.output),
                     '--with-sysroot',
                     '--disable-nls',
                     '--disable-werror']
        proc = subprocess.Popen(configCmd, cwd=buildPath, stdout=self.subprocStdout)
        proc.wait()

        # build
        proc = subprocess.Popen(['make'], cwd=buildPath, stdout=self.subprocStdout)
        proc.wait()

        # install
        proc = subprocess.Popen(['make', 'install'], cwd=buildPath, stdout=self.subprocStdout)
        proc.wait()

        print('done.')

    def _buildGcc(self):
        print('Building GCC...', end='\n' if self.args.verbose else '', flush=True)

        srcPath = self._processSrc(self.gccPath)
        buildPath = self._makeTempDir('build-' + stripPackExt(os.path.basename(self.gccPath)))

        # set environment vars
        binutilsBinDir = os.path.join(self.args.output, 'bin')
        env = os.environ
        env['PATH'] = binutilsBinDir + ':' + env['PATH']

        # get dependencies
        proc = subprocess.Popen([os.path.join('.', 'contrib', 'download_prerequisites')], cwd=srcPath, stdout=self.subprocStdout)
        proc.wait()

        # configure
        configCmd = [os.path.join(srcPath, 'configure'),
                     '--target={}'.format(self.args.target),
                     '--prefix={}'.format(self.args.output),
                     '--disable-nls',
                     '--enable-languages=c,c++',
                     '--without-headers']
        proc = subprocess.Popen(configCmd, cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        # build
        proc = subprocess.Popen(['make', 'all-gcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        proc = subprocess.Popen(['make', 'all-target-libgcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        # install
        proc = subprocess.Popen(['make', 'install-gcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        proc = subprocess.Popen(['make', 'install-target-libgcc'], cwd=buildPath, env=env, stdout=self.subprocStdout)
        proc.wait()

        print('done.')

    def _cleanUp(self):
        # delete temp dirs
        for d in self.tmpDirs:
            shutil.rmtree(d)
        self.tmpDirs = []

        # delete cached files
        if self.args.no_cache:
            for f in self.cachedFiles:
                os.remove(f)

    def build(self):
        self._getDependencies()
        self._buildBinutils()
        self._buildGcc()
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
    parser.add_argument('-b', '--binutils-version', default='2.28', help='the binutils version')
    parser.add_argument('-g', '--gcc-version', default='6.3.0', help='the GCC version')
    parser.add_argument('-t', '--target', default='i686-elf', help='the target processor (e.g. i686-elf, x86_64-elf, etc.)')
    parser.add_argument('--no-cache', action='store_true', help="don't cache downloaded files")
    parser.add_argument('-o', '--output', default=None, help='the output directory where GCC will be installed')
    parser.add_argument('-v', '--verbose', action='store_true', help='enable verbose output')

    args = parser.parse_args()

    # expand output path
    if args.output is None:
        outDir = 'gcc-{}'.format(args.gcc_version)
        args.output = os.path.join('~', 'opt', 'cross', outDir)
    args.output = os.path.expanduser(args.output)
    args.output = os.path.abspath(args.output)

    return args

def main():
    args = parseArgs()
    builder = Builder(args)
    builder.build()

if __name__ == '__main__':
    main()
