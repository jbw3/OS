#!/usr/bin/env python3

import multiprocessing
import os
import re
import subprocess
import sys

# return codes
RC_SUCCESS      = 0 # all tests passed
RC_RUN_FAILED   = 1 # tests could not be run (e.g. OS failed to boot)
RC_TESTS_FAILED = 2 # tests were run, but some failed

class TestCase:
    def __init__(self, name, fail, message=''):
        self.name = name
        self.fail = fail
        self.message = message

class TestSuite:
    def __init__(self, name):
        self.name = name
        self.errors = 0
        self.failures = 0
        self.skips = 0
        self.tests = 0
        self._testCases = []

    def addTestCase(self, testCase):
        self._testCases.append(testCase)
        self.tests += 1
        if testCase.fail:
            self.failures += 1

    @property
    def testCases(self):
        return self._testCases

class PromptTimeoutExpired(Exception): pass

def waitForPrompt(qemuProc, timeout):
    def readPrompt(qemuProc):
        lastChar = b''
        char = qemuProc.stdout.read(1)
        while lastChar != b'>' or char != b' ':
            lastChar = char
            char = qemuProc.stdout.read(1)

    p = multiprocessing.Process(target=readPrompt, args=(qemuProc,))
    p.start()
    p.join(timeout)

    # If the process is still alive, it is still waiting to read the prompt. Something must
    # have gone wrong while the OS was booting.
    if p.is_alive():
        p.terminate()
        raise PromptTimeoutExpired()

def runQemu(logFilename):
    scriptDir = os.path.dirname(__file__)
    isoPath = os.path.join(scriptDir, '..', 'bin', 'OS-x86.iso')
    isoPath = os.path.abspath(isoPath)

    qemu = 'qemu-system-i386'
    cmd = [qemu, '-nographic', '-serial', 'mon:stdio', '-serial', 'file:{}'.format(logFilename), '-cdrom', isoPath]

    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    try:
        # wait for OS to boot by watching for terminal prompt
        waitForPrompt(proc, timeout=10)
    except PromptTimeoutExpired:
        proc.kill()
        return False

    try:
        # switch to the QEMU monitor and tell it to quit
        proc.communicate(b'\x01cquit\n', timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
        return False

    return True

def parseLog(logFilename):
    testSuite = TestSuite('KernelTests')
    with open(logFilename, 'r') as logFile:
        for line in logFile:
            if line.startswith('INFO: Tests:'):
                match = re.search(r'^INFO: Tests: (.*) passed.', line)
                testCase = TestCase(match.group(1), fail=False)
                testSuite.addTestCase(testCase)
            elif line.startswith('ERROR: Tests:'):
                match = re.search(r'^ERROR: Tests: (.*), line (\d+): (.*)', line)
                testCase = TestCase(match.group(1), fail=True, message=match.group(3))
                testSuite.addTestCase(testCase)

    return testSuite

def sanitizeXmlAttribute(att):
    rv = str(att)
    rv = rv.replace('&', '&amp;') # replace ampersand first
    rv = rv.replace('"', '&quot;')
    return rv

def writeJUnitXml(testSuite, filename):
    with open(filename, 'w') as xmlFile:
        xmlFile.write('<?xml version="1.0" encoding="utf-8"?>\n')

        name = sanitizeXmlAttribute(testSuite.name)
        xmlFile.write('<testsuite errors="{}" failures="{}" name="{}" skips="{}" tests="{}">\n'
                      .format(testSuite.errors, testSuite.failures, name, testSuite.skips, testSuite.tests))

        for testCase in testSuite.testCases:
            tcName = sanitizeXmlAttribute(testCase.name)
            xmlFile.write('    <testcase name="{}">\n'.format(tcName))
            if testCase.fail:
                message = sanitizeXmlAttribute(testCase.message)
                xmlFile.write('        <failure message="{}">\n'.format(message))
                xmlFile.write('        </failure>\n')
            xmlFile.write('    </testcase>\n')

        xmlFile.write('</testsuite>\n')

def writeStdout(testSuite):
    for testCase in testSuite.testCases:
        if testCase.fail:
            print('FAIL: {}: {}'.format(testCase.name, testCase.message))

def parseArgs():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default=None, help='the output file for test results')

    args = parser.parse_args()
    return args

def main():
    args = parseArgs()

    rc = RC_SUCCESS
    logFilename = 'kernel-x86.log'

    ok = runQemu(logFilename)
    if ok:
        testSuite = parseLog(logFilename)
        if args.output is None:
            writeStdout(testSuite)
        else:
            writeJUnitXml(testSuite, args.output)

        if testSuite.errors > 0 or testSuite.failures > 0:
            rc = RC_TESTS_FAILED
    else:
        print('Failed to run tests.', file=sys.stderr)
        rc = RC_RUN_FAILED

    sys.exit(rc)

if __name__ == '__main__':
    main()
