#! /usr/bin/python

import sys, os, getopt, subprocess
from os.path import exists, abspath, dirname, basename

dpdk_drivers = [ "igb_uio", "vfio-pci", "uio_pci_generic" ]

def check_output(args, stderr=None):
  '''Run a command and capture its output'''
  return subprocess.Popen(args, stdout=subprocess.PIPE,
                            stderr=stderr).communicate()[0]

ps = subprocess.Popen(('dmidecode', '-t', 'slot'), stdout=subprocess.PIPE)
pci_slot = subprocess.check_output(('egrep', 'Bus|ID|Usage'), stdin=ps.stdout).splitlines()
ps.wait()

#pci_slot  = check_output(["dmidecode", "-t", "slot"]).splitlines()
#pci_slot  = subprocess.call(["dmidecode", "-t", "slot", "|", "egrep", "Bus|ID|Usage"])
dev_nic   = check_output(["lspci"]).splitlines()
dev_lines = check_output(["lspci", "-Dvmmn"]).splitlines()

nicInfo = {}
slotInfo = {}

slotAddr = []
slotId  = []
slotUse  = []



for slot in pci_slot:
  if ("Bus Address:" in slot):
    slotAddr.append(slot.strip().split()[2])
  elif ("ID:" in slot):
    slotId.append(slot.strip().split()[1])
  elif ("Usage:" in slot):
    slotUse.append(slot.strip().split()[2])

for eth in dev_nic:
  if ("Ethernet Controller" in eth):
    #print eth.split()[0] 
    nicInfo[eth.split()[0].strip()] = []

for index in xrange(0, len(slotAddr)):
  if ("Available" not in slotUse[index]):
    slotInfo[slotAddr[index]] = str(slotId[index])

#print slotInfo 
#print nicInfo

print ""
print ""
print ""
print "   +++ DPDK NIC to Physical slot Mapping +++ "
print " ++++++++++++++++++++++++++++++++++++++++++++++ "
print ""

for addr in slotInfo.keys():
  addrsplit = addr.split(":")
  data = check_output(["lspci", "-ts", addr.strip()]).splitlines()[0]
  #print addrsplit
  #print data
  for repl in addrsplit: 
    data =  data.replace(repl, "") 

  #print data
  for da in [":", "+", "[", "]"]:
    data = data.replace(da, "")

  data = data.split("-")
  #print data

  nicKeys = nicInfo.keys()

  for d in data:
    if (d != ''):
      #print nicKeys
      for val in nicKeys:
        if (d in val.split(":")):
          #print val.strip()
          kdata = check_output(["lspci", "-ks", val.strip()]).splitlines()
           
          for temp in kdata:
            if ("Kernel driver in use:" in temp):
              driverDet =  temp.replace("Kernel driver in use:", "").strip()
              if (driverDet in dpdk_drivers):
                cpuNode = "cat /sys/bus/pci/devices/0000\:"+str(val.replace(':','\:'))+"/numa_node"
                node = subprocess.check_output(cpuNode, shell=True, stderr=subprocess.STDOUT,)
                print " Bus: " + str(val) + " Slot: " + str(slotInfo[addr]) + " Node: " + str(node.strip()) + " Driver: " + str(driverDet)
print " ------------------------------"
print ""
