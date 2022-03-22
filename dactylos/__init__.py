"""
Software package to connect with a N675 digitizer made by CAEN.
Supports:
 - waveform readout firmware
 - DPP-PHA firmware
"""

__version__ = '0.0.30'


from .CaenN6725 import CaenN6725

LOGLEVEL = 20
