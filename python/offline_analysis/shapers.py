import numpy as np
from scipy import signal

def gaussian_shaper(order,peaktime,dt=1E-9,pz=0.0,user_tf=None):

	# Based on Ohkawa 1976 
	#"Direct synthesis of the Gaussian filter for nuclear pulse amplifiers"

	if order == 1:
		tf = 2 * 1.0844
		poles = [complex(-1,0)]
	elif order == 2:
		tf = 9.734458e-01
		c = np.sqrt(np.sqrt(2) +2)*0.5
		poles = [c*complex(-1,np.sqrt(2) - 1),c*complex(-1,1 - np.sqrt(2))]
	elif order == 3:
		tf = 6.740357e-01
		poles = [complex(-1.2633573,0),
		         complex(-1.1490948,0.7864188),complex(-1.1490948,-0.7864188)]
	elif order == 4:
		tf = 5.106046e-01
		poles = [complex(-1.3553576,0.3277948),complex(-1.3553576,-0.3277948),
		         complex(-1.1810803,1.0603749),complex(-1.1810803,-1.0603749)]
	elif order == 5:
		tf = 4.267639e-01
		poles = [complex(-1.4766878,0),
		         complex(-1.4166647,0.5978596),complex(-1.4166647,-0.5978596),
					complex(-1.2036832,1.2994843),complex(-1.2036832,-1.2994843)]
	elif order == 6:
		tf = 3.737515e-01
		poles = [complex(-1.5601279,0.2686793),complex(-1.5601279,-0.2686793),
		         complex(-1.4613750,0.8329565),complex(-1.4613750,-0.8329565),
					complex(-1.2207388,1.5145343),complex(-1.2207388,-1.5145343)]
	elif order == 7:
		tf = 3.371212e-01
		poles = [complex(-1.6610245,0),
		         complex(-1.6229725,0.5007975),complex(-1.6229725,-0.5007975),
					complex(-1.4949993,1.0454546),complex(-1.4949993,-1.0454546),
					complex(-1.2344141,1.7113028),complex(-1.2344141,-1.7113028)]
	else:
		raise ValueError('only gaussian shaper orders between 1 and 7 are supported')

	#this gave roughly the right answer, but a separate script solved for the tf values above, which give extremely acurate peaking times
	#sigma = 2 * 1.0844 * peaktime * (1./order)
	if user_tf is not None:
		tf = user_tf
	sigma = tf * peaktime
	poles = np.array(poles)/sigma
	zeros = np.array([-pz])
	dzpk = signal.bilinear_zpk(zeros,poles,1,1./dt)
	return dzpk,(zeros,poles)

def crrc_shaper(order,peaktime,dt=1E-9,pz=0.0):
	rc = peaktime/order
	poles = np.full(order + 1,-1./rc)
	zeros = np.array([-pz])
	dzpk = signal.bilinear_zpk(zeros,poles,1,1./dt)
	return dzpk,(zeros,poles)
	
def shaper(shape,order,peaktime,dt=1E-9,pz=0.0,normalize=True,return_zpk=False,bipolar=False):
	if shape == 'gaussian':
		f = gaussian_shaper
	elif shape == 'crrc':
		f = crrc_shaper
	else:
		raise ValueError('specified shaper shape not supported')
	dzpk,azpk = f(order,peaktime,dt=dt,pz=pz)
	zeros,poles,gain = dzpk
	if bipolar:
		zeros = np.append(zeros,[1])

	if normalize:
		sos = signal.zpk2sos(zeros,poles,1)
		t = np.arange(-1*peaktime,2*peaktime,dt)
		tail = np.zeros_like(t)
		tail[t>0] = np.exp(-t[t>0] * pz)
		ytail = signal.sosfilt(sos,tail)
		gain = 1./np.max(ytail)
		sos = signal.zpk2sos(zeros,poles,gain)
		dzpk = zeros,poles,gain
	else:
		sos = signal.zpk2sos(*dzpk)

	if return_zpk:
		return sos,dzpk,azpk
	else:
		return sos

#class Shaper:
#	shapes = ('crrc','gaussian','trapz')
#
#	shape: one of ('crrc','gaussian','trapezoid')
#	peaktime: the peaking time in seconds.  If trapezoid, then a 3-tuple of integers specifiying number of samples in rising edge, flat top, and falling edge.
#	order: the order (number of poles), default = 5. For shape = "gaussian", order must be <= 7.  For "crrc", it can be anything.
#	dt: the sample period in seconds, default = 32E-9 (32 nanoseconds)
#	tau: the decay time of the input pulse, default is 0 (step input).  If the input pulses are tail pulses, set tau = tail pulse decay time for pole zero correction.
#
#	def __init__(self,shape,peaktime,order=5,dt=32E-9,tau=0,trapz_causal=True)
#		self.peaktime = peaktime
#		self.dt = dt
#		self.tau = tau
#		if shape in shapes:
#			self.shape = shape
#		else:
#			raise ValueError("shape must be one of " + str(shapes))
#		if shape == 'trapezoid':
#			self.times = [8,8,8]
#			try:
#				self.times[0],self.times[1],self.times[2] = [int(t) for t in peaktime]
#			except Exception as E:
#				print("when shape = trapezoid, peaktime must be a 3-tuple of integers")
#				raise E
#			rise,flat,fall = self.times
#			self.k = np.concatenate([np.ones(rise)/rise,np.zeros(flat),-np.ones(fall)/fall])
#			if trapz_causal:
#				self.k = np.concatenate([np.zeros(len(self.k)),self.k])
#		if shape == 'gaussian':
#			if order > 7:
#				raise ValueError("maximum order for gaussian shaper is 7")
#			self.k = shaper("gaussian",order,peaktime,dt=dt,pz=1./tau)
