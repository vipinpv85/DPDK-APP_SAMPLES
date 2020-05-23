#! /usr/bin/python

import sys, os, getopt, subprocess
from os.path import exists, abspath, dirname, basename

dpdk_drivers = [ "igb_uio", "vfio-pci", "uio_pci_generic" ]
bitTable = {0:0x00, 1:0x1, 2:0x3, 3:0x7, 4:0xf, 5:0x1f, 6:0x3f, 7:0x7f, 8:0xff, 9:0x1ff, 10:0x3ff, 11:0x7ff, 12:0xfff, 13:0x1fff, 14:0x3fff, 15:0x7fff, 16:0xffff}

def check_output(args, stderr=None):
  '''Run a command and capture its output'''
  return subprocess.Popen(args, stdout=subprocess.PIPE,
                            stderr=stderr).communicate()[0]

dev_nic   = check_output(["lspci"]).splitlines()
dev_lines = check_output(["lspci", "-Dvmmn"]).splitlines()

nicInfo = []
nicCount = 0

#for lines in dev_nic:
#  print lines
#for lines in dev_lines:
#  print lines

for eth in dev_nic:
  if ("Ethernet Controller" in eth):
    #print eth.split()[0] 
    nicInfo.append(eth.split()[0].strip())
  # add for virt-IO

#print "NicInfo: " + str(nicInfo)

for addr in nicInfo:
  #print addr
  data = check_output(["lspci", "-ks", addr.strip()]).splitlines()
  #print data

  # lspci -ks 04:00.0 | grep 'Kernel driver in use:'
  for info in data:
    #print info
    if(info.strip().startswith("Kernel driver in use:")):
      #print info
      data = info.split(":")
      #print data
      if (data[1].strip() in dpdk_drivers):
        #print addr 
        nicCount += 1

print '0x' + str(bitTable[nicCount])

