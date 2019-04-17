#!/usr/bin/env python3

import multiprocessing
import os
import requests
import sys
import time

########## Globals ##########

PROCESSES = 1
REQUESTS  = 1
VERBOSE   = False
URL       = None

########## Functions ##########

### Usage ###
def usage(status=0):
    print('''Usage: {} [-p PROCESSES -r REQUESTS -v] URL
    -h              Display help message
    -v              Display verbose output

    -p  PROCESSES   Number of processes to utilize (1)
    -r  REQUESTS    Number of requests per process (1)
    '''.format(os.path.basename(sys.argv[0])))
    sys.exit(status)

### do_request ###
def do_request(pid):
    ''' Perform REQUESTS HTTP requests and return the average elapsed time. '''
    times = []
    for i in range(REQUESTS):
        pre_time = time.time()
        response = requests.get(URL)
        if VERBOSE:
            print(response.text)
        post_time = time.time()
        times.append(post_time - pre_time)
        print("Process: {}, Request: {}, Elapsed Time: {:.2f}".format(pid, i, post_time - pre_time))
    average = sum(times) / len(times)
    print("Process: {}, AVERAGE   , Elapsed Time: {:.2f}".format(pid, average))
    return average


########## Main execution ##########

### main ###
if __name__ == '__main__':

    # Parse command line arguments
    args = sys.argv[1:]
    while len(args) and args[0].startswith('-') and len(args[0]) > 1:
        arg = args.pop(0)
        if arg == "-h":
            usage(0)
        elif arg == "-v":
            VERBOSE = True
        elif arg == "-p":
            PROCESSES = int(args.pop(0))
        elif arg == "-r":
            REQUESTS = int(args.pop(0))
        else:
            usage(1)

    # Make sure only 1 argument left
    if len(args) != 1:
        usage(1)

    # Get URL
    URL = args.pop(0)

    # Create pool of workers and perform requests
    pool = multiprocessing.Pool(PROCESSES)
    result = pool.map(do_request, range(PROCESSES))
    total_average = sum(result) / len(result)
    print("TOTAL AVERAGE ELAPSED TIME: {:.2f}".format(total_average))
    pool.close()
    pool.join()

    pass

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
