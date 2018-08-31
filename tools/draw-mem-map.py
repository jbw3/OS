#!/usr/bin/env python3

import plotly.offline as py
import plotly.graph_objs as go
import re

PAGE_DIR_COLOR = 'rgb(255, 30, 60)'
PAGE_TABLE_COLOR = 'rgb(30, 180, 255)'
PAGE_COLOR = 'rgb(50, 50, 50)'

class PageRange:
    def __init__(self, startAddr, numPages):
        self.startAddr = startAddr
        self.numPages = numPages

def parseArgs():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', default='kernel-x86.log', help='the input kernel log file')

    args = parser.parse_args()
    return args

def draw(args):
    trace0 = go.Scatter(
        x=[2],
        y=[1],
        mode='text',
    )
    data = [trace0]

    layout = {
        'xaxis': {
            'range': [0, 4],
            'showgrid': False,
            'dtick': 1,
            'fixedrange': True, # disable zoom
        },
        'yaxis': {
            'range': [0, 2**32 - 1],
            'dtick': 2**30,
        },
        'shapes': []
    }

    pageSize = 4096
    pageDir = None
    pageTables = []
    pageRanges = []
    foundPageInfo = False
    with open(args.input, 'r') as inFile:
        for line in inFile:
            if foundPageInfo:
                if line == 'DEBUG: Paging: PageDirEnd\n':
                    break
                else:
                    match = re.search(r'^DEBUG: Paging: (.*): ([\da-fA-F]+),?(\d+)?$', line)
                    if match is not None:
                        if match.group(1) == 'PageRange':
                            startAddr = int(match.group(2), base=16)
                            numPages = int(match.group(3))
                            pageRanges.append(PageRange(startAddr, numPages))
                        elif match.group(1) == 'PageTable':
                            pageTables.append(int(match.group(2), base=16))
                        elif match.group(1) == 'PageDir':
                            pageDir = int(match.group(2), base=16)
            elif line == 'DEBUG: Paging: PageInfoStart\n':
                foundPageInfo = True

    # add pages
    for pageRange in pageRanges:
        layout['shapes'].append({
            'type': 'rect',
            'x0': 0,
            'x1': 4,
            'y0': pageRange.startAddr,
            'y1': pageRange.startAddr + (pageSize * pageRange.numPages) - 1,
            'line': {
                'color': PAGE_COLOR
            },
            'fillcolor': PAGE_COLOR
        })

    # add page tables
    for pageTable in pageTables:
        layout['shapes'].append({
            'type': 'rect',
            'x0': 0,
            'x1': 4,
            'y0': pageTable,
            'y1': pageTable + pageSize - 1,
            'line': {
                'color': PAGE_TABLE_COLOR
            },
            'fillcolor': PAGE_TABLE_COLOR
        })

    # add page dir
    layout['shapes'].append({
        'type': 'rect',
        'x0': 0,
        'x1': 4,
        'y0': pageDir,
        'y1': pageDir + pageSize - 1,
        'line': {
            'color': PAGE_DIR_COLOR
        },
        'fillcolor': PAGE_DIR_COLOR
    })

    fig = {'data': data, 'layout': layout}
    py.plot(fig, filename='test.html', auto_open=True)

def main():
    args = parseArgs()
    draw(args)

if __name__ == '__main__':
    main()
