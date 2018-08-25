#!/usr/bin/env python3

import plotly.offline as py
import plotly.graph_objs as go
import re

PAGE_DIR_COLOR = 'rgb(200, 0, 0)'
PAGE_TABLE_COLOR = 'rgb(0, 10, 200)'
PAGE_COLOR = 'rgb(0, 0, 0)'

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
            'ticktext': ['A', 'B']
        },
        'shapes': []
    }

    pageSize = 4096
    pageDir = None
    pageTables = []
    foundPageInfo = False
    with open(args.input, 'r') as inFile:
        for line in inFile:
            if foundPageInfo:
                if line == 'DEBUG: Paging: PageDirEnd\n':
                    break
                else:
                    match = re.search(r'^DEBUG: Paging: (.*): ([\da-fA-F]+)$', line)
                    if match is not None:
                        if match.group(1) == 'PageTable':
                            pageTables.append(int(match.group(2), base=16))
                        elif match.group(1) == 'PageDir':
                            pageDir = int(match.group(2), base=16)
            elif line == 'DEBUG: Paging: PageInfoStart\n':
                foundPageInfo = True

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

    fig = {'data': data, 'layout': layout}
    py.plot(fig, filename='test.html', auto_open=True)

def main():
    args = parseArgs()
    draw(args)

if __name__ == '__main__':
    main()
