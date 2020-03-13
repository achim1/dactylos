"""
Control a SUN EC13 temperature chamber
"""

import logging
import sys

from matplotlib import get_configdir as mpl_configdir
from os.path import split as _split
from os.path import join as _join
from os.path import exists as _exists
from shutil import copy2 as _copy
from datetime import datetime as _datetime

from .chamber import SunChamber, NI_GPIB_USB, PrologixUsbGPIBController


__all__ = ['gui', 'chamber']
__version__ = "0.0.1dev"

#__all__ = ["magnetometer", "Lutron instruments", "gui"]

_appdir = _split(__file__)[0]

LOGFORMAT = '[%(asctime)s] %(levelname)s: %(module)s(%(lineno)d):   %(message)s'
_appdir = _join(_appdir, 'gui')
STYLE_BASEFILE_STD = _join(_appdir,"risingsun.mplstyle")

logging.captureWarnings(True)


def create_timestamped_file(filename, file_ending=".log"):
    """
    Return a timestamped filename (with the date in it) which is not yet
    present on the system
    Args:
        filename (str): Name of the file (full path) (without ending)

    Keyword Args:
        file_ending (str): The fileending of the new file

    Returns:
        str
    """
    today = _datetime.now()
    today = today.strftime("%Y_%m_%d_%H_%M")
    if filename.endswith(file_ending):
        filename.replace(file_ending, today + file_ending)
    else:
        filename += (today + file_ending)
    filecount = 1
    while _exists(filename):
        filename = filename.replace("." + str(filecount - 1), "")
        filename = filename + "." + str(filecount)
        filecount += 1
        if filecount >= 60:
            raise SystemError("More than 1 file per second, this is insane.. aborting")
    return filename

def get_logger(loglevel, logfile=None):
    """
    A root logger with a formatted output logging to stdout and a file

    Args:
        loglevel (int): 10,20,30,... the higher the less logging

    Keyword Args:
        logfile (str): write logging to this file as well as stdout

    """

    def exception_handler(exctype, value, tb):
        logger.critical("Uncaught exception", exc_info=(exctype, value, tb))

    logger = logging.getLogger()
    logger.setLevel(loglevel)
    ch = logging.StreamHandler()
    ch.setLevel(loglevel)
    formatter = logging.Formatter(LOGFORMAT)
    ch.setFormatter(formatter)
    logger.addHandler(ch)
    if logfile is not None:
        logfile = create_timestamped_file(logfile, file_ending=".log")
        fh = logging.FileHandler(logfile)
        fh.setFormatter(formatter)
        fh.setLevel(loglevel)
        logger.addHandler(fh)

    sys.excepthook = exception_handler
    logger.propagate = False
    return logger


def install_styles(style_default=STYLE_BASEFILE_STD):
    """
    Sets up style files

    Keyword Args:
        style_default (str): location of style file to use by defautl
   """

    mpl_styledir = _join(mpl_configdir(),"stylelib/")
    if not _exists(_join(mpl_styledir, _split(style_default)[1])):
        assert _exists(style_default), "STYLEFILE {} missing... indicates a problem with some paths or corrupt packege.\
                                    Check source code location".format(style_default)
        _copy(style_default, _join(mpl_styledir, _split(style_default)[1]))

install_styles()