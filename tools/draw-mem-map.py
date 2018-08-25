#!/usr/bin/env python3

import plotly.offline as py
import plotly.graph_objs as go
import re

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
    foundPageDir = False
    with open(args.input, 'r') as inFile:
        for line in inFile:
            if foundPageDir:
                if line == 'DEBUG: Paging: PageDirEnd\n':
                    break
            elif line.startswith('DEBUG: Paging: PageDirStart: '):
                foundPageDir = True
                match = re.search(r'^DEBUG: Paging: PageDirStart: ([\da-fA-F]+)$', line)
                pageDir = int(match.group(1), base=16)

    layout['shapes'].append({
        'type': 'rect',
        'x0': 0,
        'x1': 4,
        'y0': pageDir,
        'y1': pageDir + pageSize - 1
    })

    fig = {'data': data, 'layout': layout}
    py.plot(fig, filename='test.html', auto_open=True)

def main():
    args = parseArgs()
    draw(args)

if __name__ == '__main__':
    main()
