#!/usr/bin/env python3

import os, shutil, subprocess, sys, tempfile

def stripPackExt(name):
    for fmt in shutil.get_unpack_formats():
        for ext in fmt[1]:
            if name.endswith(ext):
                return name.rstrip(ext)
    return name

class Builder(object):
    def __init__(self, args):
        self.args = args
        self.tmpDirs = []

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

    def _checkArgs(self):
        if os.path.exists(self.args.output) and len(os.listdir(self.args.output)) > 0:
            if self.args.force:
                shutil.rmtree(self.args.output)
            else:
                print('The following directory is not empty:')
                print(self.args.output)
                print('Add the --force option to overwrite the existing installation')
                sys.exit(1)

    def _buildBinutils(self):
        srcPath = self._processSrc(self.args.binutilsSrc)
        buildPath = self._makeTempDir('build-' + stripPackExt(os.path.basename(self.args.binutilsSrc)))

        # configure
        configCmd = [os.path.join(srcPath, 'configure'),
                     '--target={}'.format(self.args.target),
                     '--prefix={}'.format(self.args.output),
                     '--with-sysroot',
                     '--disable-nls',
                     '--disable-werror']
        proc = subprocess.Popen(configCmd, cwd=buildPath)
        proc.wait()

        # build
        proc = subprocess.Popen(['make'], cwd=buildPath)
        proc.wait()

        # install
        proc = subprocess.Popen(['make', 'install'], cwd=buildPath)
        proc.wait()

    def _buildGcc(self):
        srcPath = self._processSrc(self.args.gccSrc)
        buildPath = self._makeTempDir('build-' + stripPackExt(os.path.basename(self.args.gccSrc)))

        # set environment vars
        binutilsBinDir = os.path.join(self.args.output, 'bin')
        env = os.environ
        env['PATH'] = binutilsBinDir + ':' + env['PATH']

        # get dependencies
        proc = subprocess.Popen([os.path.join('.', 'contrib', 'download_prerequisites')], cwd=srcPath)
        proc.wait()

        # configure
        configCmd = [os.path.join(srcPath, 'configure'),
                     '--target={}'.format(self.args.target),
                     '--prefix={}'.format(self.args.output),
                     '--disable-nls',
                     '--enable-languages=c,c++',
                     '--without-headers']
        proc = subprocess.Popen(configCmd, cwd=buildPath, env=env)
        proc.wait()

        # build
        proc = subprocess.Popen(['make', 'all-gcc'], cwd=buildPath, env=env)
        proc.wait()

        proc = subprocess.Popen(['make', 'all-target-libgcc'], cwd=buildPath, env=env)
        proc.wait()

        # install
        proc = subprocess.Popen(['make', 'install-gcc'], cwd=buildPath, env=env)
        proc.wait()

        proc = subprocess.Popen(['make', 'install-target-libgcc'], cwd=buildPath, env=env)
        proc.wait()

    def _cleanUp(self):
        # delete temp dirs
        for d in self.tmpDirs:
            shutil.rmtree(d)
        self.tmpDirs = []

    def build(self):
        self._checkArgs()
        self._buildBinutils()
        self._buildGcc()
        self._cleanUp()

        print('Binutils and GCC were installed in the following directory:', end='\n\n')
        print('  ', os.path.join(self.args.output, 'bin'), sep='', end='\n\n')

def parseArgs():
    import argparse

    DESCRIPTION = '''
Tool to build Binutils and GCC. The Binutils and GCC source
code are required arguments. They can be specified either
as directories or compressed files (.zip, .tar.gz, etc.).

Source code can be downloaded at the following links:
https://www.gnu.org/software/binutils/
https://gcc.gnu.org/
'''
    EPILOG = '''Examples:

1. Build from source code in compressed archives:

  build-gcc.py binutils-2.27.tar.gz gcc-5.4.0.tar.bz2

2. Build from source code in directories. The target
architecture is i686-elf:

  build-gcc.py binutils-2.27 gcc-6.1.0 -t i686-elf
'''

    parser = argparse.ArgumentParser(description=DESCRIPTION,
                                     epilog=EPILOG,
                                     formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('binutilsSrc', metavar='BINUTILS', help='the binutils source')
    parser.add_argument('gccSrc', metavar='GCC', help='the GCC source')
    parser.add_argument('-t', '--target', default='i686-elf', help='the target processor')
    parser.add_argument('-o', '--output', default=None, help='the output directory where GCC will be installed')
    parser.add_argument('-f', '--force', action='store_true', default=False, help='force the installation over an existing one')

    args = parser.parse_args()

    # expand binutils path
    args.binutilsSrc = os.path.expanduser(args.binutilsSrc)
    args.binutilsSrc = os.path.abspath(args.binutilsSrc)

    # expand GCC path
    args.gccSrc = os.path.expanduser(args.gccSrc)
    args.gccSrc = os.path.abspath(args.gccSrc)

    # expand output path
    if args.output is None:
        outDir = os.path.basename(args.gccSrc)
        outDir = stripPackExt(outDir)
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
