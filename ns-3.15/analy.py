#! /usr/bin/python
import re
import sys

if (len(sys.argv) != 2):
	print "Input incast flow num"
	exit(1)

incast_num = int(sys.argv[1])
print "incast num = ", incast_num
#exit(1)

#name = ["DCTCP","TCP_infi","DCTCP_infi","detour", "detour_300"]
#name = ["DCTCP","DCTCP_infi","detour","detour_300"]
#name = ["DCTCP","detour"]
#name = ["DCTCP_infi"]
name = ["tmp"]
#name = ["DCTCP_128_exp1","TCP_infi_128_exp1","DCTCP_infi_128_exp1","detour_10_128_exp1","detour_300_128_exp1"]
#name = ["detour","detour_300"]
#name = ["DCTCP_10", "tag_10","tag_10_1.1","tag_1.5","tag_2","tag_5"]

#num_flow = 30

background_port = 80
incast_port = 20

resolution = 0.00001
maxt = 2
#incast, 1K, 10K, 100K, 1M, 10M, infi


 
#flow_num = [2,4,8]
#percent[5][4] = 1
#print percent[5][1]
#exit(1)

result = [[0.0 for ii in range(7)] for jj in range(4 * len(name))]
incast_result = [[0.0 for ii in range(4)] for jj in range(len(name))]


#1K * 10^(d-1) < flowsize <= 1K * 10^d
def Flow_Size_Position(flowsize):
	i = 1000
	d = 0
	while(i < flowsize):
		i *= 10
		d += 1
	return d

stra = 0.0
for name_idx in range(len(name)):
	incast_count = [0] * 1024 
	filename = name[name_idx]
	percent = [[0.0 for i in range(7)] for j in range(int(maxt / resolution))]
	avr = 0.0	
	fn = 0
	for line in open(filename):
		#print line
		p1 = re.compile("(\S*)\[ProcessWait\] Finish (\d*)\:(\d*)\-\>(\d*)\:(\d*) (\d*) FCT=(\S*)");
		#p2 = re.compile("=====round  (\d*)=====")
		m1 = p1.match(line)
		#m2 = p2.match(line)
		#if m2 != None:
			#index = int (stra / resolution)
			#percent[index][0] += 1
		if  m1 != None:
			#print m1.group(1),m1.group(2),m1.group(3)
			print m1.group(4),m1.group(5),m1.group(6),m1.group(7)
			exit(1)			
			t = float(m1.group(7))
			
			
			s = int(m1.group(6))
			port = int(m1.group(5))
			index = int(t / resolution)
			
			if ( port == background_port):
				pos = Flow_Size_Position(s) + 1
				#print s,t,pos
				#exit(1)
				#print t
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
				stra = t
			#if (t >= 0.3):
			#	print "syn timeout"
			#	continue

			#avr += t
			#fn += 1
			#print t
			#exit(1)
					
	

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
		#jj = 0 : incast flows
		#jj = 1~6:
		if (jj == 0):
			for ii in range(int(maxt / resolution)):
				accu += percent[ii][jj]
				if step == 0 and accu/total >= 0.5:
					incast_result[name_idx][0] = ii * resolution
					step += 1
					#print ii * resolution
				elif step == 1 and accu/total >= 0.95:
					incast_result[name_idx][1] = ii * resolution
					step += 1
					#print ii * resolution
				elif step == 2 and accu/total >= 0.99:
					incast_result[name_idx][2] = ii * resolution
					#print ii * resolution
					step += 1
				elif step == 3 and accu/total >= 0.999:
					incast_result[name_idx][3] = ii * resolution
					#print ii * resolution
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
					#result[2 + result_idx][jj] = ii * resolution
					result[3 + result_idx][jj] = last_value
					print ii * resolution
					step += 1

	
			
print result
wfile = open("result.plo", "w")
for ii in range(7):
	wfile.write( str(ii)+ '\t')
	#wfile.write(str(flow_num[ii]) + '\t')
	for jj in range(4 * len(name)):
		wfile.write(str(result[jj][ii]) + '\t')
	wfile.write('\r\n')

wfile = open("incast.plo", "w")
step = 0
for ii in ("median","95th","99th","99.9th"):
	
	wfile.write(ii + '\t')
	for jj in range(len(name)):
		wfile.write(str(incast_result[jj][step]) + '\t')
	step += 1
	wfile.write('\r\n')
		
	
#wfile = open("CDF.plo", "w")
#accu = [0] * len(name)
#accu_4 = [0] * len(name)
#accu_8 = [0] * len(name)
#total = [0] * len(name)
#total_4 = [0] * len(name)
#total_8 = [0] * len(name)
#for ii in range(len(name)):
#	for jj in range(int(maxt / resolution)):
#		total[ii] += percent[jj][ii]
		#total_4[ii] += percent_4[jj][ii]
		#total_8[ii] += percent_8[jj][ii]
#print total
 
#for ii in range(int(maxt / resolution)):
#	wfile.write(str(ii * resolution) + '\t')
#	for jj in range(len(accu)):
		#accu_2[jj] += percent_2[ii][jj]
		#wfile.write(str(accu_2[jj]/ total_2[jj]) + '\t')
		#accu_4[jj] += percent_4[ii][jj]
		#wfile.write(str(accu_4[jj]/ total_4[jj]) + '\t')
#		accu[jj] += percent[ii][jj]
#		wfile.write(str(accu[jj]/ total[jj]) + '\t')
#	wfile.write('\r\n')

