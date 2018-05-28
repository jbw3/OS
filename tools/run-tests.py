#!/usr/bin/env python3

import subprocess
import time

class TestCase:
    def __init__(self, classname, file, line, name, fail):
        self.classname = classname
        self.file = file
        self.line = line
        self.name = name
        self.fail = fail
        self.time = 0.0

class TestSuite:
    def __init__(self, name):
        self.name = name
        self.errors = 0
        self.failures = 0
        self.skips = 0
        self.tests = 0
        self.time = 0.0
        self._testCases = []

    def addTestCase(self, testCase):
        self._testCases.append(testCase)
        self.tests += 1
        if testCase.fail:
            self.failures += 1

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
                testCase = TestCase('TODO', 'TODO', 'TODO', 'TODO', fail=False)
                testSuite.addTestCase(testCase)
            elif line.startswith('ERROR: Tests:'):
                testCase = TestCase('TODO', 'TODO', 'TODO', 'TODO', fail=True)
                testSuite.addTestCase(testCase)

    return testSuite

def writeJUnitXml(filename, testSuite):
    with open(filename, 'w') as xmlFile:
        xmlFile.write('<?xml version="1.0" encoding="utf-8"?>\n')

        xmlFile.write('<testsuite errors="{}" failures="{}" name="{}" skips="{}" tests="{}" time="{}">\n'
                      .format(testSuite.errors, testSuite.failures, testSuite.name, testSuite.skips, testSuite.tests, testSuite.time))

        xmlFile.write('</testsuite>\n')

def main():
    logFilename = 'kernel-x86.log'
    resultsFilename = 'results.xml'

    runQemu(logFilename)
    testSuite = parseLog(logFilename)
    writeJUnitXml(resultsFilename, testSuite)

if __name__ == '__main__':
    main()
