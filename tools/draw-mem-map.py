#!/usr/bin/env python3

import plotly.offline as py
import plotly.graph_objs as go

def main():
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
            # 'tickvals': [i for i in range(2**32)],
        },
        'shapes': [
            {
                'type': 'rect',
                'x0': 0,
                'x1': 4,
                'y0': 0,
                'y1': 2**20,
            }
        ]
    }

    fig = {'data': data, 'layout': layout}
    py.plot(fig, filename='test.html', auto_open=True)

if __name__ == '__main__':
    main()
