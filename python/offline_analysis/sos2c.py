import numpy as np

def sos2c(sos,gain=1,funcname='filter'):
	l = []
	l.append('//numpy second order section array:')
	l.append('/*')
	l.append(str(sos))
	l.append('*/')
	l.append('double %s(double x)\n{' % funcname)
	l.append('\n\tdouble y;\n')
	for i in range(sos.shape[0]):
		l.append('\t//biquad section %d (bq%d)' % (i,i))
		l.append('\tstatic double bq%dz1 = 0;' % i)
		l.append('\tstatic double bq%dz2 = 0;' % i)
		l.append('\ty = %.15e * x + bq%dz1;' % (sos[i,0],i))
		l.append('\tbq%dz1 = bq%dz2 + %.15e * x - %.15e * y;' % (i,i,sos[i,1],sos[i,4]))
		l.append('\tbq%dz2 = %.15e * x - %.15e * y;' % (i,sos[i,2],sos[i,5]))
		l.append('\tx = y;\n')

	l.append('\treturn %.15e * y;' % gain)
	l.append('}')
	return '\n'.join(l)

