#! /usr/bin/python
import os


server_list = [13,21,24,25,35,37,38,40,42,43]
for server in server_list:
	os.system('scp src/net-routing/ecmp/* minlanyu@sns'+str(server)+'.cs.princeton.edu:/disk/local/minlanyu/ns-'+str(server)+'/src/net-routing/ecmp/')
	#os.system('scp src/internet/model/* minlanyu@sns'+str(server)+'.cs.princeton.edu:/disk/local/minlanyu/ns-'+str(server)+'/src/internet/model/')
		
