#!/usr/bin/env python3

import re
import subprocess
import sys
import time

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

def runQemu(logFilename):
    qemu = 'qemu-system-i386'
    cmd = [qemu, '-nographic', '-monitor', 'stdio', '-serial', 'file:/dev/null', '-serial', 'file:{}'.format(logFilename), '-cdrom', 'bin/OS-x86.iso']

    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE)
    time.sleep(3)

    proc.communicate(b'quit\n')
    proc.wait()

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

def writeJUnitXml(testSuite, filename):
    with open(filename, 'w') as xmlFile:
        xmlFile.write('<?xml version="1.0" encoding="utf-8"?>\n')

        xmlFile.write('<testsuite errors="{}" failures="{}" name="{}" skips="{}" tests="{}">\n'
                      .format(testSuite.errors, testSuite.failures, testSuite.name, testSuite.skips, testSuite.tests))

        for testCase in testSuite.testCases:
            xmlFile.write('    <testcase name="{}">\n'.format(testCase.name))
            if testCase.fail:
                xmlFile.write('        <failure message="{}">\n'.format(testCase.message))
                xmlFile.write('        </failure>\n')
            xmlFile.write('    </testcase>\n')

        xmlFile.write('</testsuite>\n')

def writeStdout(testSuite):
    for testCase in testSuite.testCases:
        if testCase.fail:
            print('FAIL: {}'.format(testCase.name))

def parseArgs():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default=None, help='the output file for test results')

    args = parser.parse_args()
    return args

def main():
    args = parseArgs()

    logFilename = 'kernel-x86.log'

    runQemu(logFilename)
    testSuite = parseLog(logFilename)

    if args.output is None:
        writeStdout(testSuite)
    else:
        writeJUnitXml(testSuite, args.output)

    # return 0 if all tests passed, or 1 if any tests failed
    rc = 0
    if testSuite.errors > 0 or testSuite.failures > 0:
        rc = 1
    sys.exit(rc)

if __name__ == '__main__':
    main()
