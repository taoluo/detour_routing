#! /usr/bin/python
import re
import sys

if (len(sys.argv) != 2):
	print "Input incast flow num"
	exit(1)

incast_num = int(sys.argv[1])
print "incast num = ", incast_num
#exit(1)

#name = ["DCTCP_128_","detour_10_128_"]
name = ["tmp"]

background_port = 80
incast_port = 20

resolution = 0.00001
maxt = 3


interval = [10,40,80,120]


#1K * 10^(d-1) < flowsize <= 1K * 10^d
def Flow_Size_Position(flowsize):
	i = 1000
	d = 0
	while(i < flowsize):
		i *= 10
		d += 1
		if (d == 5):
			break;
	return d

stra = 0.0
file_1K = open("background_1K_exp15.plo","w")
file_10K = open("background_10K_exp15.plo", "w")
file_100K = open("background_100K_exp15.plo","w")
file_incast = open("query_exp15.plo","w")

for inter_idx in range(len(interval)):
        file_incast.write(str(interval[inter_idx])+'\t')
        file_1K.write(str(interval[inter_idx])+'\t')
        file_10K.write(str(interval[inter_idx])+'\t')
        file_100K.write(str(interval[inter_idx])+'\t')
        inter = interval[inter_idx]        
        for name_idx in range(len(name)):
                
                result = [[0.0 for ii in range(7)] for jj in range(4 * len(name))]
                incast_result = [[0.0 for ii in range(4)] for jj in range(len(name))]

                incast_count = [0] * 1024 
                #filename = name[name_idx] + str(interval[inter_idx]) + '_exp15_FCT'
		filename = "tmp"
                print filename
                percent = [[0.0 for i in range(7)] for j in range(int(maxt / resolution))]
                avr = 0.0	
                fn = 0
                for line in open(filename):
                        p1 = re.compile("(\S*) \[FLOW\] (\d*)\:(\d*)\-\>(\d*)\:(\d*) SIZE (\d*) TO (\d*) FCT (\S*)");
                        m1 = p1.match(line)

                        if  m1 != None:
                                t = float(m1.group(8))
                                s = int(m1.group(6))
                                port = int(m1.group(5))
                                index = int(t / resolution)
                                #print t,s,port,index
                                #print t
                                #exit(1)
                                if ( port == background_port):
                                        pos = Flow_Size_Position(s) + 1
                                        #print pos
                                        #exit(1)
                                        if (t < 3):	 
                                                percent[index][pos] += 1
                                elif (port == incast_port):
                                        dst = int(m1.group(4))
                                        incast_count[dst] += 1
                                        if ( incast_count[dst] == incast_num ):
                                                if ( t < 3) :
                                                        index = int (t / resolution)
                                                        percent[index][0] += 1
                                                        incast_count[dst] = 0
                                                        #print "incast"
                                                        #exit(1)


             
                #print percent
                for jj in range(7):
                        accu = 0
                        total = 0
                        step = 0
                        result_idx = name_idx * 4
                        for ii in range(int(maxt / resolution)):
                                total += percent[ii][jj]
                        print total
                                
                        if total == 0:
                                print "no",jj	
                                continue
                        if (jj == 0):
                                for ii in range(int(maxt / resolution)):
                                        accu += percent[ii][jj]
                                        if step == 0 and accu/total >= 0.5:
                                                incast_result[name_idx][0] = ii * resolution
                                                step += 1
                                        elif step == 1 and accu/total >= 0.95:
                                                incast_result[name_idx][1] = ii * resolution
                                                step += 1
                                        elif step == 2 and accu/total >= 0.99:
                                                incast_result[name_idx][2] = ii * resolution
                                                step += 1
                                                
                        else:
                                last_value = 0
                                for ii in range(int(maxt / resolution)):
                                        accu += percent[ii][jj]
                                        if (ii * resolution < 2.9):
                                                last_value = ii * resolution 		
                                                if step == 0 and accu/total >= 0.5:
                                                        result[0 + result_idx][jj] = ii * resolution
                                                        step += 1
                                                        print ii * resolution
                                                elif step == 1 and accu/total >= 0.75:
                                                        result[1 + result_idx][jj] = ii * resolution
                                                        step += 1
                                                        print ii * resolution
                                                elif step == 2 and accu/total >= 0.95:
                                                        result[2 + result_idx][jj] = last_value
                                                        print ii * resolution
                                                        step += 1
                                                elif step == 3 and accu/total >= 0.99:
                                                        
                                                        result[3 + result_idx][jj] = last_value
                                                        print ii * resolution
                                                        step += 1

	
                #endfile
                
                for ii in range(4):
                        file_incast.write(str(incast_result[name_idx][ii]) + '\t')
                        file_1K.write(str(result[ii + result_idx][2])+'\t')
                        file_10K.write(str(result[ii + result_idx][3])+'\t')
                        file_100K.write(str(result[ii + result_idx][4])+'\t')

        file_1K.write('\r\n')
        file_10K.write('\r\n')
        file_100K.write('\r\n')
        file_incast.write('\r\n')
                        
#print result
#wfile = open("result.plo", "w")
#for ii in range(7):
#	wfile.write( str(ii)+ '\t')
	#wfile.write(str(flow_num[ii]) + '\t')
#	for jj in range(4 * len(name)):
#		wfile.write(str(result[jj][ii]) + '\t')
#	wfile.write('\r\n')

#wfile = open("incast.plo", "w")
#step = 0
#for ii in ("media","95th","99th","99.9th"):
	
#	wfile.write(ii + '\t')
#	for jj in range(len(name)):
#		wfile.write(str(incast_result[jj][step]) + '\t')
#	step += 1
#	wfile.write('\r\n')
		
