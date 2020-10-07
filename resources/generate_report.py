#! /usr/bin/env python

"""
Assemble all plots and relevant metainformation in a single pdf
report.
"""


from pdf_reports import pug_to_html, write_report, preload_stylesheet
from glob import glob

import sys
import os
import os.path
import re
import datetime as dt

DATAFOLDER='/data'
DATAFOLDER=''

pattern = re.compile("summary-(?P<id>[0-9]*).dat")

def generate_report(context,\
                    outfilename='default.pdf'):
    """
    Generate a pdf report.

    Args:
        context (dict) : context contains the information to fill in the report

    """
    module_id   = context['module_id']
    module_name = context['module_name']
    title = f'Calibration results for module #{module_id} "{module_name}"'
    css = preload_stylesheet('calibration-style.scss')
    
    #for i in range(context['ndetectors']):
    #testplot    = os.path.abspath(context['det1_nm'][0])
    testplot = ''
    html = pug_to_html("calibration-template.pug",\
                       title=title,\
                       context=context,\
                       my_name=context['name'],\
                       det_id1='n.a.',\
                       det_1_stripA=testplot)
    write_report(html, outfilename, extra_stylesheets=[css])

if __name__ == '__main__':

    import argparse
    description = 'Create a report for a calibrated module'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--run-ids',
                        dest='run_ids',
                        nargs='+',
                        type=str,
                        default=[],
                        help='The run ids to get the data')
    args = parser.parse_args()
    run_ids = [int(k) for k in args.run_ids]

    
    context = {\
              'module_name' : 'Jaskier',\
              'module_id'   : -1,
              'ndetectors'  : 0,\
              'date' : dt.datetime.now(),\
              'name' : 'Yennefer of Vengerberg'}
    detector_context = {}
    outfilename = f'calib-mod-{context["module_id"]}-det'
    for i,runid in enumerate(run_ids):
        # FIXME: get run from database
        print (runid)
        datafolder = os.path.join(DATAFOLDER,f'{runid}-gauss-4')
        summaryfilename = os.path.join(datafolder, 'summary*.dat')
        summaryfile = glob(summaryfilename)[0]
        plots = glob(os.path.join(datafolder, '*.png'))
        print (summaryfile)
        print (plots)
        # FIXME:  this comes from the db
        comment = "Working hard for something we don't care about is called stress; working hard for something we love is called passion"
        nmplots = [os.path.abspath(k) for k in filter(lambda x : x.endswith('nmfit.png'), plots)]
        nmplots = sorted(nmplots)
        detid = pattern.search(summaryfile).groupdict()['id']

        asicplotname = os.path.abspath(f'asic-pr-det{detid}.png')
        outfilename += f"-{detid}"
        context['ndetectors'] +=1 
        context[f'detector{i + 1}'] = {'detid'   : int(detid),\
                                       'nmplots' : nmplots,\
                                       'runid'   : runid,\
                                       'comment' : comment,\
                                       'asicplot': asicplotname}
    outfilename += '.pdf'
    generate_report(context,\
                    outfilename=outfilename)

# template stuff

#            img(style="width:100%; display:block; margin:0 auto;"
#                src="file:///{{ det_1_stripA }}")

#            img(style="width:180%; margin:-10;"
#                src="file:///{{ det_1_stripA }}")
# 

#sidebar I am the text in the sidebar.
#216 x 356
