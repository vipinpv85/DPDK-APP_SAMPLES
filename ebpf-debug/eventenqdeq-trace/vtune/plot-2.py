import matplotlib.pyplot as plt
import numpy as np
import csv
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--file',  required=True, help='csv file to plot')
#args = parser.parse_args()
args = vars(parser.parse_args())

#parser.print_help()
#print("Hi there {}, it's nice to meet you!".format(args["file"]))

starttime = 0
maxactual = 0
time = []
request = []
actual = []
diff = []

csvFile = csv.reader(open(args["file"]), delimiter=',', quotechar='|')
csvFile.next()


for row in csvFile:
  #print " r  1 " + str(row[1]) + " row 2 " + str(row[2])

  if (len(time) == 0):
    starttime = int(row[0])
    time.append(0)
  else:
    time.append(int(row[0]) - starttime)

  request.append(row[2])
  actual.append(row[3])
  diff.append(int(row[2]) - int(row[3]))

  if (maxactual < int(row[3])):
    maxactual = int(row[3])

  if (csvFile.line_num > 100):
    break

#plt.axis([0,int(time[-1]), 0, 2 * maxactual])
#plt.axis()

width = 0.35
p1 = plt.bar(time, request, width) 
p2 = plt.bar(time, actual, width, bottom=diff)

plt.title('Eventdev Deq')
plt.xlabel('time slot diff')
plt.ylabel('number of events')
plt.legend((p1[0], p2[0]), ('Request', 'Actual'))

plt.plot(time, request, 'ro', time, actual, 'gs')

plt.show()
