#!/usr/bin/env python3

import os, shutil, subprocess

class Builder(object):
    def __init__(self, args):
        self.args = args

    def _makeTempDir(self, name):
        # set path in temp directory
        path = os.path.join('/tmp', name)

        # remove path if it already exits
        if os.path.exists(path):
            shutil.rmtree(path)

        # create the directory
        os.makedirs(path)

        return path

    def _buildBinutils(self):
        buildPath = self._makeTempDir('build-binutils')

        # configure
        configCmd = [os.path.join(self.args.binutilsSrc, 'configure'),
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

        # delete build directory
        shutil.rmtree(buildPath)

    def _buildGcc(self):
        buildPath = self._makeTempDir('build-gcc')

        # set environment vars
        binutilsBinDir = os.path.join(self.args.output, 'bin')
        env = os.environ
        env['PATH'] = binutilsBinDir + ':' + env['PATH']

        # get dependencies
        proc = subprocess.Popen(['./contrib/download_prerequisites'], cwd=self.args.gccSrc)
        proc.wait()

        # configure
        configCmd = [os.path.join(self.args.gccSrc, 'configure'),
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

        # delete build directory
        shutil.rmtree(buildPath)

    def build(self):
        self._buildBinutils()
        self._buildGcc()

def parseArgs():
    import argparse

    parser = argparse.ArgumentParser(description='Tool to build Binutils and GCC. Binutils can be downloaded at https://www.gnu.org/software/binutils/. GCC can be downloaded at https://gcc.gnu.org/.')
    parser.add_argument('binutilsSrc', metavar='BINUTILS', help='the binutils source')
    parser.add_argument('gccSrc', metavar='GCC', help='the GCC source')
    parser.add_argument('-t', '--target', default='i686-elf', help='the target processor')
    parser.add_argument('-o', '--output', default=None, help='the output directory where GCC will be installed')

    args = parser.parse_args()

    # check binutils path
    args.binutilsSrc = os.path.abspath(args.binutilsSrc)

    # check GCC path
    args.gccSrc = os.path.abspath(args.gccSrc)

    # check output path
    if args.output is None:
        outDir = os.path.basename(args.gccSrc)
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
