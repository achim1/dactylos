"""
Graphical user interface with the tkinter package to visualize data
readout from a 
"""


import tkinter, time
import tkinter.messagebox
import numpy as np
import pylab as p
# import seaborn as sb

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

from .. import __version__
from .. import get_logger, create_timestamped_file

class SunChamberMonitoringGraphical(object):
    """
    A TKinter widget to monitor Sun chamber temperature data
    """

    def __init__(self, master, chamber,  interval=2,\
                 maxpoints=200, loglevel=20):
        """
        Initialize the application window

        Args:
            master (tkinter.Tk): A tkinter main application

        Keyword Args:
            interval (int): Update the plot every interval seconds
            maxpoints (int): Max number of points visible in the plot


        """

        # Create container and menus
        self.master = master
        self.logger = get_logger(loglevel)
        self.frame = tkinter.Frame(self.master)
        top = self.master.winfo_toplevel()
        self.menu_bar = tkinter.Menu(top)
        top['menu'] = self.menu_bar

        self.sub_menu_help = tkinter.Menu(self.menu_bar)
        self.sub_menu_plot = tkinter.Menu(self.menu_bar)
        self.menu_bar.add_cascade(label='Plot', menu=self.sub_menu_plot)
        self.menu_bar.add_cascade(label='Help', menu=self.sub_menu_help)
        self.sub_menu_help.add_command(label='About', command=self._about_handler)
        self.sub_menu_plot.add_command(label="Reset", command=self.init_plot)
        self.sub_menu_plot.add_command(label="Log to file", command=self.init_datafile)

        # physics quantities
        self.chamber = chamber
        self.start_time = time.monotonic()
        self.interval = interval
        self.maxpoints = maxpoints

        # writing results to file
        self.datafile_active = False
        self.datafilename = None
        self.datafile = None

        # plot
        fig = Figure()
        self.ax = fig.gca()
        self.canvas = FigureCanvasTkAgg(fig, master=self.master)
        #self.canvas.show()
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side='top', fill='both', expand=1)
        self.frame.pack()
        self.init_plot()
        self.update()
        

    def init_plot(self):
        """
        Initialize the plot
        """

        unit = self.chamber.temperature_unit
        axis_label = self.chamber.axis_label
        self.ax.set_xlabel("measurement time [s]")
        self.ax.set_ylabel("{} [{}]".format(axis_label, unit))

        # FIXME: make it versatile for n temperatures
        self.user_temp = self.ax.plot(range(0), color="blue", lw=3)[0]
        self.chamber_temp = self.ax.plot(range(0), color="red", lw=3)[0]

    @staticmethod
    def _about_handler():
        """
        Action performed if "about" menu item is clicked

        """
        tkinter.messagebox.showinfo("About", "Version: {}".format(__version__))

    def update(self):
        """
        Update the plot with recent temperature data
        """

    
        sec = time.monotonic() - self.start_time
    
        # make sure data in the plot is "falling over"
        # so that it does not get too crammed
        index = 0
    
        chamber_temps = []
        for channel, line_plot in enumerate([self.chamber_temp, self.user_temp]):

            secs, fields = line_plot.get_data()

            field = None
            try:
                field = self.chamber.get_temperature(channel=channel)
            except Exception as e:
                self.logger.warning("Can not acquire data for chanel {}! {}".format(channel, e))
                return
             
            if field is not None:
                fields = np.append(fields[index:], field)
            if not np.isfinite(field):
                print (field)
                # update the plot
                self.ax.set_xlim(xmin=xmin, xmax=xmax)
                self.ax.set_ylim(ymin=datamin, ymax=datamax)
                line_plot.set_ydata(fields)
                line_plot.set_xdata(secs)
                return

            if len(secs) >= self.maxpoints:
                self.logger.debug("Restricting line to {} points".format(self.maxpoints))
                index = 1
            secs = np.append(secs[index:], sec)
            chamber_temps.append(field)
            datamin = min(fields)
            datamax = max(fields)
            xmin = min(secs)
            xmax = max(secs)
    
            # avoid matplotlib warning
            if abs(datamin - datamax) < 1:
                datamin -= 1
                datamax += 1
    
            if abs(xmax - xmin) < 1:
                xmin -= 1
                xmax += 1

            # update the plot
            self.ax.set_xlim(xmin=xmin, xmax=xmax)
            self.ax.set_ylim(ymin=datamin, ymax=datamax)
            line_plot.set_ydata(fields)
            line_plot.set_xdata(secs)

        self.canvas.draw()
        self.master.after(self.interval, self.update)
    
        # write to the datafile if desired
        if self.datafile_active:
            self.datafile.write("{:4.2f} {:4.2f} {:4.2f}\n".format(sec, chamber_temps[0], chamber_temps[1]))

    def init_datafile(self):
        """
        Write measurement results to a logfile
        """
        if self.datafile_active:
            self.datafile.close()

        self.datafilename = create_timestamped_file("SUNEC13_TEMPERATURES", file_ending=".dat")
        self.logger.info("Writing to file {}".format(self.datafilename))
        tkinter.messagebox.showinfo("Writing to a file!", "Writing data to file {}".format(self.datafilename))
        self.datafile = open(self.datafilename, "w")
        self.datafile.write("# seconds {} {}\n".format(self.chamber.unit, self.chamber.unit))
        self.datafile_active = True

    def __del__(self):
        """
        Close open files
        """
        if self.datafile_active:
            self.datafile.close()


