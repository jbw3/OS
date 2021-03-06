#!/usr/bin/env python3

import multiprocessing
import os
import re
import subprocess
import sys

# return codes
RC_SUCCESS      = 0 # all tests passed
RC_TESTS_FAILED = 1 # tests were run, but some failed
RC_RUN_FAILED   = 2 # all tests may not have been run (e.g. OS failed to boot, page fault occurred, etc.)

class TestCase:
    def __init__(self, className, testName):
        self.className = className
        self.testName = testName
        self.error = False
        self.fail = False
        self.errorMessage = ''
        self.failMessage = ''
        self.filename = ''
        self.line = -1

class TestSuite:
    def __init__(self):
        self.name = 'KernelTests'
        self._testCases = []

    def addTestCase(self, testCase):
        self._testCases.append(testCase)

    @property
    def testCases(self):
        return self._testCases

    @property
    def numTests(self):
        return len(self._testCases)

    @property
    def numSkips(self):
        # right now, tests cannot be skipped
        return 0

    @property
    def numErrors(self):
        return sum([1 for tc in self._testCases if tc.error])

    @property
    def numFailures(self):
        return sum([1 for tc in self._testCases if tc.fail])

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

    # wait for OS to boot by watching for terminal prompt
    try:
        waitForPrompt(proc, timeout=10)
    except PromptTimeoutExpired:
        proc.kill()
        return False

    # run unit tests
    proc.stdin.write(b'run-kernel-tests\n')
    proc.stdin.flush()
    try:
        waitForPrompt(proc, timeout=10)
    except PromptTimeoutExpired:
        proc.kill()
        return False

    # switch to the QEMU monitor and tell it to quit
    try:
        proc.communicate(b'\x01cquit\n', timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
        return False

    return True

def parseLog(logFilename):
    testSuite = TestSuite()
    testClassName = None
    testCase = None
    with open(logFilename, 'r') as logFile:
        for line in logFile:
            # check if this is the start of a test class
            if line.startswith('INFO: Tests: TestClass:'):
                match = re.search(r'^INFO: Tests: TestClass: (.*)', line)
                testClassName = match.group(1)
            # check if this is the start of a test
            elif line.startswith('INFO: Tests: Test:'):
                match = re.search(r'^INFO: Tests: Test: (.*)', line)
                testCase = TestCase(testClassName, match.group(1))
                testSuite.addTestCase(testCase)
            # check if the the current test failed
            elif line.startswith('ERROR: Tests: Fail:'):
                match = re.search(r'^ERROR: Tests: Fail: (.*), (.*), (\d+): (.*)', line)
                testCase.fail = True
                testCase.filename = match.group(2)
                testCase.line = int(match.group(3))
                testCase.failMessage = match.group(4)
            # check if there was an error while the current test was running
            elif testCase is not None and line.startswith('ERROR: '):
                match = re.search(r'^ERROR: (.*)', line)
                testCase.error = True
                testCase.errorMessage = match.group(1)

    return testSuite

def sanitizeXml(att):
    rv = str(att)
    rv = rv.replace('&', '&amp;') # replace ampersand first
    rv = rv.replace('"', '&quot;')
    rv = rv.replace('<', '&lt;')
    rv = rv.replace('>', '&gt;')
    return rv

def writeJUnitXml(testSuite, filename):
    with open(filename, 'w') as xmlFile:
        xmlFile.write('<?xml version="1.0" encoding="utf-8"?>\n')

        tsName = sanitizeXml(testSuite.name)
        xmlFile.write('<testsuite name="{}" tests="{}" skips="{}" errors="{}" failures="{}">\n'
                      .format(tsName, testSuite.numTests, testSuite.numSkips, testSuite.numErrors, testSuite.numFailures))

        for testCase in testSuite.testCases:
            className = sanitizeXml(testCase.className)
            testName = sanitizeXml(testCase.testName)
            if testCase.error or testCase.fail:
                xmlFile.write('    <testcase classname="{}" name="{}">\n'.format(className, testName))
                if testCase.error:
                    msg = sanitizeXml(testCase.errorMessage)
                    xmlFile.write('        <error message="{}">\n'.format(msg))
                    xmlFile.write(msg + '\n')
                    xmlFile.write('        </error>\n')
                if testCase.fail:
                    msg = sanitizeXml(testCase.failMessage)
                    xmlFile.write('        <failure message="{}">\n'.format(msg))
                    xmlFile.write(msg + '\n')
                    xmlFile.write('File: {}\n'.format(testCase.filename))
                    xmlFile.write('Line: {}\n'.format(testCase.line))
                    xmlFile.write('        </failure>\n')
                xmlFile.write('    </testcase>\n')
            else:
                xmlFile.write('    <testcase classname="{}" name="{}"/>\n'.format(className, testName))

        xmlFile.write('</testsuite>\n')

def writeStdout(testSuite):
    for testCase in testSuite.testCases:
        if testCase.error:
            print('ERROR: {}: {}: {}'.format(testCase.className, testCase.testName, testCase.errorMessage))
        if testCase.fail:
            print('FAIL: {}: {}: {}, line {}: {}'.format(testCase.className, testCase.testName, testCase.filename, testCase.line, testCase.failMessage))

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

    runSuccessful = runQemu(logFilename)

    testSuite = parseLog(logFilename)
    if args.output is None:
        writeStdout(testSuite)
    else:
        writeJUnitXml(testSuite, args.output)

    if not runSuccessful:
        print('OS failed to shut down gracefully. All tests may not have been run successfully.', file=sys.stderr)
        rc = RC_RUN_FAILED
    elif testSuite.numErrors > 0 or testSuite.numFailures > 0:
        rc = RC_TESTS_FAILED

    sys.exit(rc)

if __name__ == '__main__':
    main()
